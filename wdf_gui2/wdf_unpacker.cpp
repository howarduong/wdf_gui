// wdf_unpacker.cpp
#include "stdafx.h"

#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable:4996)   // 禁用 strncpy/_wfopen 等安全警告
#pragma warning(disable:4819)   // 禁用编码警告

#include "wdf_unpacker.h"
#include <windows.h>
#include <shlobj.h>
#include <stdlib.h>    // malloc, free
#include <string.h>    // memcpy, strncpy
#include <vector>
#include <string>

// 将 ANSI (CP_ACP) 字符串转换为 std::wstring
static std::wstring AnsiToWide(const char* str) {
    if (!str) return L"";
    int len = ::MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, 0);
    std::wstring w;
    w.resize(len);
    ::MultiByteToWideChar(CP_ACP, 0, str, -1, &w[0], len);
    return w;
}

// 原版 string_adjust：ASCII 大写→小写，'/'→'\\'
static void string_adjust(char* p) {
    int i = 0;
    while (p[i]) {
        if (p[i] >= 'A' && p[i] <= 'Z')
            p[i] += 'a' - 'A';
        else if (p[i] == '/')
            p[i] = '\\';
        ++i;
    }
}

// 原版 string_id：x86 汇编哈希算法
DWORD string_id(const char* str) {
    int i;
    unsigned int v;
    static unsigned m[70];
    strncpy((char*)m, str, 256);
    for (i = 0; i < 256/4 && m[i]; i++);
    m[i++] = 0x9BE74448;
    m[i++] = 0x66F42C48;
    v = 0xF4FA8928;
    __asm {
        mov esi,0x37A8470E
        mov edi,0x7758B42B
        xor ecx,ecx
    _loop:
        mov ebx,0x267B0B11
        rol v,1
        lea eax,m
        xor ebx,v
        mov eax,[eax+ecx*4]
        mov edx,ebx
        xor esi,eax
        xor edi,eax
        add edx,edi
        or edx,0x2040801
        and edx,0xBFEF7FDF
        mov eax,esi
        mul edx
        adc eax,edx
        mov edx,ebx
        adc eax,0
        add edx,esi
        or edx,0x804021
        and edx,0x7DFEFBFF
        mov esi,eax
        mov eax,edi
        mul edx
        add edx,edx
        adc eax,edx
        jnc _skip
        add eax,2
    _skip:
        inc ecx
        mov edi,eax
        cmp ecx,i
        jnz _loop
        xor esi,edi
        mov v,esi
    }
    return v;
}

// 构造 / 析构
WDFUnpacker::WDFUnpacker()
  : m_File(INVALID_HANDLE_VALUE) {
}

WDFUnpacker::~WDFUnpacker() {
    Close();
}

