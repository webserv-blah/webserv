.DELETE_ON_ERROR:
.DEFAULT_GOAL = all

CXXFLAGS = -Wall -Wextra -Werror -std=c++98
ARFLAGS = rcs
CPPFLAGS = $(shell find $(SRCDIR) -type d | sort -u | sed 's|^|-I|')
# src경로의 모든 디렉토리를 include path로 추가

# 컨테이너 환경을 위한 플래그
## OS 판별
OS := $(shell uname -s)
## Linux일 경우에만 -lkqueue 추가
ifeq ($(OS), Linux)
    LDLIBS += -lkqueue
    CPPFLAGS += -I/usr/include/kqueue/
endif

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