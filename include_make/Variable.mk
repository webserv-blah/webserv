.DELETE_ON_ERROR:
.DEFAULT_GOAL = all

# OS 판별
OS := $(shell uname -s)

# Linux일 경우에만 -lkqueue 추가
ifeq ($(OS), Linux)
    LDLIBS += -lkqueue
endif

CXXFLAGS = -Wall -Wextra -Werror -std=c++98
ARFLAGS = rcs
CPPFLAGS = $(shell find $(SRCDIR) -type d | sort -u | sed 's|^|-I|') -I/usr/include/kqueue/
# src경로의 모든 디렉토리를 include path로 추가

ARCHIVES = ServerManager.a TimeoutHandler.a EventHandler.a Demultiplexer.a \
		   RequestHandler.a ResponseBuilder.a RequestParser.a ClientManager.a \
		   ClientSession.a RequestMessage.a utils.a GlobalConfig.a ConfigParser.a 

OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.d)
-include $(DEPS)

ifdef DEPS
	CPPFLAGS += -MMD -MP
endif
ifdef DEBUG
	CPPFLAGS += -g3
endif
ifdef ADDRESS
	CPPFLAGS += -fsanitize=address
	LDFLAGS += -fsanitize=address
endif