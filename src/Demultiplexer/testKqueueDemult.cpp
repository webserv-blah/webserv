#include <iostream>
#include <cassert>
#include <set>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <csignal>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <unistd.h>
#include "KqueueDemultiplexer.hpp"

// 전역 변수: 타임아웃 발생 여부를 체크하기 위한 플래그
volatile sig_atomic_t timeoutOccurred = 0;

/*
 * SIGALRM 시그널 핸들러
 * 타임아웃이 발생했을 때, 전역 변수 timeoutOccurred를 설정합니다.
 */
void alarm_handler(int signum) {
    timeoutOccurred = 1;
}

/*
 * 헬퍼 함수: create_listening_socket
 * 루프백(127.0.0.1) 주소와 지정된(혹은 임의의) 포트번호로 리스닝 소켓을 생성합니다.
 * 포트번호 0을 사용하면 운영체제가 임의의 사용 가능한 포트를 할당합니다.
 */
int create_listening_socket(int port = 0) {
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    assert(listen_fd != -1); // 소켓 생성 실패 시 프로그램 종료
    int opt = 1;
    // 소켓 옵션 설정: 주소 재사용 가능하도록 함
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
    
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;                 // IPv4 사용
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // 루프백 주소 (127.0.0.1)
    addr.sin_port = htons(port);                // 지정된 포트 (0이면 임의 할당)
    
    int ret = bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr));
    assert(ret != -1); // 바인드 실패 시 종료
    
    ret = listen(listen_fd, 5); // 리스닝 큐 길이 5로 설정
    assert(ret != -1); // listen 실패 시 종료
    return listen_fd;
}

/*
 * 헬퍼 함수: get_listening_port
 * 소켓의 실제 바인드된 포트 번호를 반환합니다.
 * 주로 포트번호 0을 사용해 임의 포트를 할당받은 경우, 해당 포트번호를 확인할 때 사용합니다.
 */
int get_listening_port(int fd) {
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    int ret = getsockname(fd, (struct sockaddr*)&addr, &len);
    assert(ret != -1); // 실패 시 종료
    return ntohs(addr.sin_port);
}

/*
 * 테스트 1: Client가 보낸 메시지로 인한 서버측 READ_EVENT 테스트
 * - 리스닝 소켓을 KqueueDemultiplexer에 등록
 * - fork()를 통해 생성된 자식 프로세스(클라이언트)가 접속 후,
 *   클라이언트가 보낸 메시지로 인해 서버측의 client_fd에서 READ_EVENT가 발생하는지 확인합니다.
 */
