#ifndef KQUEUE_DEMULTIPLEXER_HPP
#define KQUEUE_DEMULTIPLEXER_HPP

#ifdef __APPLE__

#include "DemultiplexerBase.hpp"
#include <vector>
#include <set>

class KqueueDemultiplexer : public DemultiplexerBase<KqueueDemultiplexer> {
	public:
		KqueueDemultiplexer(std::set<int>& listenFds);
		~KqueueDemultiplexer();
		int			waitForEventImpl();
		void		addSocketImpl(int fd);
		void		removeSocketImpl(int fd);
		void		addWriteEventImpl(int fd);
		void		removeWriteEventImpl(int fd);
		int			getSocketFdImpl(int idx);
		EnumEvent	getEventTypeImpl(int idx);
	
	
	private:
		int							kq_;
		int							numEvents_;
		std::vector<struct kevent>	eventList_;
		std::vector<struct kevent>	changedEvents_;
		std::set<int>				writeEvents_;
		
		bool 						isWriteEventRegistered(int fd) const;
	
};

typedef KqueueDemultiplexer Demultiplexer;

#endif
#endif