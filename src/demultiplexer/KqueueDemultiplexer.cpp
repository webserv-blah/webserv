#include "KqueueDemultiplexer.hpp"

KqueueDemultiplexer::KqueueDemultiplexer() {
	_Demultiplexer_fd = kqueue();
	if (_Demultiplexer_fd == -1) {
		//에러처리
		perror("kqueue");
	}
	//_events초기화가 필요한가. 
}

KqueueDemultiplexer::~KqueueDemultiplexer() {
	
}

int		KqueueDemultiplexer::waitForEventImpl() {
	int num_events = kevent(_Demultiplexer_fd, &_events[0], _events.len(), );
	return num_events;
}

void	KqueueDemultiplexer::addSocketImpl(int fd) {
	//_ev.. 를 사용해야 맞는건지, ev를 매번 생성하거나 해야하는건지..
	//events랑 changed_list 구분해서 관리해야하나?
	EV_SET(&_ev, fd, EVFILT_READ, EV_ADD, 0, 0, nullptr);
	EV_SET(&_ev, fd, EVFILT_EXCEPT, EV_ADD, 0, 0, nullptr);
}
void	KqueueDemultiplexer::removeSocketImpl() {
	
}
void	KqueueDemultiplexer::addWriteEventImpl() {
	
}
void	KqueueDemultiplexer::removeWriteEventImpl() {
	
}

bool	KqueueDemultiplexer::isReadEventImpl(int idx) {
	if (_events[idx].filter == EVFILT_READ) {
		return true;
	}
	return false;
}
bool	KqueueDemultiplexer::isWriteEventImpl(int idx) {
	if (_events[idx].filter == EVFILT_WRITE) {
		return true;
	}
	return false;
	
}
bool	KqueueDemultiplexer::isExceptionEventImpl(int idx) {
	if (_events[idx].filter == EVFILT_EXCEPT) {
		return true;
	}
	return false;
}