ifndef TOPDIR
	TOPDIR = $(abspath .)
endif

all clean fclean re:
	$(Q)$(MAKE) TOPDIR=`pwd` -C src $@

SHELL = /bin/bash
REQ_TEST_FILE=_request_message.sh
reqtest: $(REQ_TEST_FILE)
	source $(REQ_TEST_FILE); ./webserv config.conf

.PHONY: all clean fclean re reqtest

include $(TOPDIR)/include_make/Verbose.mk