all: 
	$(call color_printf,$(YELLOW),$(NAME),🎯 starting compile $(NAME))
	$(MAKE) $(NAME)
	$(call color_printf,$(BLUE),$(NAME),🔰 done!)

$(NAME): $(OBJS)
	$(call color_printf,$(GREEN),$(NAME),📚 archive object)
	$(AR) $(ARFLAGS) $@ $^
	$(MAKE) files=$(NAME) src_dir=`pwd` dst_dir=$(SRCDIR) link_files
	$(foreach head,$(HEAD), $(MAKE) files=$(head) src_dir=`pwd` dst_dir=$(SRCDIR)/include link_files;)

clean:
	$(MAKE) files=$(NAME) src_dir=`pwd` dst_dir=$(SRCDIR) unlink_files
	$(foreach head,$(HEAD), $(MAKE) files=$(head) src_dir=`pwd` dst_dir=$(SRCDIR)/include unlink_files;)
	$(call color_printf,$(RED),$(NAME),🗑️  remove Objects && Dependency file)
	$(RM) $(OBJS) $(DEPS)
	$(RM) $(NAME)

fclean: clean

re:
	$(MAKE) fclean
	$(MAKE) all

.PHONY: all clean fclean re