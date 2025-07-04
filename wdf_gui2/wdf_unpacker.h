#pragma once
#include <windows.h>
#include <vector>
#include <string>

struct WDataFileIndex {
    DWORD uid;
    DWORD offset;
    DWORD size;
    DWORD space;
};

struct WDataFileHeader {
    DWORD id;
    int number;
    unsigned offset;
};

class WDFUnpacker {
public:
    WDFUnpacker();
    bool OpenWdf(const std::wstring& wdfPath);
    void Close();
    bool LoadIndex();
    // innerName改为std::string，和wpp16一致
    bool ExtractFile(const std::string& innerNameA, const std::wstring& outPath, std::wstring& log);
    bool ExtractByLst(const std::wstring& lstPath, const std::wstring& outDir, std::vector<std::wstring>& logs, int& success, int& fail);
    size_t GetFileCount() const;
    const std::vector<WDataFileIndex>& GetIndex() const;

private:
    HANDLE m_File; // WDF文件句柄
    std::wstring m_wdfPath; // WDF文件路径
    std::vector<WDataFileIndex> m_Index; // 索引表
};

// WPP16 original hash algorithm (string_id and string2id)
uint32_t ws_string_id(const std::string& filename);
uint32_t ws_string2id(const char* str); 