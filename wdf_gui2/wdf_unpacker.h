#pragma once

// VS2010 下屏蔽 C4996（安全函数）和 C4819（编码警告）
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable:4996)
#pragma warning(disable:4819)

#include <windows.h>
#include <vector>
#include <string>

// 索引条目：UID + 文件偏移 + 大小 + 可用空洞（目前解包不使用 space）
struct WDataFileIndex {
    DWORD uid;      // 文件唯一标识
    DWORD offset;   // 数据区文件起始偏移
    DWORD size;     // 文件大小（字节）
    DWORD space;    // 空洞大小（解包时忽略）
};

// 文件头：魔数 + 条目数 + 索引区偏移
struct WDataFileHeader {
    DWORD id;       // 应为 'WDFP'
    int number;     // 条目数量
    unsigned offset;// 索引区在文件中的偏移
};

// WDF 解包器：仅提供读取索引 & 按名单批量/单文件解包
class WDFUnpacker {
public:
    WDFUnpacker();
    ~WDFUnpacker();

    // 打开 .wdf 文件并加载索引
    bool OpenWdf(const std::wstring& wdfPath);
    void Close();

    // 从文件头读取并验证索引区
    bool LoadIndex();

    // 单文件解包：innerNameA 是 .lst 里的窄字符串
    // outPath 是完整的目标路径（宽字符）
    // log 用于返回 "[Extracted] xxx" 或 "[Not found] xxx"
    bool ExtractFile(
        const std::string& innerNameA,
        const std::wstring& outPath,
        std::wstring& log
    );

    // 按 .lst 批量解包：logs 存每条记录的输出信息，success/fail 计数
    bool ExtractByLst(
        const std::wstring& lstPath,
        const std::wstring& outDir,
        std::vector<std::wstring>& logs,
        int& success,
        int& fail
    );

    // 索引访问
    size_t GetFileCount() const;
    const std::vector<WDataFileIndex>& GetIndex() const;

private:
    HANDLE                   m_File;    // WDF 文件句柄
    std::wstring             m_wdfPath; // 打开的 WDF 路径
    std::vector<WDataFileIndex> m_Index; // 内存中的索引表
};

DWORD string_id(const char* str);
