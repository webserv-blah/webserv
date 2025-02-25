ifndef TOPDIR
	TOPDIR = $(abspath ../../)
endif
ifndef SRCDIR
	SRCDIR = $(abspath ../)
endif

NAME := ConfigParser.a
HEAD := ConfigParser.hpp
SRCS := $(wildcard *.cpp)

include $(TOPDIR)/include_make/Verbose.mk
include $(TOPDIR)/include_make/Color.mk
include $(TOPDIR)/include_make/Variable.mk
CPPFLAGS += -I$(SRCDIR)/GlobalConfig
include $(TOPDIR)/include_make/Link.mk
include $(TOPDIR)/include_make/Recipe_subsrc.mk