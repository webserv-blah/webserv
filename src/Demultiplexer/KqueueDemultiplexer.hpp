#ifdef __APPLE__

#pragma once
#include "DemultiplexerBase.hpp"
#include <iostream>
#include <vector>
#include <set>
#include <sys/event.h>
#include <unistd.h>

class KqueueDemultiplexer : public DemultiplexerBase<KqueueDemultiplexer> {
	public:
		KqueueDemultiplexer(std::set<int>& serverFds);
		~KqueueDemultiplexer();
		int			waitForEventImpl();
		void		addSocketImpl(int fd);
		void		removeSocketImpl(int fd);
		void		addWriteEventImpl(int fd);
		void		removeWriteEventImpl(int fd);
		int			getSocketFdImpl(int idx);
		TypeEvent	getEventTypeImpl(int idx);
	
	
	private:
		int							kq_;
		std::vector<struct kevent>	eventList_;
		std::vector<struct kevent>	changedEvents_;
	
};

typedef KqueueDemultiplexer Demultiplexer;
#endif