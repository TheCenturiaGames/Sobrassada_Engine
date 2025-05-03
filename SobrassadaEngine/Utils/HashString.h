#pragma once
#pragma message("Incluyendo HashString.h")

#include <string>

struct HashString
{
    size_t hash_size = 0;
    std::string original;

    HashString() = default;
    HashString(const std::string& str)
    {
        original = str;
        hash_size     = std::hash<std::string> {}(str);
    }

    bool operator==(const HashString& other) const { return hash_size == other.hash_size; }
    bool operator!=(const HashString& other) const { return !(*this == other); }
    bool operator<(const HashString& other) const { return hash_size < other.hash_size; }
    const std::string& GetString() const { return original; }
    std::string& GetString() { return original; }
    const bool empty() const { return original.empty(); }
    const char* c_str() const { return original.c_str(); }
    const size_t size() const { return original.size(); }
};