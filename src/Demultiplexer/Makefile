ifndef TOPDIR
	TOPDIR = $(abspath ../../)
endif
ifndef SRCDIR
	SRCDIR = $(abspath ../)
endif

NAME := Demultiplexer.a
HEAD := DemultiplexerBase.hpp \
		DemultiplexerBase.tpp \
		KqueueDemultiplexer.hpp \

SRCS := $(wildcard *.cpp)

include $(TOPDIR)/include_make/Verbose.mk
include $(TOPDIR)/include_make/Color.mk
include $(TOPDIR)/include_make/Variable.mk
include $(TOPDIR)/include_make/Link.mk
include $(TOPDIR)/include_make/Recipe_subsrc.mk