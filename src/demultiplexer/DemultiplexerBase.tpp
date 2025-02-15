#include "DemultiplexerBase.hpp"

template <typename Derived>
DemultiplexerBase<Derived>::DemultiplexerBase() {
	
}

template <typename Derived>
DemultiplexerBase<Derived>::~DemultiplexerBase() {
	
}

template <typename Derived>
int waitForEvent() {
	return static_cast<Derived*>(this)->waitForEventImpl();
}

template <typename Derived>
void addSocket() {
	return static_cast<Derived*>(this)->addSocketImpl();
}

template <typename Derived>
void removeSocket() {
	return static_cast<Derived*>(this)->removeSocketImpl();
}

template <typename Derived>
void addWriteEvent() {
	return static_cast<Derived*>(this)->addWriteEventImpl();
}

template <typename Derived>
void removeWriteEvent() {
	return static_cast<Derived*>(this)->removeWriteEventImpl();
}

template <typename Derived>
bool isReadEvent() {
	return static_cast<Derived*>(this)->isReadEventImpl();
}

template <typename Derived>
bool isWriteEvent() {
	return static_cast<Derived*>(this)->isWriteEventImpl();
}

template <typename Derived>
bool isExceptionEvent() {
	return static_cast<Derived*>(this)->isExceptionEventImpl();
}


