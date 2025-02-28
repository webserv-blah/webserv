#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <cstring>
#include <cerrno>
#include <limits>
#include <csignal>
#include "../include/commonEnums.hpp"

// 전역 함수
void signalHandler(int signum);
void setupSignalHandlers();

namespace webserv
{

void stringTrim(std::string& str);
void stringToUppercase(std::string& str);
void stringToLowercase(std::string& str);
void stringSplit(const std::string& str, char delim, std::vector<std::string>& tokens);
void appendString(std::string& target, const std::vector<char>& source, size_t len);
void intToString(int num, std::string& str);

std::string errorMessage(EnumErrorLevel level, const std::string& cause, 
                        const std::string& context, const std::string& source);

} // namespace webserv

namespace utils 
{
    int stoi(const std::string& str);
    size_t sto_size_t(const std::string& str);
    std::string strtrim(const std::string& str);
    std::string size_t_tos(size_t num);
    
    template <typename InputIt, typename UnaryPredicate>
    bool all_of(InputIt first, InputIt last, UnaryPredicate p) {
        for (; first != last; ++first) {
            if (!p(*first)) {
                return false;
            }
        }
        return true;
    }
}

#endif