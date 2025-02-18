ifndef TOPDIR
	TOPDIR = $(abspath ../)
endif
ifndef SRCDIR
	SRCDIR = $(abspath ./)
endif

NAME := webserv
SRCS := $(wildcard *.cpp)

# SRCS(in SRCDIR) ------
DIRS := utils \
		GlobalConfig \
		ConfigParser \
		RequestMessage \
		RequestParser \
		ServerManager \
		Demultiplexer \
#		TimeoutHandler \# RequestParser 테스트를 위해 임시로 주석처리


SUBS := $(addsuffix .a,$(DIRS))

include $(TOPDIR)/include_make/Variable.mk
include $(TOPDIR)/include_make/Link.mk
include $(TOPDIR)/include_make/Recipe_src.mk