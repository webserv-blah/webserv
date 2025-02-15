#ifndif UTILITIES_HPP
# define UTILITIES_HPP

#include <string>
#include <stdexcept>
#include <cerrno>
#include <climits>

namespace utils {

    int stoi(const std::string& str);

    template <typename InputIterator, typename Predicate>
    bool all_of(InputIterator first, InputIterator last, Predicate pred);

}

#define UTILITIES_HPP