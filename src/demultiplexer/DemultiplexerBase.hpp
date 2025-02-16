#pragma once

#define MAX_EVENT 1024
#define READ_EVENT 1
#define WRITE_EVENT 2
#define EXCEPTION_EVENT 3
#define UNKNOWN_EVENT -1

template <typename Derived>
class DemultiplexerBase {
	public:
		int		waitForEvent(); 
		void	addSocket(int fd);
		void	removeSocket(int fd);
		void	addWriteEvent(int fd);
		void	removeWriteEvent(int fd);
		int		getEventType(int idx);
		int		getSocketFd(int idx);
	
	protected:
		DemultiplexerBase();
		~DemultiplexerBase();
	
};

# include "DemultiplexerBase.tpp"
