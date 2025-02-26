ifndef TOPDIR
	TOPDIR = $(abspath .)
endif

all clean fclean re:
	$(Q)$(MAKE) TOPDIR=`pwd` DEBUG=$(DEBUG) ADDRESS=$(ADDRESS) -C src $@

.PHONY: all clean fclean re

include $(TOPDIR)/include_make/Verbose.mk
