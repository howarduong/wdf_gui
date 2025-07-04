#include "StdAfx.h"
#include "DictionaryManager.h"
#include <fstream>
#include <sstream>
#include <algorithm>

static const wchar_t* defaultSamples[] = {
    L"xxx.xxx",
    L"xxx/xxx.xxx",
    L"xxx/xxx/xxx.xxx",
    L"xxx/xxx/xxx/xxx.xxx",
    L"xxx/xxx/xxx/xxx/xxx.xxx",
    L"xxx/xxx/xxx/xxx/xxx/xxx.xxx"
};

DictionaryManager::DictionaryManager() : depth(3), tries(500) {
    exts.push_back(L"png");
    exts.push_back(L"dds");
    exts.push_back(L"bmp");
}

bool DictionaryManager::Load(const std::wstring& filename) {
    fragments.clear();
    sampleFormats.clear();
    std::wifstream fin(filename.c_str());
    if (!fin) {
        // 文件不存在，首次创建，写入默认示例格式
        for (int i=0; i<6; ++i) sampleFormats.push_back(defaultSamples[i]);
        return false;
    }
    std::wstring line;
    int lineNo = 0;
    if (std::getline(fin, line)) {
        ParseParamsLine(line);
        ++lineNo;
    }
    // 读取最多6行示例格式
    for (int i=0; i<6 && std::getline(fin, line); ++i) {
        if (!line.empty() && line.find(L"xxx") != std::wstring::npos)
            sampleFormats.push_back(line);
        else
            break;
        ++lineNo;
    }
    // 如果没有示例格式，补充默认
    if (sampleFormats.empty()) {
        for (int i=0; i<6; ++i) sampleFormats.push_back(defaultSamples[i]);
    }
    // 跳过注释行（如#maps等）
    while (std::getline(fin, line)) {
        if (!line.empty() && line[0] != L'#') {
            fragments.insert(line);
        }
    }
    return true;
}

bool DictionaryManager::Save(const std::wstring& filename) const {
    std::wofstream fout(filename.c_str());
    if (!fout) return false;
    // 自动补充默认示例格式
    if (sampleFormats.empty()) {
        for (int i=0; i<6; ++i) ((DictionaryManager*)this)->sampleFormats.push_back(defaultSamples[i]);
    }
    fout << MakeParamsLine() << std::endl;
    for (size_t i=0; i<sampleFormats.size(); ++i) {
        fout << sampleFormats[i] << std::endl;
    }
    std::set<std::wstring>::const_iterator it = fragments.begin();
    for (; it != fragments.end(); ++it) {
        fout << *it << std::endl;
    }
    return true;
}

void DictionaryManager::AddPathFragments(const std::wstring& path) {
    size_t start = 0, end = 0;
    while (end < path.size()) {
        wchar_t c = path[end];
        if (c == L'\\' || c == L'/' || c == L'.' || c == L'-' || c == L'_') {
            if (end > start)
                fragments.insert(path.substr(start, end - start));
            start = end + 1;
        }
        end++;
    }
    if (end > start)
        fragments.insert(path.substr(start, end - start));
}

void DictionaryManager::MergeFragments(const std::set<std::wstring>& newFrags) {
    fragments.insert(newFrags.begin(), newFrags.end());
}

const std::set<std::wstring>& DictionaryManager::GetFragments() const {
    return fragments;
}

void DictionaryManager::SetParams(int d, int t, const std::vector<std::wstring>& e) {
    depth = d;
    tries = t;
    exts = e;
}

int DictionaryManager::GetDepth() const { return depth; }
int DictionaryManager::GetTries() const { return tries; }
const std::vector<std::wstring>& DictionaryManager::GetExts() const { return exts; }
const std::vector<std::wstring>& DictionaryManager::GetSampleFormats() const { return sampleFormats; }

void DictionaryManager::ParseParamsLine(const std::wstring& line) {
    depth = 3; tries = 500; exts.clear();
    size_t pos = 0, next = 0;
    while (pos < line.size()) {
        next = line.find(L';', pos);
        std::wstring token = line.substr(pos, next-pos);
        size_t eq = token.find(L'=');
        if (eq != std::wstring::npos) {
            std::wstring key = token.substr(0, eq);
            std::wstring val = token.substr(eq+1);
            if (key == L"depth") depth = _wtoi(val.c_str());
            else if (key == L"tries") tries = _wtoi(val.c_str());
            else if (key == L"exts") {
                size_t s=0,e=0;
                while (e < val.size()) {
                    if (val[e]==L',') {
                        if (e>s) exts.push_back(val.substr(s,e-s));
                        s=e+1;
                    }
                    e++;
                }
                if (e>s) exts.push_back(val.substr(s,e-s));
            }
        }
        if (next==std::wstring::npos) break;
        pos = next+1;
    }
    if (exts.empty()) {
        exts.push_back(L"png");
        exts.push_back(L"dds");
        exts.push_back(L"bmp");
    }
}

std::wstring DictionaryManager::MakeParamsLine() const {
    std::wstringstream ss;
    ss << L"depth=" << depth << L";tries=" << tries << L";exts=";
    for (size_t i=0; i<exts.size(); ++i) {
        if (i>0) ss << L",";
        ss << exts[i];
    }
    return ss.str();
} 