ifndef TOPDIR
	TOPDIR = $(abspath ../)
endif
ifndef SRCDIR
	SRCDIR = $(abspath ./)
endif

NAME := program
SRCS := $(wildcard *.cpp)

# SRCS(in SRCDIR) ------
DIRS := $(subst /,,$(filter-out include/, $(wildcard */)))
SUBS := $(addsuffix .a,$(DIRS))

include $(TOPDIR)/include_make/Variable.mk
include $(TOPDIR)/include_make/Link.mk
include $(TOPDIR)/include_make/Recipe_src.mk
