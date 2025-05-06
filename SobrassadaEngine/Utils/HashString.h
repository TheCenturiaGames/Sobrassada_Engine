#pragma once

#include <string>

struct HashString
{
    size_t hash = 0;
    std::string original;

    HashString() = default;
    HashString(const std::string& str)
    {
        original = str;
        hash     = std::hash<std::string> {}(str);
    }

    bool operator==(const HashString& other) const { return hash == other.hash; }
    bool operator!=(const HashString& other) const { return !(*this == other); }
    bool operator<(const HashString& other) const { return hash < other.hash; }
    const std::string& GetString() const { return original; }
};