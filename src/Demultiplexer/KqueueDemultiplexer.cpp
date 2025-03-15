#include "KqueueDemultiplexer.hpp"
#include <iostream>
#include <sys/event.h>
#include <unistd.h>
#include "../include/errorUtils.hpp"

// 생성자: kqueue를 생성하고, 리스닝 소켓들을 이벤트 감지 목록에 추가
KqueueDemultiplexer::KqueueDemultiplexer(std::set<int>& listenFds) : eventList_(MAX_EVENT) {
	kq_ = kqueue(); // kqueue 인스턴스 생성
	if (kq_ == -1) {
		webserv::throwSystemError("kqueue", "Creating event queue", "KqueueDemultiplexer::KqueueDemultiplexer"); // kqueue 생성 실패 시 예외 발생
	}

	// 리스닝 소켓을 kqueue에 등록
	std::set<int>::const_iterator it;
	for (it = listenFds.begin(); it != listenFds.end(); ++it) {
		addFdImpl(*it);
	}
}

// 소멸자: kqueue 파일 디스크립터 닫기
KqueueDemultiplexer::~KqueueDemultiplexer() {
	close(kq_);
}

// 이벤트를 대기하는 함수 (kevent 호출)
int KqueueDemultiplexer::waitForEventImpl(timespec* timeout) {
	// 변경된 이벤트가 없으면(numChanges == 0) NULL 전달
	int numChanges = changedEvents_.size();
	const struct kevent* changedEvents = numChanges ? &changedEvents_[0] : NULL;

	// kevent 호출: 이벤트 감지
	// 반환값 : 감지된 이벤트 개수
	numEvents_ = kevent(kq_, changedEvents, numChanges, &eventList_[0], MAX_EVENT, timeout);
	if (numEvents_ == -1) {
		webserv::throwSystemError("kevent", "Waiting for events", "KqueueDemultiplexer::waitForEventImpl"); // kevent 호출 실패
	}
	changedEvents_.clear(); // 변경된 이벤트 목록 초기화

	return numEvents_;
}

// 소켓을 kqueue에 등록 (읽기 이벤트 및 예외 이벤트 추가)
void KqueueDemultiplexer::addFdImpl(int fd) {
	struct kevent change;

	EV_SET(&change, fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
	changedEvents_.push_back(change);
}

// 소켓을 kqueue에서 제거 (읽기, 예외, 쓰기 이벤트 삭제)
void KqueueDemultiplexer::removeFdImpl(int fd) {
	struct kevent changes[2];

	EV_SET(&changes[0], fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);   // 읽기 이벤트 제거
	EV_SET(&changes[1], fd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);  // 쓰기 이벤트 제거
	changedEvents_.insert(changedEvents_.end(), changes, changes + 2);
}

// 읽기 이벤트를 추가 (cgi 처리가 완료되었을 시)
void KqueueDemultiplexer::addReadEventImpl(int fd) {
	struct kevent change;

	EV_SET(&change, fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
	changedEvents_.push_back(change);
}

// 읽기 이벤트를 제거(cgi 처리가 진행 중일 경우)
void KqueueDemultiplexer::removeReadEventImpl(int fd) {
	struct kevent change;
	
	EV_SET(&change, fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
	changedEvents_.push_back(change);
}

// 쓰기 이벤트를 추가 (fd가 쓰기 가능할 때 감지)
void KqueueDemultiplexer::addWriteEventImpl(int fd) {
	struct kevent change;

	EV_SET(&change, fd, EVFILT_WRITE, EV_ADD, 0, 0, NULL);
	changedEvents_.push_back(change);
}

// 쓰기 이벤트를 제거
void KqueueDemultiplexer::removeWriteEventImpl(int fd) {
	struct kevent change;
	
	EV_SET(&change, fd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
	changedEvents_.push_back(change);
}

// 특정 이벤트의 타입을 반환
EnumEvent KqueueDemultiplexer::getEventTypeImpl(int idx) {
	if (idx < numEvents_) { // 유효한 인덱스인지 확인
		if (eventList_[idx].filter == EVFILT_READ) {
			return READ_EVENT; // 읽기 이벤트
		}
		if (eventList_[idx].filter == EVFILT_WRITE) {
			return WRITE_EVENT; // 쓰기 이벤트
		}
	}
	return UNKNOWN_EVENT; // 알 수 없는 이벤트
}

// 이벤트 목록에서 해당 인덱스의 파일 디스크립터 반환
int KqueueDemultiplexer::getSocketFdImpl(int idx) {
	if (idx < numEvents_) { // 유효한 인덱스인지 확인
		return eventList_[idx].ident; // 해당 이벤트의 fd 반환
	}
	return -1; // 유효하지 않은 경우 -1 반환
}