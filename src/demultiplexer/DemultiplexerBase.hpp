#pragma once
#define MAX_EVENT 100

template<typename Derived>
class DemultiplexerBase {
	public:
		int waitForEvent() {
			return static_cast<Derived*>(this)->waitForEventImpl();
		}
		void addSocket() {
			return static_cast<Derived*>(this)->addSocketImpl();
		}
		void removeSocket() {
			return static_cast<Derived*>(this)->removeSocketImpl();
		}
		void addWriteEvent() {
			return static_cast<Derived*>(this)->addWriteEventImpl();
		}
		void removeWriteEvent() {
			return static_cast<Derived*>(this)->removeWriteEventImpl();
		}
		bool isReadEvent() {
			return static_cast<Derived*>(this)->isReadEventImpl();
		}
		bool isWriteEvent() {
			return static_cast<Derived*>(this)->isWriteEventImpl();
		}
		bool isExceptionEvent() {
			return static_cast<Derived*>(this)->isExceptionEventImpl();
		}
	
	protected:
		DemultiplexerBase() = default;
		~DemultiplexerBase() = default;
	
};

# include "DemultiplexerBase.tpp"
