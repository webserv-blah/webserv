ifndef TOPDIR
	TOPDIR = $(abspath .)
endif

all clean fclean re:
	$(Q)$(MAKE) TOPDIR=`pwd` -C src $@

.PHONY: all clean fclean re

include $(TOPDIR)/include_make/Verbose.mk
