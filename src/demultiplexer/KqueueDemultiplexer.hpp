#ifdef __APPLE__

#pragma once
#include "DemultiplexerBase.hpp"
#include <sys/event.h>
#include <iostream>
#include <vector>
#include <unistd.h>

class KqueueDemultiplexer : public DemultiplexerBase<KqueueDemultiplexer> {
	public:
		KqueueDemultiplexer();
		~KqueueDemultiplexer();
		int		waitForEventImpl();
		void	addSocketImpl(int fd);
		void	removeSocketImpl(int fd);
		void	addWriteEventImpl(int fd);
		void	removeWriteEventImpl(int fd);
		
		int		getEventTypeImpl(int idx);
		int		getSocketFdImpl(int idx);
	
	
	private:
		int							kq_;
		std::vector<struct kevent>	eventList_;
		std::vector<struct kevent>	changedEvents_;
	
};

typedef KqueueDemultiplexer Demultiplexer;
#endif