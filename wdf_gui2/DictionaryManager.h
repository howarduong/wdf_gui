#pragma once
#include <set>
#include <string>
#include <vector>

class DictionaryManager {
public:
    DictionaryManager();
    bool Load(const std::wstring& filename);
    bool Save(const std::wstring& filename) const;
    void AddPathFragments(const std::wstring& path);
    void MergeFragments(const std::set<std::wstring>& newFrags);
    const std::set<std::wstring>& GetFragments() const;
    void SetParams(int depth, int tries, const std::vector<std::wstring>& exts);
    int GetDepth() const;
    int GetTries() const;
    const std::vector<std::wstring>& GetExts() const;
    void ParseParamsLine(const std::wstring& line);
    std::wstring MakeParamsLine() const;
    const std::vector<std::wstring>& GetSampleFormats() const;
private:
    std::set<std::wstring> fragments;
    int depth;
    int tries;
    std::vector<std::wstring> exts;
    std::vector<std::wstring> sampleFormats;
}; 