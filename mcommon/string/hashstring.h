#pragma once

#include <unordered_map>

// A simple conversion of string to a hash int.  Useful for quick string lookups.
// Debug version keeps the string
struct HashString
{
    HashString(uint32_t hash = 0);
    HashString(const char* pszString);
    HashString(const std::string& str);
    uint32_t m_hash;

    static const HashString& Empty() 
    {
        static HashString empty;
        return empty;
    }

    bool operator < (const HashString& rhs) const { return m_hash < rhs.m_hash; }
    std::string ToString() const;

    static std::unordered_map<uint32_t, std::string> StringDatabase;
};

inline std::ostream& operator<< (std::ostream &out, const HashString &t)
{
    out << t.ToString();
    return out;
}
