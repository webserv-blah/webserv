#include "utilities.hpp"

namespace utils {

    int stoi(const std::string& str) {
        char* end;
        errno = 0; // Reset errno before the call
        long result = std::strtol(str.c_str(), &end, 10);

        // Check for various possible errors
        if (errno == ERANGE || result > INT_MAX || result < INT_MIN) {
            throw std::overflow_error("Integer overflow");
        }
        if (end == str.c_str() || *end != '\0') {
            throw std::invalid_argument("Invalid integer format");
        }

        return static_cast<int>(result);
    }

    template <typename InputIterator, typename Predicate>
    bool all_of(InputIterator first, InputIterator last, Predicate pred) {
        for ( ; first != last; ++first) {
            if (!pred(*first)) {
                return false;
            }
        }
        return true;
    }

}