void test_client_to_server_read_event() {
    std::cout << "📌[TEST] Running Client -> Server Read Event Test\n";

    // 1. 리스닝 소켓 생성 및 포트번호 획득
    int listen_fd = create_listening_socket();
    int port = get_listening_port(listen_fd);

    // 2. 리스닝 소켓을 이벤트 등록할 set에 추가하고 Demultiplexer 생성
    std::set<int> listenSet;
    listenSet.insert(listen_fd);
    KqueueDemultiplexer demux(listenSet);

    // 3. fork()를 사용하여 자식 프로세스를 생성 (자식: 클라이언트 역할)
    pid_t pid = fork();
    if (pid == 0) {
        // ── 자식 프로세스 (Client) ──────────────────────────────
        // 서버가 준비될 시간을 주기 위해 잠시 대기
        sleep(1);

        // 클라이언트 소켓 생성 및 서버에 연결
        int client_fd = socket(AF_INET, SOCK_STREAM, 0);
        assert(client_fd != -1);
        struct sockaddr_in servAddr;
        memset(&servAddr, 0, sizeof(servAddr));
        servAddr.sin_family = AF_INET;
        servAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        servAddr.sin_port = htons(port);
        int ret = connect(client_fd, (struct sockaddr*)&servAddr, sizeof(servAddr));
        assert(ret != -1);

        // 서버가 client_fd를 등록하고 읽기 이벤트를 대기할 시간을 주기 위해 대기
        sleep(2);

        // ─ 클라이언트 -> 서버 전송 테스트 ─
        // 클라이언트가 서버로 메시지를 전송하여, 서버측 client_fd에서 READ_EVENT가 발생하는지 확인합니다.
        const char* reply = "Hello from client";
        ret = write(client_fd, reply, strlen(reply));
        assert(ret == (int)strlen(reply));

        close(client_fd);
        _exit(0);  // 자식 프로세스 종료
    } else {
        // ── 부모 프로세스 (Server) ──────────────────────────────
        // 서버는 리스닝 소켓에 대한 이벤트를 먼저 대기 (클라이언트의 접속 요청)
        int numEvents = demux.waitForEvent();
        bool listenReadEventFound = false;
        for (int i = 0; i < numEvents; ++i) {
            int fd = demux.getSocketFd(i);
            EnumEvent eventType = demux.getEventType(i);
            if (fd == listen_fd && eventType == READ_EVENT) {
                listenReadEventFound = true;
                break;
            }
        }
        assert(listenReadEventFound);
        std::cout << " - ✅[TEST] Read Event on ListeningFd passed.\n";

        // 클라이언트의 연결 요청을 accept()를 통해 수락
        int client_fd = accept(listen_fd, NULL, NULL);
        assert(client_fd != -1);
        std::cout << " - [SERVER] Accepted connection.\n";

        // ─ 서버측에서 클라이언트가 보낸 데이터에 대한 READ_EVENT 테스트 ─
        // client_fd에 대해 읽기/예외 이벤트를 등록 (이전에 등록되지 않았다면 추가)
        demux.addSocket(client_fd);

        // 클라이언트가 메시지를 전송할 시간을 주기 위해 대기
        sleep(2);

        int newEvents = demux.waitForEvent();
        bool serverReadEventFound = false;
        for (int i = 0; i < newEvents; ++i) {
            int fd = demux.getSocketFd(i);
            EnumEvent eventType = demux.getEventType(i);
            if (fd == client_fd && eventType == READ_EVENT) {
                serverReadEventFound = true;
                break;
            }
        }
        assert(serverReadEventFound);
        std::cout << " - ✅[TEST] Read Event on ClientFd (server side) passed.\n";

        // 서버는 클라이언트가 보낸 데이터를 읽어 확인함
        char buf[128] = {0};
        int n = read(client_fd, buf, sizeof(buf));
        assert(n > 0);
        std::cout << " - [SERVER] Received: " << buf << "\n";

        // 소켓 정리
        close(client_fd);
        close(listen_fd);
        int status;
        wait(&status); // 자식 프로세스 종료 대기
        std::cout << "✅[TEST] Client -> Server Read Event passed.\n" << std::endl;
    }
}


/*
 * 테스트 2: Write 이벤트 테스트
 * - 클라이언트와 연결된 소켓에 대해 addWriteEvent() 호출 후, 쓰기 가능 이벤트(WRITE_EVENT)가 즉시 발생하는지 확인
 * - 이후 removeWriteEvent() 호출 후, 일정 시간 동안 WRITE_EVENT가 발생하지 않는지 타임아웃으로 검증
 */
void test_write_event() {
    std::cout << "📌[TEST]Running Write Event Test\n";

    // 리스닝 소켓 생성 및 포트 번호 획득
    int listen_fd = create_listening_socket();
    int port = get_listening_port(listen_fd);
    
    // fork()를 통해 자식 프로세스에서 클라이언트 역할 수행
    pid_t pid = fork();
    if (pid == 0) {
        // 자식 프로세스: 클라이언트로 접속 후 연결 유지
        sleep(1); // 서버 준비 대기
        int client_fd = socket(AF_INET, SOCK_STREAM, 0);
        assert(client_fd != -1);
        struct sockaddr_in servAddr;
        memset(&servAddr, 0, sizeof(servAddr));
        servAddr.sin_family = AF_INET;
        servAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        servAddr.sin_port = htons(port);
        int ret = connect(client_fd, (struct sockaddr*)&servAddr, sizeof(servAddr));
        assert(ret != -1);
        // 연결 유지: 쓰기 가능한 상태를 유지하기 위해 잠시 대기
        sleep(2);
        close(client_fd);
        _exit(0);
    } else {
        // 부모 프로세스: accept()를 통해 연결 수락
        int client_fd = accept(listen_fd, NULL, NULL);
        assert(client_fd != -1);
        std::set<int> emptySet;
        // KqueueDemultiplexer 생성 시, 리스닝 소켓은 등록하지 않고, 이후에 쓰기 이벤트만 추가
        KqueueDemultiplexer demux(emptySet);

        // 쓰기 이벤트 등록
        demux.addWriteEvent(client_fd);

        // 쓰기 가능 상태이므로 waitForEvent() 호출 시 WRITE_EVENT가 바로 발생해야 함
        int numEvents = demux.waitForEvent();
        bool found = false;
        for (int i = 0; i < numEvents; ++i) {
            int fd = demux.getSocketFd(i);
            EnumEvent eventType = demux.getEventType(i);
            if (fd == client_fd && eventType == WRITE_EVENT) {
                found = true;
                break;
            }
        }
        assert(found); // WRITE_EVENT가 확인되어야 함
        std::cout << " - ✅[TEST]addWriteEvent passed.\n" << std::endl;

        // 쓰기 이벤트 제거 후, 이벤트가 더 이상 발생하지 않는지 타임아웃 방식으로 확인
        demux.removeWriteEvent(client_fd);

        // SIGALRM을 사용한 타임아웃 설정
        signal(SIGALRM, alarm_handler);
        timeoutOccurred = 0;
        alarm(2); // 2초 후 타임아웃
        int ret = demux.waitForEvent();

        /* 
         * 타임아웃 혹은 이벤트 미발생 시 ret가 0이거나, SIGALRM에 의해 timeoutOccurred가 설정되어야 합니다.
         * kevent()의 동작은 시스템에 따라 다를 수 있으므로 ret==0 또는 타임아웃 발생 둘 다 허용합니다.
         */
        if (!(ret == 0 || timeoutOccurred)) {
            assert(false);
        }
        std::cout << " - ✅[TEST]removeWriteEvent passed.\n";
        alarm(0); // 알람 취소
        
        close(client_fd);
        close(listen_fd);
        int status;
        wait(&status); // 자식 프로세스 종료 대기
        std::cout << "✅[TEST]writeEvent passed.\n";
    }
}

