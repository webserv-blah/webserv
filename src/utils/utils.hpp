#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <cstring>
#include <cerrno>
#include <limits>
#include <climits>
#include <csignal>
#include <cstdlib>
#include "../include/commonEnums.hpp"

// 전역 함수
void signalHandler(int signum);
void setupSignalHandlers();

namespace webserv {

std::string errorMessage(EnumErrorLevel level, const std::string& cause, 
                        const std::string& context, const std::string& source);

} // namespace webserv

namespace utils 
{
    int stoi(const std::string& str);
    size_t sto_size_t(const std::string& str);
    std::string strtrim(const std::string& str);
    std::string size_t_tos(size_t num);
    std::string int_tos(int num);
    
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