#include "DSRTArchiver.h"
#include <windows.h>
#include "utility.h"

#define KNOWNFRAGMENTCOUNT 35

const wchar_t* KnownEntryNames[KNOWNFRAGMENTCOUNT]={
    L"Graphics/mapchip",
    L"Graphics/charchip",
    L"Graphics/face",
    L"Graphics/icon",
    L"Graphics/motion",
    L"Graphics/effect",
    L"Graphics/weapon",
    L"Graphics/bow",
    L"Graphics/thumbnail",
    L"Graphics/battleback",
    L"Graphics/eventback",
    L"Graphics/screenback",
    L"Graphics/worldmap",
    L"Graphics/eventstill",
    L"Graphics/charillust",
    L"Graphics/picture",
    L"UI/menuwindow",
    L"UI/textwindow",
    L"UI/title",
    L"UI/number",
    L"UI/bignumber",
    L"UI/gauge",
    L"UI/line",
    L"UI/risecursor",
    L"UI/mapcursor",
    L"UI/pagecursor",
    L"UI/selectcursor",
    L"UI/scrollcursor",
    L"UI/panel",
    L"UI/faceframe",
    L"UI/screenframe",
    L"Audio/music",
    L"Audio/sound",
    L"Fonts",
    L"Video"
};

struct Fragment
{
    const wchar_t* entry_name;
    unsigned int position;
    unsigned int length;
};

const std::wstring DSRTEntry::GetEntryName(){
    return this->entry_name;
}

 const std::vector<DSRTEntry> DSRTArchiver::GetEntries(){
    return *entries;
}

unsigned int DSRTEntry::GetLength(){
    return length;
}

unsigned int DSRTArchiver::ExtractEntry(DSRTEntry entry,unsigned char* buffer,const unsigned char* key=nullptr,unsigned int keylength=0){
    unsigned int length=entry.length;
    ReadFromFile(file_path.c_str(),entry.offset,length,buffer);
    if(this->is_encrypted)
        length=DecryptAsset(key,keylength,buffer,length);
    return length;
}

DSRTArchiver::DSRTArchiver(const wchar_t* filepath){
    #define BUFFERSIZE 32768
    DSRTEntry entry;
    file_path=filepath;
    HANDLE hFile=CreateFileW(filepath,GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);
    if(!hFile)
        throw L"Open file error";
    BYTE* buffer=new BYTE[BUFFERSIZE];
    DWORD bytesRead;
    entries=new std::vector<DSRTEntry>();
    ReadFile(hFile,buffer,24,nullptr,nullptr);
    {
        entry.entry_name=L"Project.srpgs";
        entry.offset=*((uint32_t*)(buffer+20))+168;
        entry.length=GetFileSize(hFile,nullptr)-entry.offset;
        entries->push_back(entry);
    }
    if(!BufferEquals(buffer,DSRTSignature,4))
        throw L"Signature error";
    is_encrypted=*((UINT*)(buffer+4))==1;
    version=*((UINT*)(buffer+8));
    Fragment* fragments=new Fragment[KNOWNFRAGMENTCOUNT];
    ReadFile(hFile,buffer,36*4,nullptr,nullptr);
    UINT* offsets=(UINT*)buffer;
    for(int i=0;i<KNOWNFRAGMENTCOUNT;i++){
        fragments[i].entry_name=KnownEntryNames[i];
        fragments[i].position=offsets[i]+168;
        fragments[i].length=offsets[i+1]-offsets[i];
    }
    
    for(UINT i=0,x1;i<KNOWNFRAGMENTCOUNT;i++){
        
        Fragment* curFrag=&fragments[i];
        SetFilePointer(hFile,curFrag->position,nullptr,FILE_BEGIN);
        ReadFile(hFile,buffer,4,nullptr,nullptr);
        UINT resource_group_count=*((UINT*)buffer);
        if(curFrag->length==0)continue;
        UINT info_length=(resource_group_count)*4;
        UINT offset=0;

        UINT* resource_group_offsets=new UINT[resource_group_count+1];
        x1=0;

        while (offset<info_length)
        {
            ReadFile(hFile,buffer,info_length-offset>BUFFERSIZE?BUFFERSIZE:info_length-offset,&bytesRead,nullptr);
            offset+=bytesRead;
            for(UINT j=0;j<bytesRead;j+=4)
                resource_group_offsets[x1++]=*((UINT*)(buffer+j))+curFrag->position;
        }
        resource_group_offsets[x1]=curFrag->position+curFrag->length+1;
        
        for (UINT j=0;j<resource_group_count;j++)
        {
            UINT resource_group_offset=resource_group_offsets[j],
                resource_group_length=resource_group_offsets[j+1]-resource_group_offsets[j],
                resource_group_name_length;
            SetFilePointer(hFile,resource_group_offset,nullptr,FILE_BEGIN);
            ReadFile(hFile,buffer,4,nullptr,nullptr);
            resource_group_name_length=*((UINT*)buffer);
            wchar_t* resource_group_name=new wchar_t[resource_group_name_length/2];
            if(resource_group_name_length){
                ReadFile(hFile,buffer,resource_group_name_length,nullptr,nullptr);
                Wstrncpy(resource_group_name,(const wchar_t*)buffer,resource_group_name_length/2); 
                //PrintWstring(resource_group_name,resource_group_name_length/2);  
                
            }   
            
            {
                ReadFile(hFile,buffer,12,nullptr,nullptr);
                unsigned int baseOffset=SetFilePointer(hFile,0,nullptr,FILE_CURRENT);
                UINT file_count=*((UINT*)(buffer+8));
                info_length=file_count*4;
                UINT x2=0,x3=0;

                ReadFile(hFile,buffer,info_length,nullptr,nullptr);
                for(int k=0;k<file_count;k++){
                    entry.entry_name=curFrag->entry_name;
                    entry.entry_name+=L'/';
                    if(k>0){
                        entry.entry_name.append(resource_group_name);
                        entry.entry_name+=L'-';
                        entry.entry_name+=L'0'+k;
                    }      
                    else
                        entry.entry_name.append(resource_group_name);
                    entry.length=*((UINT*)(buffer+(x3+=4,x3-4)));
                    entry.offset=baseOffset+x2+info_length;
                    x2+=entry.length;
                    entries->push_back(entry);
                }
                
            }
            delete[] resource_group_name;
        }
        
        delete[] resource_group_offsets;

    }
    

    CloseHandle(hFile);
    delete[] buffer;
    delete[] fragments;
    #undef BUFFERSIZE
    return;
}

DSRTArchiver::~DSRTArchiver(){
    delete entries;
}

#undef KNOWNENTRYCOUNT