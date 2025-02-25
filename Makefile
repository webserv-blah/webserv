ifndef TOPDIR
	TOPDIR = $(abspath .)
endif

all clean fclean re:
	$(MAKE) TOPDIR=`pwd` -C src $@

.PHONY: all clean fclean re
