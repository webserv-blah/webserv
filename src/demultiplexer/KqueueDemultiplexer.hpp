#ifdef __APPLE__

#pragma once
#include "DemultiplexerBase.hpp"
#include <sys/event.h>
#include <iostream>

class KqueueDemultiplexer : public DemultiplexerBase<KqueueDemultiplexer> {
	public:
		KqueueDemultiplexer();
		~KqueueDemultiplexer();
		int		waitForEventImpl();
		void	addSocketImpl(int fd);
		void	removeSocketImpl();
		void	addWriteEventImpl();
		void	removeWriteEventImpl();
		
		bool	isReadEventImpl(int idx);
		bool	isWriteEventImpl(int idx);
		bool	isExceptionEventImpl(int idx);
	
	private:
		int				_Demultiplexer_fd;
		struct kevent	_events[MAX_EVENT];
		struct kevent	_ev;
		
	
};

typedef KqueueDemultiplexer Demultiplexer;

#endif