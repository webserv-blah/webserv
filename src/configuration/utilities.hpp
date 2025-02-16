#ifndef UTILITIES_HPP
# define UTILITIES_HPP

#include <string>
#include <stdexcept>
#include <cerrno>
#include <climits>

// 유틸리티 함수들을 담는 네임스페이스
namespace utils {

    // 문자열을 정수로 변환하는 함수
    int stoi(const std::string& str);

    // 입력 반복자 [first, last) 범위 내의 모든 요소가
    // 주어진 조건(pred)을 만족하는지 확인하는 템플릿 함수
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

#endif