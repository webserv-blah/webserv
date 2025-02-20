#include "EventHandler.hpp"
#include "ClientSession.hpp"
#define CHUNK_SIZE 8192

EnumSesStatus EventHandler::sendResponse(ClientSession& session) {
    std::string writeBuffer = session.getWriteBuffer();
    int clientFd = session.getClientFd();
    size_t bufferSize = writeBuffer.size();

    if (bufferSize == 0) {
        // 보낼 내용이 없으면 완료 상태로 설정
        session.setStatus(WRITE_COMPLETE);
        return WRITE_COMPLETE;
    }

    // 이번 호출에서 보낼 바이트 수는 CHUNK_SIZE와 버퍼 남은 크기 중 작은 값
    size_t bytesToSend = (bufferSize < CHUNK_SIZE) ? bufferSize : CHUNK_SIZE;
    ssize_t sent = send(clientFd, writeBuffer.c_str(), bytesToSend, 0);
    
    if (sent < 0) {
        session.setStatus(CONNECTION_ERROR);
        return CONNECTION_ERROR;
    }

    // 전송된 만큼 writeBuffer에서 제거하여 남은 데이터를 다시 저장
    std::string remaining = writeBuffer.substr(sent);
    session.setWriteBuffer(remaining);

    // 전송 후 남은 데이터가 있다면 WRITE_CONTINUE 상태로, 모두 전송되었다면 WRITE_COMPLETE 상태로 설정
    if (remaining.size() > 0) {
        session.setStatus(WRITE_CONTINUE); // 아직 전송할 데이터가 남음
        return WRITE_CONTINUE;
    } else {
        session.setStatus(WRITE_COMPLETE);
        return WRITE_COMPLETE;
    }
}