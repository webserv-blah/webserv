all: 
	$(Q)$(MAKE) $(NAME) >/dev/null

$(NAME): $(OBJS)
	$(Q)$(AR) $(ARFLAGS) $@ $^
	$(Q)$(MAKE) files=$(NAME) src_dir=`pwd` dst_dir=$(SRCDIR) link_files >/dev/null
	$(Q)$(foreach head,$(HEAD), $(MAKE) files=$(head) src_dir=`pwd` dst_dir=$(SRCDIR)/include link_files >/dev/null;)

clean:
	$(Q)$(MAKE) files=$(NAME) src_dir=`pwd` dst_dir=$(SRCDIR) unlink_files >/dev/null
	$(Q)$(foreach head,$(HEAD), $(MAKE) files=$(head) src_dir=`pwd` dst_dir=$(SRCDIR)/include unlink_files >/dev/null;)
	$(Q)$(RM) $(OBJS) $(DEPS)
	$(Q)$(RM) $(NAME)

fclean: clean

re:
	$(Q)$(MAKE) fclean >/dev/null
	$(Q)$(MAKE) all >/dev/null

.PHONY: all clean fclean re