// test_timeout_handler.cpp
#include <cassert>
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <time.h>
#include <unistd.h>

#include "TimeoutHandler.hpp"


// --- Dummy 클래스 정의 ---
// EventHandler 인터페이스의 최소한의 구현체
class DummyEventHandler : public EventHandler {
public:
    bool errorHandled = false;
    int  handledErrorCode = 0;
    int  handledclientFd = -1; // ClientSession 내부에 clientFd를 저장한다고 가정

    virtual void handleError(int code, ClientSession& client) override {
        errorHandled = true;
        handledErrorCode = code;
        // 여기서는 client의 주소 대신 clientFd값을 구분하기 위해 다운캐스팅 (DummyClientSession 사용)
        handledclientFd = *((int*)&client); // (테스트용 단순 처리)
    }
};

// Demultiplexer 인터페이스의 최소한의 구현체
class DummyDemultiplexer : public Demultiplexer {
public:
    std::vector<int> removedSockets;
    virtual void removeSocket(int clientFd) override {
        removedSockets.push_back(clientFd);
    }
};


// --- Helper: std::cerr 캡쳐용 ---
// 에러 메시지 출력을 캡쳐하여 테스트 시 에러 메시지가 발생했는지 검증합니다.
class CerrCapture {
public:
    CerrCapture() : oldBuf(std::cerr.rdbuf(capture.rdbuf())) { }
    ~CerrCapture() { std::cerr.rdbuf(oldBuf); }
    std::string getCaptured() const { return capture.str(); }
private:
    std::stringstream capture;
    std::streambuf* oldBuf;
};

// --- 테스트 함수들 ---

// 1. Add Connection 테스트
void testAddConnection() {
    std::cout << "📌[Test] testAddConnection 시작" << std::endl;
    TimeoutHandler handler;
    int clientFd = 1;  // 초기 시간 설정
    handler.addConnection(clientFd);
    // 바로 updateActivity 호출 시 에러가 없어야 함 (내부에 등록된 clientFd에 대해)
    handler.updateActivity(clientFd);
    std::cout << "✅[Test] testAddConnection passed." << std::endl;
}

// 2. Update Activity 테스트
void testUpdateActivity() {
    std::cout << "📌[Test] testUpdateActivity 시작" << std::endl;
    TimeoutHandler handler;
    int clientFd = 2;
    handler.addConnection(clientFd);

    // 시간이 진행되었다고 가정
    handler.updateActivity(clientFd);
    // checkTimeouts를 호출할 때, 아직 만료전이므로 처리되지 않아야 함.
    DummyEventHandler dummyEH;
    DummyDemultiplexer dummyDemux;
    ClientManager CM;
    CM.addClient(clientFd);
    handler.checkTimeouts(dummyEH, dummyDemux, CM);
    // 에러 처리되지 않아야 하므로 errorHandled는 false
    assert(dummyEH.errorHandled == false);
    std::cout << "✅[Test] testUpdateActivity passed." << std::endl;
}

// 3. Remove Connection 테스트
void testRemoveConnection() {
    std::cout << "📌[Test] testRemoveConnection 시작" << std::endl;
    TimeoutHandler handler;
    int clientFd = 3;
    handler.addConnection(clientFd);

    // 외부 제거
    handler.removeConnection(clientFd);

    // 제거된 clientFd에 대해 updateActivity 호출 시 에러 메시지가 출력되어야 함.
    {
        CerrCapture capture;
        handler.updateActivity(clientFd);
        std::string errMsg = capture.getCaptured();
        assert(errMsg.find("Connection Not Found") != std::string::npos);
    }
    std::cout << "✅[Test] testRemoveConnection passed." << std::endl;
}

// 4. Check Timeouts 테스트 (만료된 연결 처리)
void testCheckTimeouts_expired() {
    std::cout << "📌[Test] testCheckTimeouts_expired 시작" << std::endl;
    TimeoutHandler handler;
    int clientFd = 4;
    handler.addConnection(clientFd);
    
    // 클라이언트 추가 (checkTimeouts()에서 accessClientSession()가 유효한 포인터를 반환하도록)
    ClientManager CM;
    CM.addClient(clientFd);

    DummyEventHandler dummyEH;
    DummyDemultiplexer dummyDemux;
    
    //만료 시각
    sleep(3);
    handler.checkTimeouts(dummyEH, dummyDemux, CM);
    
    // handleError가 호출되었고, 소켓 및 클라이언트 삭제가 수행되었는지 확인
    assert(dummyEH.errorHandled == true);
    assert(dummyEH.handledErrorCode == 408);
    // DummyDemultiplexer에서 제거된 clientFd 확인
    assert(!dummyDemux.removedSockets.empty());
    assert(dummyDemux.removedSockets[0] == clientFd);
    // ClientManager에서 해당 클라이언트가 제거되었어야 함.
    assert(CM.accessClientSession(clientFd) == nullptr);
    std::cout << "✅[Test] testCheckTimeouts_expired passed." << std::endl;
}

// 5. Check Timeouts 테스트 (만료되지 않은 연결)
void testCheckTimeouts_notExpired() {
    std::cout << "📌[Test] testCheckTimeouts_notExpired 시작" << std::endl;
    TimeoutHandler handler;
    int clientFd = 5;
    handler.addConnection(clientFd);

    ClientManager CM;
    CM.addClient(clientFd);

    DummyEventHandler dummyEH;
    DummyDemultiplexer dummyDemux;
    
    handler.checkTimeouts(dummyEH, dummyDemux, CM);
    
    // 만료되지 않았으므로 error 처리나 제거가 발생하지 않아야 함.
    assert(dummyEH.errorHandled == false);
    assert(dummyDemux.removedSockets.empty());
    // ClientManager에 여전히 클라이언트가 존재해야 함.
    assert(CM.accessClientSession(clientFd) != nullptr);
    std::cout << "✅[Test] testCheckTimeouts_notExpired passed." << std::endl;
}

// 6. 존재하지 않는 연결에 대한 에러 처리 테스트
void testInvalidOperations() {
    std::cout << "📌[Test] testInvalidOperations 시작" << std::endl;
    TimeoutHandler handler;
    int invalidclientFd = 999;

    {
        CerrCapture capture;
        handler.updateActivity(invalidclientFd);
        std::string errMsg = capture.getCaptured();
        assert(errMsg.find("Connection Not Found") != std::string::npos);
    }
    {
        CerrCapture capture;
        handler.removeConnection(invalidclientFd);
        std::string errMsg = capture.getCaptured();
        assert(errMsg.find("[Error][TimeoutHandler][removeConnection] Fd Not Found") != std::string::npos);
    }
    std::cout << "✅[Test] testInvalidOperations passed." << std::endl;
}

// --- main ---
int main() {
    testAddConnection();
    testUpdateActivity();
    testRemoveConnection();
    testCheckTimeouts_expired();
    testCheckTimeouts_notExpired();
    testInvalidOperations();

    std::cout << "All TimeoutHandler tests passed." << std::endl;
    return 0;
}
