#include "EventHandler.hpp"
#include "../ClientSession/ClientSession.hpp"
#include <errno.h>
#include <algorithm>
#include <sys/socket.h>
#include "errorUtils.hpp"

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 8192
#endif

EnumSesStatus EventHandler::sendResponse(ClientSession& session) {
    std::string writeBuffer = session.getWriteBuffer();
    int clientFd = session.getClientFd();

	// DEBUG_LOG("[EventHandler]Will Be Sent MSG:\n" << writeBuffer)

    // 보낼 내용이 없으면 바로 완료 상태로 설정
    if (writeBuffer.empty()) {
        return WRITE_COMPLETE;
    }

    // 전송할 바이트 수는 BUFFER_SIZE와 남은 데이터 크기 중 작은 값
    size_t bytesToSend = std::min(writeBuffer.size(), static_cast<size_t>(BUFFER_SIZE));
    ssize_t sent = send(clientFd, writeBuffer.data(), bytesToSend, 0);

    if (sent < 0) {
        // 재시도 가능한 오류인 경우 (EINTR, EAGAIN, EWOULDBLOCK)는 WRITE_CONTINUE 상태로 처리
        if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) {
            return WRITE_CONTINUE;
        }
        // 그 외의 오류는 치명적 오류로 간주하여 CONNECTION_CLOSED 상태로 처리
        return CONNECTION_CLOSED;
    }

    // 전송된 만큼 버퍼에서 제거
    writeBuffer.erase(0, sent);
    EnumSesStatus status = writeBuffer.empty() ? WRITE_COMPLETE : WRITE_CONTINUE;
    
    // 업데이트된 버퍼와 상태를 세션에 반영
    session.setWriteBuffer(writeBuffer);
    return status;
}
