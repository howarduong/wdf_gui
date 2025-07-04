#include "stdafx.h"
#include "wdf_unpacker.h"
#include <algorithm>
#include <vector>
#include <string>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <direct.h>
#include <shlobj.h> // 新增头文件

// wpp16原版string_adjust
static void string_adjust(char* p) {
    for (int i = 0; p[i]; i++) {
        if (p[i] >= 'A' && p[i] <= 'Z') p[i] += 'a' - 'A';
        else if (p[i] == '/') p[i] = '\\';
    }
}

// wpp16原版string_id
static DWORD string_id(const char* str) {
    int i;
    unsigned int v;
    static unsigned m[70];
    strncpy((char*)m, str, 256);
    for (i = 0; i < 256 / 4 && m[i]; i++);
    m[i++] = 0x9BE74448, m[i++] = 0x66F42C48;
    v = 0xF4FA8928;
    __asm {
        mov esi, 0x37A8470E
        mov edi, 0x7758B42B
        xor ecx, ecx
    _loop:
        mov ebx, 0x267B0B11
        rol v, 1
        lea eax, m
        xor ebx, v
        mov eax, [eax + ecx * 4]
        mov edx, ebx
        xor esi, eax
        xor edi, eax
        add edx, edi
        or edx, 0x2040801
        and edx, 0xBFEF7FDF
        mov eax, esi
        mul edx
        adc eax, edx
        mov edx, ebx
        adc eax, 0
        add edx, esi
        or edx, 0x804021
        and edx, 0x7DFEFBFF
        mov esi, eax
        mov eax, edi
        mul edx
        add edx, edx
        adc eax, edx
        jnc _skip
        add eax, 2
    _skip:
        inc ecx
        mov edi, eax
        cmp ecx, i
        jnz _loop
        xor esi, edi
        mov v, esi
    }
    return v;
}

WDFUnpacker::WDFUnpacker() : m_File(INVALID_HANDLE_VALUE) {}

bool WDFUnpacker::OpenWdf(const std::wstring& wdfPath) {
    Close();
    m_File = CreateFileW(wdfPath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
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

bool WDFUnpacker::LoadIndex() {
    if (m_File == INVALID_HANDLE_VALUE) return false;
    WDataFileHeader header;
    DWORD readed = 0;
    SetFilePointer(m_File, 0, 0, FILE_BEGIN);
    if (!ReadFile(m_File, &header, sizeof(header), &readed, NULL) || readed != sizeof(header)) return false;
    if (header.id != 'WDFP') return false;
    m_Index.resize(header.number);
    SetFilePointer(m_File, header.offset, 0, FILE_BEGIN);
    if (!ReadFile(m_File, m_Index.data(), sizeof(WDataFileIndex) * header.number, &readed, NULL) || readed != sizeof(WDataFileIndex) * header.number) return false;
    return true;
}

size_t WDFUnpacker::GetFileCount() const {
    return m_Index.size();
}

const std::vector<WDataFileIndex>& WDFUnpacker::GetIndex() const {
    return m_Index;
}

// 用char*方式查找和导出
bool WDFUnpacker::ExtractFile(const std::string& innerNameA, const std::wstring& outPath, std::wstring& log) {
    if (m_File == INVALID_HANDLE_VALUE) return false;
    char nameBuf[260];
    strncpy(nameBuf, innerNameA.c_str(), 259);
    nameBuf[259] = 0;
    string_adjust(nameBuf);
    DWORD uid = string_id(nameBuf);
    auto it = std::find_if(m_Index.begin(), m_Index.end(), [uid](const WDataFileIndex& idx) { return idx.uid == uid; });
    if (it == m_Index.end()) {
        CStringW msg;
        msg.Format(L"[Not found] %S", innerNameA.c_str());
        log = msg;
        return false;
    }
    SetFilePointer(m_File, it->offset, 0, FILE_BEGIN);
    std::vector<char> buffer(it->size);
    DWORD readed = 0;
    if (!ReadFile(m_File, buffer.data(), it->size, &readed, NULL) || readed != it->size) {
        CStringW msg;
        msg.Format(L"[Read failed] %S", innerNameA.c_str());
        log = msg;
        return false;
    }
    // 递归创建父目录
    CStringW dir = outPath.c_str();
    int pos = dir.ReverseFind(L'\\');
    if (pos > 0) {
        CStringW parent = dir.Left(pos);
        SHCreateDirectoryExW(NULL, parent, NULL);
    }
    FILE* fout = _wfopen(outPath.c_str(), L"wb");
    if (!fout) {
        CStringW msg;
        msg.Format(L"[Write failed] %s", outPath.c_str());
        log = msg;
        return false;
    }
    fwrite(buffer.data(), 1, it->size, fout);
    fclose(fout);
    CStringW msg;
    msg.Format(L"[Extracted] %S", innerNameA.c_str());
    log = msg;
    return true;
}

// 完全仿照wpp16的LST读取和hash方式
bool WDFUnpacker::ExtractByLst(const std::wstring& lstPath, const std::wstring& outDir, std::vector<std::wstring>& logs, int& success, int& fail) {
    FILE* f = _wfopen(lstPath.c_str(), L"rb");
    if (!f) return false;
    fseek(f, 0, SEEK_END);
    int size = ftell(f);
    fseek(f, 0, SEEK_SET);
    std::vector<char> buffer(size + 1);
    fread(buffer.data(), 1, size, f);
    fclose(f);
    buffer[size] = 0;
    int i = 0;
    char* p = buffer.data();
    char* line = nullptr;
    success = 0; fail = 0;
    bool in_line = false;
    for (i = 0; i < size; i++) {
        if (in_line) {
            if (p[i] <= 32 && p[i] >= 0) {
                p[i] = 0;
                std::wstring outPath = outDir;
                outPath += L"\\";
                outPath += CStringW(line);
                std::wstring log;
                if (ExtractFile(line, outPath, log))
                    ++success;
                else
                    ++fail;
                logs.push_back(log);
                in_line = false;
            }
        } else {
            if (p[i] > 32 || p[i] < 0) {
                line = &p[i];
                in_line = true;
            }
        }
    }
    return true;
}