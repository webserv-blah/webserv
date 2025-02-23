// test_ClientManager.cpp
#include <cassert>
#include <iostream>

// dummy_close.hpp를 포함하여 실제 close() 호출을 방지
#include "dummy_close.hpp"
#include "ClientManager.hpp"

void testAddAndAccessClient() {
    std::cout << "[Test] addClient 및 accessClientSession 테스트 시작" << std::endl;
    ClientManager manager;
    // 초기에는 없는 fd에 대해 NULL 반환
    assert(manager.accessClientSession(100) == NULL);
    assert(manager.accessClientSessionMap().size() == 0);

    // 클라이언트 추가
    manager.addClient(1, 100);
    ClientSession* cs = manager.accessClientSession(100);
    assert(cs != NULL);
    // dummy ClientSession의 값이 올바른지 확인
    assert(cs->getListenFd() == 1);
    assert(cs->getClientFd() == 100);
    // 존재하지 않는 fd 접근 시 NULL이어야 함
    assert(manager.accessClientSession(200) == NULL);
    // 내부 map 크기 확인
    assert(manager.accessClientSessionMap().size() == 1);

    std::cout << "[Test] addClient 및 accessClientSession 테스트 성공" << std::endl;
}

void testRemoveClient() {
    std::cout << "[Test] removeClient 테스트 시작" << std::endl;
    ClientManager manager;
    // 두 클라이언트 추가
    manager.addClient(1, 101);
    manager.addClient(1, 102);
    // fd 101 클라이언트 제거
    ClientManager::TypeClientMap::iterator it = manager.removeClient(101);
    // 제거 후 해당 fd로의 접근은 NULL이어야 함
    assert(manager.accessClientSession(101) == NULL);
    // 나머지 클라이언트는 정상 접근 가능
    assert(manager.accessClientSession(102) != NULL);
    // 존재하지 않는 fd 제거 시 end iterator 반환
    it = manager.removeClient(999);
    assert(it == manager.accessClientSessionMap().end());
    // 내부 map의 크기는 1이어야 함
    assert(manager.accessClientSessionMap().size() == 1);

    std::cout << "[Test] removeClient 테스트 성공" << std::endl;
}

void testDestructorCleanup() {
    std::cout << "[Test] 소멸자 자원 정리 테스트 시작" << std::endl;
    {
        ClientManager manager;
        manager.addClient(1, 201);
        manager.addClient(1, 202);
        // 클라이언트 생성 후 카운터는 2여야 함
        assert(ClientSession::counter == 2);
    }
    // manager 소멸 후 모든 ClientSession이 해제되어 카운터는 0이어야 함
    assert(ClientSession::counter == 0);
    std::cout << "[Test] 소멸자 자원 정리 테스트 성공" << std::endl;
}

int main() {
    std::cout << "=== ClientManager 테스트 시작 ===" << std::endl;
    testAddAndAccessClient();
    testRemoveClient();
    testDestructorCleanup();
    std::cout << "=== 모든 테스트 성공 ===" << std::endl;
    return 0;
}
