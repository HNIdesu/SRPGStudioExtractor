#include <iostream>
#include <windows.h>
#include <string>
#include <vector>
#include "Utility.h"
using namespace std;

HCRYPTPROV GlobalHCryptProvicer;
HCRYPTKEY GlobalHCryptKey;


struct Format{
    int Length;
    vector<uint8_t> Signature;
    const wchar_t* Ext;
};
Format KnownFormats[]={
    {2,{0xFF,0xD8},L".jpeg"},
    {4,{0x89,'P','N','G'},L".png"},
    {4,{'O','g','g','S'},L".ogg"},
    {3,{'I','D','3'},L".mp3"},
    {2,{0xFF,0xFB},L".mp3"},
    {4,{'R','I','F','F'},L".wav"}
};

int32_t Wstrwstr(const wchar_t* str,uint32_t len1,const wchar_t* substr,uint32_t len2){
    if(!len1)
        len1=Wstrlen(str);
    if(!len2)
        len2=Wstrlen(substr);

    for(uint32_t offset=0;len1-offset>=len2;){
        bool flag=true;
        for(uint32_t i=0;i<len2;i++)
        {
            if(str[offset+i]!=substr[i]){
                offset+=i+1;
                flag=false;
                break;
            }
        }
        if(flag)return offset;
    }
    return -1;
}

bool BufferEquals(const uint8_t* buf1,const uint8_t* buf2,uint32_t len){
    while(len--)
        if(buf1[len]!=buf2[len])
            return false;
    return true;
}

void WriteToFile(const wchar_t* filename,const uint8_t* data,int length){
    HANDLE hFile=CreateFileW(filename,GENERIC_WRITE,FILE_SHARE_READ,nullptr,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
    WriteFile(hFile,data,length,nullptr,nullptr);
    CloseHandle(hFile);
}

uint32_t Wstrncpy(wchar_t* dst,const wchar_t* src,uint32_t count){
    memcpy(dst,src,count*2);
    return count;
}

void ReadFromFile(const wchar_t* filename,uint32_t offset,uint32_t length,uint8_t* buffer){
    HANDLE hFile=CreateFileW(filename,GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
    if(offset)
        SetFilePointer(hFile,offset,nullptr,FILE_BEGIN);
    ReadFile(hFile,buffer,length,nullptr,nullptr);
    CloseHandle(hFile);
}

void PrintWstring(const wchar_t* str,int length){
    char* buf=new char[length*3+1];
    buf[WideCharToMultiByte(GetACP(),0,str,length,buf,length*3,nullptr,nullptr)]='\0';
    cout<<buf<<endl;
    delete[] buf;
}

uint32_t DecryptAsset(const uint8_t* pwd,uint32_t plength,uint8_t* inputBuf,uint32_t length){
    if(!GlobalHCryptProvicer){
        CryptAcquireContextW(&GlobalHCryptProvicer,nullptr,nullptr,PROV_RSA_FULL,CRYPT_VERIFYCONTEXT);
        HCRYPTHASH hHash;
        CryptCreateHash(GlobalHCryptProvicer,CALG_MD5,0,0,&hHash);
        CryptHashData(hHash,pwd,plength,0);
        if(!CryptDeriveKey(GlobalHCryptProvicer,26625,hHash,0,&GlobalHCryptKey)){
            DWORD code= GetLastError();
            CryptDestroyHash(hHash);
            if(code&ERROR_INVALID_HANDLE){
                throw L"INVALID HANDLE";
            }else if(code&ERROR_INVALID_PARAMETER){
                throw L"INVALID PARAMETER";
            }else if(code&NTE_BAD_ALGID){
                throw L"BAD ALGID";
            }else if (code&NTE_BAD_FLAGS)
                throw L"BAD FLAGS";
            else if(code&NTE_BAD_HASH)
                throw L"BAD HASH";
            else if (code&NTE_BAD_HASH_STATE)
                throw L"BAD STATE";
            else
                throw L"UNKNOWN ERROR";
        }
    }
    CryptDecrypt(GlobalHCryptKey,0,TRUE,0,inputBuf,(DWORD*)&length);
    return length;
}

size_t Wstrcpy(wchar_t* dst,const wchar_t* sour){
    size_t length=0;
    while (*sour)
        *(dst++)=*(sour++),length++;
    return length;
    
}

size_t Wstrcat(wchar_t* dst,const wchar_t* sour){
    size_t length=0;
    while(*dst)
    {
        dst++;
        length++;
    }
    while(*sour){
        *(dst++)=*(sour++);
        length++;
    }
    return length;
}

uint32_t Wstrlen(const wchar_t* str){
    uint32_t count=0;
    while(*(str++))count++;
    return count;
}

int Wstrwch(const wchar_t* str,wchar_t ch,int start){
    for(int i=0;*(str+start+i);i++)
        if(*(str+start+i)==ch)
            return start+i;
    return -1;
}

void ReplaceAllWchar(wchar_t* str,wchar_t src,wchar_t dest){
    while(*str){
        if(*str==src)
            *str=dest;
        str++;
    }

}

void MakeDirectory(const wchar_t* fullpath){
    uint32_t path_length=Wstrlen(fullpath);
    wchar_t* subPath=new wchar_t[path_length+1];
    for(uint32_t i=0;i<path_length;i++){
        i=Wstrwch(fullpath,L'\\',i);
        if(i!=-1){
            subPath[Wstrncpy(subPath,fullpath,i)]=L'\0';
            CreateDirectoryW(subPath,nullptr);
        }
        else
            break;
      
    }
    delete[] subPath;
}



const wchar_t* DetectExt(const uint8_t* data,const wchar_t* entry_name){
    int count=sizeof(KnownFormats)/sizeof(Format);
    if(Wstrwstr(entry_name,0,L"Fonts",0)==0)return L".ttf";
    for(int i=0;i<count;i++)
        if(BufferEquals(data,KnownFormats[i].Signature.data(),KnownFormats[i].Length))
            return KnownFormats[i].Ext;
    
    return L"";
}