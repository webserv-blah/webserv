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
		# RequestMessage \
		# RequestParser \
		# ServerManager \
		# Demultiplexer \
		# TimeoutHandler


SUBS := $(addsuffix .a,$(DIRS))

include $(TOPDIR)/include_make/Variable.mk
include $(TOPDIR)/include_make/Link.mk
include $(TOPDIR)/include_make/Recipe_src.mk