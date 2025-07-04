#include "stdafx.h"
#include "wdf_unpacker.h"
#include <fstream>
#include <algorithm>
#include <cstring>
#include <string>

static DWORD WString2ID(const std::wstring& str) {
    // 复刻wpp16的哈希算法（简化示例，需用原版算法替换）
    DWORD hash = 0;
    for (size_t i = 0; i < str.length(); ++i) {
        wchar_t ch = str[i];
        wchar_t c = (ch >= L'A' && ch <= L'Z') ? (ch + 32) : ch;
        if (c == L'/') c = L'\\';
        hash = hash * 131 + c;
    }
    return hash;
}

// WPP16 original hash algorithm (string_id and string2id)
uint32_t ws_string2id(const char* str) {
    uint32_t ebp = 0xF4FA8928;
    uint32_t ecx = 0x37A8470E;
    uint32_t edi = 0x7758B42B;
    uint32_t ebx, eax, edx;
    const unsigned char* p = (const unsigned char*)str;
    while (*p) {
        ebx = 0x267B0B11;
        ebp = (ebp << 1) | (ebp >> 31);
        ebx ^= ebp;
        eax = 0;
        for (int i = 0; i < 4 && p[i]; ++i)
            eax |= (p[i] << (i * 8));
        p += (eax & 0xFF000000) ? 4 : (eax & 0xFF0000) ? 3 : (eax & 0xFF00) ? 2 : 1;
        if (!*p) {
            if (!(eax & 0xFF00)) eax &= 0xFF;
            else if (!(eax & 0xFF0000)) eax &= 0xFFFF;
            else if (!(eax & 0xFF000000)) eax &= 0xFFFFFF;
        }
        ecx ^= eax;
        edx = ebx;
        edi ^= eax;
        edx += edi;
        edx |= 0x2040801;
        edx &= 0xBFEF7FDF;
        uint64_t tmp = (uint64_t)ecx * edx;
        eax = (uint32_t)tmp + edx;
        edx = ebx;
        eax += 0;
        edx += ecx;
        edx |= 0x804021;
        edx &= 0x7DFEFBFF;
        ecx = eax;
        tmp = (uint64_t)edi * edx;
        eax = (uint32_t)tmp;
        edx += edx;
        eax += edx;
        if (eax < edx) eax += 2;
        edi = eax;
    }
    ebx = 0x267B0B11;
    ebp = (ebp << 1) | (ebp >> 31);
    ebx ^= ebp;
    ecx ^= 0x9BE74448;
    edx = ebx;
    edi ^= 0x9BE74448;
    edx += edi;
    edx |= 0x2040801;
    edx &= 0xBFEF7FDF;
    uint64_t tmp = (uint64_t)ecx * edx;
    eax = (uint32_t)tmp + edx;
    edx = ebx;
    eax += 0;
    edx += ecx;
    edx |= 0x804021;
    edx &= 0x7DFEFBFF;
    ecx = eax;
    tmp = (uint64_t)edi * edx;
    eax = (uint32_t)tmp;
    edx += edx;
    eax += edx;
    if (eax < edx) eax += 2;
    edi = eax;
    ebx = 0x267B0B11;
    ebp = (ebp << 1) | (ebp >> 31);
    ebx ^= ebp;
    ecx ^= 0x66F42C48;
    edx = ebx;
    edi ^= 0x66F42C48;
    edx += edi;
    edx |= 0x2040801;
    edx &= 0xBFEF7FDF;
    tmp = (uint64_t)ecx * edx;
    eax = (uint32_t)tmp + edx;
    edx = ebx;
    eax += 0;
    edx += ecx;
    edx |= 0x804021;
    edx &= 0x7DFEFBFF;
    ecx = eax;
    tmp = (uint64_t)edi * edx;
    eax = (uint32_t)tmp;
    edx += edx;
    eax += edx;
    if (eax < edx) eax += 2;
    edx = ecx;
    eax ^= ecx;
    return eax;
}

uint32_t ws_string_id(const std::string& filename) {
    char buffer[256];
    int i;
    for (i = 0; i < (int)filename.size() && i < 255; i++) {
        if (filename[i] >= 'A' && filename[i] <= 'Z')
            buffer[i] = filename[i] + 'a' - 'A';
        else if (filename[i] == '/')
            buffer[i] = '\\';
        else
            buffer[i] = filename[i];
    }
    buffer[i] = 0;
    return ws_string2id(buffer);
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

bool WDFUnpacker::ExtractFile(const std::wstring& innerName, const std::wstring& outPath, std::wstring& log) {
    if (m_File == INVALID_HANDLE_VALUE) return false;
    std::string nameA(innerName.begin(), innerName.end());
    DWORD uid = ws_string_id(nameA);
    auto it = std::find_if(m_Index.begin(), m_Index.end(), [uid](const WDataFileIndex& idx) { return idx.uid == uid; });
    if (it == m_Index.end()) {
        log = L"[Not found] " + innerName;
        return false;
    }
    SetFilePointer(m_File, it->offset, 0, FILE_BEGIN);
    std::vector<char> buffer(it->size);
    DWORD readed = 0;
    if (!ReadFile(m_File, buffer.data(), it->size, &readed, NULL) || readed != it->size) {
        log = L"[Read failed] " + innerName;
        return false;
    }
    std::ofstream fout(outPath, std::ios::binary);
    if (!fout) {
        log = L"[Write failed] " + outPath;
        return false;
    }
    fout.write(buffer.data(), it->size);
    fout.close();
    log = L"[Extracted] " + innerName;
    return true;
}

bool WDFUnpacker::ExtractByLst(const std::wstring& lstPath, const std::wstring& outDir, std::vector<std::wstring>& logs, int& success, int& fail) {
    std::wifstream fin(lstPath);
    if (!fin) return false;
    std::wstring line;
    success = 0; fail = 0;
    while (std::getline(fin, line)) {
        if (line.empty()) continue;
        std::wstring outPath = outDir + L"\\" + line;
        std::wstring log;
        if (ExtractFile(line, outPath, log))
            ++success;
        else
            ++fail;
        logs.push_back(log);
    }
    return true;
} 