all: $(NAME)

$(NAME): $(OBJS)
	$(AR) $(ARFLAGS) $@ $^
	$(MAKE) files=$(NAME) src_dir=`pwd` dst_dir=$(SRCDIR) link_files
	$(foreach head,$(HEAD), $(MAKE) files=$(head) src_dir=`pwd` dst_dir=$(SRCDIR)/include link_files;)

clean:
	$(MAKE) files=$(NAME) src_dir=`pwd` dst_dir=$(SRCDIR) unlink_files
	$(foreach head,$(HEAD), $(MAKE) files=$(head) src_dir=`pwd` dst_dir=$(SRCDIR)/include unlink_files;)
	$(RM) $(OBJS) $(DEPS)
	$(RM) $(NAME)

fclean: clean

re:
	$(MAKE) fclean
	$(MAKE) all

.PHONY: all clean fclean re