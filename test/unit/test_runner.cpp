#include "TestFramework.hpp"
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <cerrno>

// Stub for global variable used in signal handlers
volatile bool globalServerRunning = false;

int main(int /*argc*/, char* /*argv*/[]) {
    std::cout << "Running webserv unit tests..." << std::endl;
    
    // 현재 작업 디렉토리를 프로젝트 루트로 변경
    if (chdir("../..") != 0) {
        std::cerr << "Failed to change directory: " << strerror(errno) << std::endl;
        return 1;
    }
    
    // All tests are automatically registered via the TEST_CASE macro
    return RUN_ALL_TESTS();
}