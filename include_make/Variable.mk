.DELETE_ON_ERROR:
.DEFAULT_GOAL = all

CXXFLAGS = -Wall -Wextra -Werror -std=c++98
ARFLAGS = rcs
CPPFLAGS = $(shell find $(SRCDIR) -type d | sort -u | sed 's|^|-I|')
# src경로의 모든 디렉토리를 include path로 추가

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