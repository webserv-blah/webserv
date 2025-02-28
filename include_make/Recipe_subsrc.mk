all: 
	$(Q)$(call $(AQ_color_printf),$(YELLOW),$(NAME),🎯 starting compile $(NAME))
	$(Q)$(MAKE) $(NAME)
	$(Q)$(call $(AQ_color_printf),$(BLUE),$(NAME),🔰 done!)

%.o: %.cpp
	$(AQ)$(COMPILE.cpp) $(OUTPUT_OPTION) $<

$(NAME): $(OBJS)
	$(Q)$(call $(AQ_color_printf),$(GREEN),$(NAME),📚 archive object)
	$(Q)$(AR) $(ARFLAGS) $@ $^
	$(Q)$(MAKE) files=$(NAME) src_dir=`pwd` dst_dir=$(SRCDIR) link_files
	$(Q)$(foreach head,$(HEAD), $(MAKE) files=$(head) src_dir=`pwd` dst_dir=$(SRCDIR)/include link_files;)

clean:
	$(Q)$(MAKE) files=$(NAME) src_dir=`pwd` dst_dir=$(SRCDIR) unlink_files
	$(Q)$(foreach head,$(HEAD), $(MAKE) files=$(head) src_dir=`pwd` dst_dir=$(SRCDIR)/include unlink_files;)
	$(Q)$(Q)$(call $(AQ_color_printf),$(RED),$(NAME),🗑️  remove Objects && Dependency file)
	$(Q)$(RM) $(OBJS) $(DEPS)
	$(Q)$(RM) $(NAME)

fclean: clean

re:
	$(Q)$(MAKE) fclean
	$(Q)$(MAKE) all

.PHONY: all clean fclean re