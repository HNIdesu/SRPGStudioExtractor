#pragma once
#include <windows.h>
#include <iostream>
#include <vector>
#include <string>

using namespace std;

class DSRTEntry{
    public:
    friend class DSRTArchiver;
    const std::wstring GetEntryName();
    unsigned int GetLength();
    
    private:
    std::wstring entry_name;
    unsigned int offset;
    unsigned length;
};

class DSRTArchiver
{
private:
    std::vector<DSRTEntry>* entries;
    const unsigned char DSRTSignature[4]={0x53,0x44,0x54,0x53};
    bool is_encrypted;
    unsigned int version;
    wstring file_path;
public:
    const std::vector<DSRTEntry> GetEntries();
    unsigned int ExtractEntry(DSRTEntry entry,unsigned char* buffer,const unsigned char* key,unsigned int keylength);
    DSRTArchiver(const wchar_t* filename);
    ~DSRTArchiver();
};

