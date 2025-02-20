#pragma once

#include <string>       // std::string
#include <stdexcept>    // std::invalid_argument, std::overflow_error
#include <cerrno>       // errno
#include <climits>      // INT_MAX, INT_MIN
#include <csignal>
#include <iostream>
#include <limits>
#include <csignal>
#include <iostream>
#include <limits>

// 유틸리티 함수들을 담는 네임스페이스
namespace utils {

	// 문자열을 int로 변환하는 함수
	int stoi(const std::string& str);
	// 문자열을 size_t로 변환하는 함수
	size_t stosizet(const std::string& str);

	// 입력 반복자 [first, last) 범위 내의 모든 요소가
	// 주어진 조건(pred)을 만족하는지 확인하는 템플릿 함수
	template <typename InputIterator, typename Predicate>
	bool	all_of(InputIterator first, InputIterator last, Predicate pred) {
		for ( ; first != last; ++first) {
			if (!pred(*first)) {
				return false;
			}
		}
		return true;
	}

}

// 시그널 핸들러 함수
void signalHandler(int signum);
// 시그널 핸들러 설정 함수
void setupSignalHandlers();