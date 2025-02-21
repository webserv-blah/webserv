// dummy_close.hpp
#ifndef DUMMY_CLOSE_HPP
#define DUMMY_CLOSE_HPP

// 테스트 환경에서는 실제 소켓 close()를 호출하지 않도록 dummy 구현을 제공합니다.
extern "C" {
    int close(int fd) {
        // 실제로 닫지 않고 0 리턴
        return 0;
    }
}

#endif // DUMMY_CLOSE_HPP
