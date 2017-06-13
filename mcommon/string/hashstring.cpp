#include "mcommon.h"
#include "hashstring.h"
#include "murmur_hash.h"

std::unordered_map<uint32_t, std::string> HashString::StringDatabase;

HashString::HashString(uint32_t hash)
    : m_hash(hash)
{
}

HashString::HashString(const char* pszString)
{
    m_hash = murmur_hash(pszString, int(strlen(pszString)), 0);
    StringDatabase[m_hash] = pszString;
}

HashString::HashString(const std::string& str)
{
    m_hash = murmur_hash(str.c_str(), int(str.length()), 0);
    StringDatabase[m_hash] = str;
}

std::string HashString::ToString() const
{
    auto itrFound = StringDatabase.find(m_hash);
    if (itrFound != StringDatabase.end())
    {
        return itrFound->second;
    }
    return std::to_string(m_hash);
}

