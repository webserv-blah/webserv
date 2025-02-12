.DELETE_ON_ERROR:
.DEFAULT_GOAL = all

CFLAGS = -Wall -Wextra -Werror -std=c++98
ARFLAGS = rcs
CPPFLAGS = -I$(TOPDIR)/include -I$(SRCDIR)/include

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