/*
 * 테스트 3: Exception (OOB) 이벤트 테스트
 * - TCP 연결 후, 클라이언트가 OOB 데이터를 전송하면,
 *   서버 측에서 해당 소켓에 대해 EXCEPTION_EVENT(예외 이벤트)가 발생하는지 확인합니다.
 */
void test_exception_event() {
    std::cout << "📌[TEST]Running Exception Event Test\n";

    // 리스닝 소켓 생성 및 포트 번호 획득
    int listen_fd = create_listening_socket();
    int port = get_listening_port(listen_fd);
    
    // fork()를 사용하여 자식 프로세스에서 클라이언트 역할 수행
    pid_t pid = fork();
    if (pid == 0) {
        // 자식 프로세스: 클라이언트로 접속 후, OOB 데이터를 전송
        sleep(1); // 서버 준비 대기
        int client_fd = socket(AF_INET, SOCK_STREAM, 0);
        assert(client_fd != -1);
        struct sockaddr_in servAddr;
        memset(&servAddr, 0, sizeof(servAddr));
        servAddr.sin_family = AF_INET;
        servAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        servAddr.sin_port = htons(port);
        int ret = connect(client_fd, (struct sockaddr*)&servAddr, sizeof(servAddr));
        assert(ret != -1);
        sleep(1); // 서버가 소켓을 등록할 시간을 주기 위함
        char oob = 'X'; // 전송할 OOB 데이터
        ret = send(client_fd, &oob, 1, MSG_OOB);
        // OOB 전송 실패 시에도 시스템에 따라 다르게 처리될 수 있으므로 assert 없이 진행
        close(client_fd);
        _exit(0);
    } else {
        // 부모 프로세스: 클라이언트 연결 수락
        int client_fd = accept(listen_fd, NULL, NULL);
        assert(client_fd != -1);
        std::set<int> emptySet;
        // OOB 이벤트 검출을 위해 소켓을 등록 (addSocket()는 읽기/예외 이벤트를 등록)
        KqueueDemultiplexer demux(emptySet);
        demux.addSocket(client_fd);

        // waitForEvent() 호출 시 EXCEPTION_EVENT(예외 이벤트)가 발생해야 함
        int numEvents = demux.waitForEvent();
        bool found = false;
        for (int i = 0; i < numEvents; ++i) {
            int fd = demux.getSocketFd(i);
            EnumEvent eventType = demux.getEventType(i);
            if (fd == client_fd && eventType == EXCEPTION_EVENT) {
                found = true;
                break;
            }
        }
        assert(found); // 예외 이벤트가 감지되지 않으면 테스트 실패

        close(client_fd);
        close(listen_fd);
        int status;
        wait(&status); // 자식 프로세스 종료 대기
        std::cout << "✅[TEST]Exception Event passed.\n" << std::endl;
    }
}

/*
 * main 함수: 모든 테스트 케이스 실행
 */
int main() {
    test_client_to_server_read_event();
    test_write_event();
    test_exception_event();
    std::cout << "All tests passed.\n";
    return 0;
}
