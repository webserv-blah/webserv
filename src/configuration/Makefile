ifndef TOPDIR
	TOPDIR = $(abspath ../../)
endif
ifndef SRCDIR
	SRCDIR = $(abspath ../)
endif

NAME := configuration.a
HEAD := configuration.hpp \
		GlobalConfig.hpp \
		ConfigParser.hpp \
		Optional.hpp \
		utilities.hpp
SRCS := $(wildcard *.cpp)

include $(TOPDIR)/include_make/Variable.mk
include $(TOPDIR)/include_make/Link.mk
include $(TOPDIR)/include_make/Recipe_subsrc.mk