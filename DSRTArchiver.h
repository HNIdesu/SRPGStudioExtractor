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
    uint32_t GetLength();
    
    private:
    std::wstring entry_name;
    uint32_t offset;
    uint32_t length;
};

class DSRTArchiver
{
private:
    std::vector<DSRTEntry>* entries;
    const uint8_t DSRTSignature[4]={0x53,0x44,0x54,0x53};
    bool is_encrypted;
    uint32_t version;
    wstring file_path;
public:
    const std::vector<DSRTEntry> GetEntries();
    uint32_t ExtractEntry(DSRTEntry entry,unsigned char* buffer,const unsigned char* key,unsigned int keylength);
    DSRTArchiver(const wchar_t* filename);
    ~DSRTArchiver();
};

