#ifndef KQUEUE_DEMULTIPLEXER_HPP
#define KQUEUE_DEMULTIPLEXER_HPP

// #ifdef __APPLE__

#include "DemultiplexerBase.hpp"
#include <vector>
#include <set>
#include <time.h>

class KqueueDemultiplexer : public DemultiplexerBase<KqueueDemultiplexer> {
	public:
		KqueueDemultiplexer(std::set<int>& listenFds);
		~KqueueDemultiplexer();
		int			waitForEventImpl(timespec* timeout);

		void		addReadEventImpl(int fd);
		void		removeReadEventImpl(int fd);
		void		addWriteEventImpl(int fd);
		void		removeWriteEventImpl(int fd);
		void		removeFdImpl(int fd);

		int			getSocketFdImpl(int idx);
		EnumEvent	getEventTypeImpl(int idx);
	
	
	private:
		int							kq_;
		int							numEvents_;
		std::vector<struct kevent>	eventList_;
		std::vector<struct kevent>	changedEvents_;
	
};

typedef KqueueDemultiplexer Demultiplexer;

// #endif
#endif