// 打开 WDF 并加载索引
bool WDFUnpacker::OpenWdf(const std::wstring& wdfPath) {
    Close();
    m_File = CreateFileW(
        wdfPath.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    if (m_File == INVALID_HANDLE_VALUE) return false;
    m_wdfPath = wdfPath;
    return LoadIndex();
}

void WDFUnpacker::Close() {
    if (m_File != INVALID_HANDLE_VALUE) {
        CloseHandle(m_File);
        m_File = INVALID_HANDLE_VALUE;
    }
    m_Index.clear();
}

// 读取 & 校验文件头与索引区
bool WDFUnpacker::LoadIndex() {
    if (m_File == INVALID_HANDLE_VALUE) return false;
    WDataFileHeader header;
    DWORD readed = 0;

    SetFilePointer(m_File, 0, NULL, FILE_BEGIN);
    if (!ReadFile(m_File, &header, sizeof(header), &readed, NULL)
        || readed != sizeof(header)) {
        return false;
    }
    if (header.id != 'WDFP') return false;

    m_Index.resize(header.number);
    SetFilePointer(m_File, header.offset, NULL, FILE_BEGIN);
    if (!ReadFile(
            m_File,
            m_Index.data(),
            sizeof(WDataFileIndex) * header.number,
            &readed,
            NULL
        ) || readed != sizeof(WDataFileIndex) * header.number) {
        return false;
    }
    return true;
}

size_t WDFUnpacker::GetFileCount() const {
    return m_Index.size();
}

const std::vector<WDataFileIndex>& WDFUnpacker::GetIndex() const {
    return m_Index;
}

// 单文件解压：分块读写 + 目录创建 + 增强日志
bool WDFUnpacker::ExtractFile(
    const std::string& innerNameA,
    const std::wstring& outPath,
    std::wstring& log
) {
    if (m_File == INVALID_HANDLE_VALUE) return false;

    // 1. 动态缓冲，完整拷贝名称，计算 UID
    std::vector<char> nameBuf(innerNameA.size() + 1);
    memcpy(nameBuf.data(), innerNameA.c_str(), innerNameA.size() + 1);
    string_adjust(nameBuf.data());
    DWORD uid = string_id(nameBuf.data());

    // 2. 查索引
    size_t idxPos = 0, cnt = m_Index.size();
    for (; idxPos < cnt; ++idxPos) {
        if (m_Index[idxPos].uid == uid) break;
    }
    if (idxPos == cnt) {
        wchar_t bufW[256];
        swprintf_s(bufW, 256,
            L"[Not found] %S (UID=0x%08X)",
            innerNameA.c_str(), uid);
        log = bufW;
        return false;
    }
    const WDataFileIndex& idx = m_Index[idxPos];

    // 3. 创建父目录
    {
        std::wstring tmp = outPath;
        int pos = tmp.find_last_of(L"\\/");
        if (pos != -1) {
            std::wstring parent = tmp.substr(0, pos);
            SHCreateDirectoryExW(NULL, parent.c_str(), NULL);
        }
    }

    // 4. 打开输出文件
    HANDLE hOut = CreateFileW(
        outPath.c_str(),
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    if (hOut == INVALID_HANDLE_VALUE) {
        DWORD err = GetLastError();
        wchar_t bufW[300];
        swprintf_s(bufW, 300,
            L"[Write failed] %s (UID=0x%08X) err=%u",
            outPath.c_str(), uid, err);
        log = bufW;
        return false;
    }

    // 5. 分块读写
    const DWORD CHUNK = 1024;
    BYTE* buffer = (BYTE*)malloc(CHUNK);
    if (!buffer) { CloseHandle(hOut); return false; }

    DWORD remain = idx.size;
    DWORD readBytes = 0, writtenBytes = 0;
    SetFilePointer(m_File, idx.offset, NULL, FILE_BEGIN);

    while (remain > 0) {
        DWORD toRead = (remain < CHUNK) ? remain : CHUNK;
        if (!ReadFile(m_File, buffer, toRead, &readBytes, NULL)) {
            DWORD err = GetLastError();
            wchar_t bufW[300];
            swprintf_s(bufW, 300,
                L"[Read failed] %S (UID=0x%08X) offset=0x%08X err=%u",
                innerNameA.c_str(), uid, idx.offset, err);
            log = bufW;
            free(buffer);
            CloseHandle(hOut);
            return false;
        }
        if (readBytes == 0) break;
        if (!WriteFile(hOut, buffer, readBytes, &writtenBytes, NULL)) {
            DWORD err = GetLastError();
            wchar_t bufW[300];
            swprintf_s(bufW, 300,
                L"[Write failed] %s (UID=0x%08X) err=%u",
                outPath.c_str(), uid, err);
            log = bufW;
            free(buffer);
            CloseHandle(hOut);
            return false;
        }
        remain -= readBytes;
    }

    free(buffer);
    CloseHandle(hOut);

    // 6. 成功日志
    {
        wchar_t bufW[200];
        swprintf_s(bufW,200,
            L"[Extracted] %S (UID=0x%08X)",
            innerNameA.c_str(), uid);
        log = bufW;
    }
    return true;
}

// 批量按 .lst 解包：长路径前缀 + 逐行处理 + 实时日志
bool WDFUnpacker::ExtractByLst(
    const std::wstring& lstPath,
    const std::wstring& outDir,
    std::vector<std::wstring>& logs,
    int& success,
    int& fail
) {
    HANDLE h = INVALID_HANDLE_VALUE;
    FILE* f = NULL;

    // 判断是否需要加长路径前缀
    std::wstring p = lstPath;
    bool useCreate = (p.size() >= MAX_PATH - 2 && p.compare(0,4,L"\\\\?\\") != 0);
    if (useCreate) {
        std::wstring lp;
        lp.reserve(p.size() + 4);
        lp.append(L"\\\\?\\");  // 对应 \\?\ 前缀
        lp += p;
        h = CreateFileW(
            lp.c_str(),
            GENERIC_READ,
            FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );
        if (h == INVALID_HANDLE_VALUE) {
            f = _wfopen(lstPath.c_str(), L"rb");
        }
    } else {
        f = _wfopen(lstPath.c_str(), L"rb");
    }

    // 读取整个 .lst
    char* buf = NULL;
    long size = 0;
    if (h != INVALID_HANDLE_VALUE) {
        DWORD sz = GetFileSize(h, NULL);
        buf = (char*)malloc(sz + 1);
        DWORD rd = 0;
        ReadFile(h, buf, sz, &rd, NULL);
        buf[rd] = '\0';
        size = rd;
        CloseHandle(h);
    } else if (f) {
        fseek(f, 0, SEEK_END);
        size = ftell(f);
        fseek(f, 0, SEEK_SET);
        buf = (char*)malloc(size + 1);
        fread(buf, 1, size, f);
        buf[size] = '\0';
        fclose(f);
    } else {
        return false;
    }

    success = fail = 0;
    char* ptr = buf;
    while (*ptr) {
        // 跳过空白
        while (*ptr && (*ptr=='\r'||*ptr=='\n'||*ptr==' '||*ptr=='\t'))
            ++ptr;
        if (!*ptr) break;

        // 取一行
        char* line = ptr;
        while (*ptr && *ptr!='\r' && *ptr!='\n') ++ptr;
        char saved = *ptr;
        *ptr = '\0';

        // 构造输出路径
        std::string inner = line;
        std::wstring wname = AnsiToWide(inner.c_str());
        for (size_t i = 0; i < wname.length(); ++i)
            if (wname[i] == L'/') wname[i] = L'\\';
        std::wstring outPath = outDir + L"\\" + wname;

        // 解包并打印日志
        std::wstring log;
        if (ExtractFile(inner, outPath, log))
            ++success;
        else
            ++fail;
        wprintf(L"%s\n", log.c_str());
        logs.push_back(log);

        *ptr = saved;
        if (*ptr) ++ptr;
    }

    free(buf);
    return true;
}
