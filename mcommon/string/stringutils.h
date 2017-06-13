#pragma once

namespace StringUtils
{


std::string ReplaceString(std::string subject, const std::string& search, const std::string& replace);
std::vector<std::string> Split(std::string s, const char delimiter);

// trim from beginning of string (left)
inline std::string& LTrim(std::string& s, const char* t = " \t\n\r\f\v")
{
    s.erase(0, s.find_first_not_of(t));
    return s;
}

// trim from end of string (right)
inline std::string& RTrim(std::string& s, const char* t = " \t\n\r\f\v")
{
    s.erase(s.find_last_not_of(t) + 1);
    return s;
}

// trim from both ends of string (left & right)
inline std::string& Trim(std::string& s, const char* t = " \t\n\r\f\v")
{
    return LTrim(RTrim(s, t), t);
}

template<typename T>
std::string toString(const T &t) {
    std::ostringstream oss;
    oss << t;
    return oss.str();
}

template<typename T>
T fromString(const std::string& s) {
    std::istringstream stream(s);
    T t;
    stream >> t;
    return t;
}
}