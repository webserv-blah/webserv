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

    if (writeBuffer.empty()) {
        return WRITE_COMPLETE;
    }

    size_t bytesToSend = std::min(writeBuffer.size(), static_cast<size_t>(BUFFER_SIZE));
    ssize_t sent = send(clientFd, writeBuffer.data(), bytesToSend, 0);

    if (sent > 0) {
        writeBuffer.erase(0, sent);
        session.setWriteBuffer(writeBuffer);
        return writeBuffer.empty() ? WRITE_COMPLETE : WRITE_CONTINUE;
    }

    if (sent == 0) {
        // 클라이언트가 연결을 닫았을 가능성이 크므로 CONNECTION_CLOSED 반환
        DEBUG_LOG("[EventHandler]Client closed connection: " << clientFd)
        return CONNECTION_CLOSED;
    }

    // send가 -1을 반환한 경우, 오류 발생 (논블로킹이면 다시 시도 가능)
    return WRITE_CONTINUE;
}
