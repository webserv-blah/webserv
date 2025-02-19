ifndef TOPDIR
	TOPDIR = $(abspath ../../)
endif
ifndef SRCDIR
	SRCDIR = $(abspath ../)
endif

NAME := GlobalConfig.a
HEAD := GlobalConfig.hpp
SRCS := $(wildcard *.cpp)

include $(TOPDIR)/include_make/Variable.mk
CPPFLAGS += -I$(SRCDIR)/ConfigParser
include $(TOPDIR)/include_make/Link.mk
include $(TOPDIR)/include_make/Recipe_subsrc.mk