all:
	$(Q)$(call color_printf,$(BLUE),$(NAME),✨ starting compile $(NAME))
	$(Q)$(foreach dir,$(DIRS),$(MAKE) TOPDIR=$(TOPDIR) SRCDIR=`pwd` -C $(dir) $@;)
	$(Q)$(MAKE) $(NAME)
	$(Q)$(call color_printf,$(BOLD_PURPLE),$(NAME),✨ compiled!)

%.o: %.cpp
	$(AQ)$(COMPILE.cpp) $(OUTPUT_OPTION) $<

$(NAME): $(OBJS) $(SUBS)
	$(Q)$(LINK.cpp) $^ $(LOADLIBES) $(LDLIBS) -o $@
	$(Q)$(MAKE) files=$(NAME) src_dir=`pwd` dst_dir=$(TOPDIR) link_files

clean:
	$(Q)$(foreach dir,$(DIRS),$(MAKE) TOPDIR=$(TOPDIR) SRCDIR=`pwd` -C $(dir) $@;)
	$(Q)$(RM) $(OBJS) $(DEPS)

fclean: clean
	$(Q)$(foreach dir,$(DIRS),$(MAKE) TOPDIR=$(TOPDIR) SRCDIR=`pwd` -C $(dir) $@;)
	$(Q)$(MAKE) files=$(NAME) src_dir=`pwd` dst_dir=$(TOPDIR) unlink_files
	$(Q)$(RM) $(NAME)

re:
	$(Q)$(MAKE) fclean
	$(Q)$(MAKE) all

.PHONY: all clean fclean re