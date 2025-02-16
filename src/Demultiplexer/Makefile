ifndef TOPDIR
	TOPDIR = $(abspath ../../)
endif
ifndef SRCDIR
	SRCDIR = $(abspath ../)
endif

NAME := Demuliplexer.a
HEAD := DemuliplexerBase.hpp DemuliplexerBase.tpp KqueueDemultiplexer.hpp
SRCS := $(wildcard *.cpp)

include $(TOPDIR)/include_make/Variable.mk
include $(TOPDIR)/include_make/Link.mk
include $(TOPDIR)/include_make/Recipe_subsrc.mk