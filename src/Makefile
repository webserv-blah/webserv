ifndef TOPDIR
	TOPDIR = $(abspath ../)
endif
ifndef SRCDIR
	SRCDIR = $(abspath ./)
endif

NAME := webserv
SRCS := $(wildcard *.cpp)

# SRCS(in SRCDIR) ------
# 두 가지 버전이 있는데, 개인적으로 전자가 더 빠르다고 생각합니다.
# 확인 후 의견 부탁드립니다!
# ver1 (기존 내용 기반)
DIRS := utils \
		GlobalConfig \
		ConfigParser \
		RequestMessage \
		ClientSession \
		ClientManager \
		RequestParser \
		ResponseBuilder \
		RequestHandler \
		Demultiplexer \
		EventHandler \
		TimeoutHandler \
		ServerManager

REVERSED_DIRS := $(foreach d, $(DIRS), $(eval REVERSED := $(d) $(REVERSED))$(REVERSED))
SUBS := $(addsuffix .a, $(REVERSED_DIRS))

# ver2 (최근 수정 사항 반영)
# DIRS := $(subst /,,$(filter-out include/, $(wildcard */)))
# SUBS := ServerManager.a TimeoutHandler.a EventHandler.a Demultiplexer.a \
# 		RequestHandler.a ResponseBuilder.a RequestParser.a ClientManager.a \
# 		ClientSession.a RequestMessage.a utils.a GlobalConfig.a ConfigParser.a 

include $(TOPDIR)/include_make/Verbose.mk
include $(TOPDIR)/include_make/Color.mk
include $(TOPDIR)/include_make/Variable.mk
include $(TOPDIR)/include_make/Link.mk
include $(TOPDIR)/include_make/Recipe_src.mk