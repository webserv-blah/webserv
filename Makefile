ifndef TOPDIR
	TOPDIR = $(abspath .)
endif

all clean fclean re:
	$(MAKE) TOPDIR=`pwd` -C src $@

REQ_TEST_FILE=_request_msg_test.sh
reqtest: $(REQ_TEST_FILE)
	source $(REQ_TEST_FILE); ./webserv

.PHONY: all clean fclean re reqtest