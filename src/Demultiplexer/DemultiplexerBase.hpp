#ifndef DEMULTIPLEXER_BASE_HPP
#define DEMULTIPLEXER_BASE_HPP

#include "../include/commonEnums.hpp"
static const int MAX_EVENT = 1024;

template <typename Derived>
class DemultiplexerBase {
	public:
		int			waitForEvent(); 
		void		addSocket(int fd);
		void		removeSocket(int fd);
		void		addWriteEvent(int fd);
		void		removeWriteEvent(int fd);
		int			getSocketFd(int idx);
		EnumEvent	getEventType(int idx);
	
	protected:
		DemultiplexerBase();
		~DemultiplexerBase();
	
};

#include "DemultiplexerBase.tpp"

#endif
