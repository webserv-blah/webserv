#include "KqueueDemultiplexer.hpp"

KqueueDemultiplexer::KqueueDemultiplexer(std::set<int>& listenFds) : eventList_(MAX_EVENT){
	kq_ = kqueue();
	if (kq_ == -1) {
		perror("kqueue creation failed");
	}

	std::set<int>::const_iterator it;
	for (it = listenFds.begin(); it != listenFds.end(); ++it) {
		addSocketImpl(*it);
	}
}

KqueueDemultiplexer::~KqueueDemultiplexer() {
	close(kq_);
}

int		KqueueDemultiplexer::waitForEventImpl() {
	int numEvents = kevent(kq_, changedEvents_.data(), changedEvents_.size(), eventList_.data(), MAX_EVENT, nullptr);
	changedEvents_.clear(); 
	return numEvents;
}

void	KqueueDemultiplexer::addSocketImpl(int fd) {
	struct kevent changes[2];

	EV_SET(&changes[0], fd, EVFILT_READ, EV_ADD, 0, 0, nullptr);
	EV_SET(&changes[1], fd, EVFILT_EXCEPT, EV_ADD, 0, 0, nullptr);
	changedEvents_.insert(changedEvents_.end(), changes, changes + 2);
}

void	KqueueDemultiplexer::removeSocketImpl(int fd) {
	struct kevent changes[3];

	EV_SET(&changes[0], fd, EVFILT_READ, EV_DELETE, 0, 0, nullptr);
	EV_SET(&changes[1], fd, EVFILT_EXCEPT, EV_DELETE, 0, 0, nullptr);
	EV_SET(&changes[2], fd, EVFILT_WRITE, EV_DELETE, 0, 0, nullptr);
	changedEvents_.insert(changedEvents_.end(), changes, changes + 3);
}

void	KqueueDemultiplexer::addWriteEventImpl(int fd) {
	struct kevent change;

	EV_SET(&change, fd, EVFILT_WRITE, EV_ADD, 0, 0, nullptr);
	changedEvents_.push_back(change);
}

void	KqueueDemultiplexer::removeWriteEventImpl(int fd) {
	struct kevent change;
	
	EV_SET(&change, fd, EVFILT_WRITE, EV_DELETE, 0, 0, nullptr);
	changedEvents_.push_back(change);
}

TypeEvent	KqueueDemultiplexer::getEventTypeImpl(int idx) {
	if (eventList_[idx].filter == EVFILT_EXCEPT) {
		return EXCEPTION_EVENT;
	}
	if (eventList_[idx].filter == EVFILT_READ) {
		return READ_EVENT;
	}
	if (eventList_[idx].filter == EVFILT_WRITE) {
		return WRITE_EVENT;
	}
	return UNKNOWN_EVENT;
}

int KqueueDemultiplexer::getSocketFdImpl(int idx) {
	return eventList_[idx].ident;
}