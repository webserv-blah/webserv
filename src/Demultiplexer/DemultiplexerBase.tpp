#include "DemultiplexerBase.hpp"
#include "sys/event.h"

template <typename Derived>
DemultiplexerBase<Derived>::DemultiplexerBase() {

}

template <typename Derived>
DemultiplexerBase<Derived>::~DemultiplexerBase() {

}

template <typename Derived>
int DemultiplexerBase<Derived>::waitForEvent(timespec* timeout) {
	return static_cast<Derived&>(*this).waitForEventImpl(timeout);
}

template <typename Derived>
void DemultiplexerBase<Derived>::addSocket(int fd) {
	static_cast<Derived&>(*this).addSocketImpl(fd);
}

template <typename Derived>
void DemultiplexerBase<Derived>::removeSocket(int fd) {
	static_cast<Derived&>(*this).removeSocketImpl(fd);
}

template <typename Derived>
void DemultiplexerBase<Derived>::addWriteEvent(int fd) {
	static_cast<Derived&>(*this).addWriteEventImpl(fd);
}

template <typename Derived>
void DemultiplexerBase<Derived>::removeWriteEvent(int fd) {
	static_cast<Derived&>(*this).removeWriteEventImpl(fd);
}

template <typename Derived>
EnumEvent	DemultiplexerBase<Derived>::getEventType(int idx) {
	return static_cast<Derived&>(*this).getEventTypeImpl(idx);
}

template <typename Derived>
int DemultiplexerBase<Derived>::getSocketFd(int idx) {
	return static_cast<Derived&>(*this).getSocketFdImpl(idx);
}
