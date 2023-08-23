#pragma once
bool BufferEquals(const uint8_t*,const uint8_t*,uint32_t);
uint32_t Wstrncpy(wchar_t* dst,const wchar_t* src,uint32_t count);
void PrintWstring(const wchar_t*,int);
uint32_t DecryptAsset(const uint8_t* pwd,uint32_t plength,uint8_t* inputBuf,uint32_t length);
void WriteToFile(const wchar_t* filename,const uint8_t* data,int length);
void ReadFromFile(const wchar_t* filename,uint32_t offset,uint32_t length,uint8_t* buffer);
void ReplaceAllWchar(wchar_t* str,wchar_t src,wchar_t dest);
size_t Wstrcat(wchar_t* dst,const wchar_t* sour);
int32_t Wstrwstr(const wchar_t* str,uint32_t len1,const wchar_t* substr,uint32_t len2);
size_t Wstrcpy(wchar_t* dst,const wchar_t* sour);
void MakeDirectory(const wchar_t* dir);
int Wstrwch(const wchar_t*,wchar_t ch,int start);
uint32_t Wstrlen(const wchar_t* str);
const wchar_t* DetectExt(const uint8_t* data,const wchar_t* entry_name);