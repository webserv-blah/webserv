all:
	$(Q)$(foreach dir,$(DIRS),$(MAKE) TOPDIR=$(TOPDIR) SRCDIR=`pwd` DEBUG=$(DEBUG) ADDRESS=$(ADDRESS) -C $(dir) $@ >/dev/null;)
	$(Q)$(MAKE) DEBUG=$(DEBUG) ADDRESS=$(ADDRESS) $(NAME) >/dev/null
	@$(call color_printf,$(BOLD_PURPLE),$(NAME),✨ compiled!)

$(NAME): $(OBJS) $(SUBS)
	$(Q)$(LINK.cpp) $^ $(LOADLIBES) $(LDLIBS) -o $@
	$(Q)$(MAKE) files=$(NAME) src_dir=`pwd` dst_dir=$(TOPDIR) link_files

clean:
	$(Q)$(foreach dir,$(DIRS),$(MAKE) TOPDIR=$(TOPDIR) SRCDIR=`pwd` -C $(dir) $@ >/dev/null;)
	$(Q)$(RM) $(OBJS) $(DEPS)
	@$(call color_printf,$(RED),"Clean",🧹 object files removed)

fclean: clean
	$(Q)$(foreach dir,$(DIRS),$(MAKE) TOPDIR=$(TOPDIR) SRCDIR=`pwd` -C $(dir) $@ >/dev/null;)
	$(Q)$(MAKE) files=$(NAME) src_dir=`pwd` dst_dir=$(TOPDIR) unlink_files >/dev/null
	$(Q)$(RM) $(NAME)
	@$(call color_printf,$(RED),"Full Clean",🧹 binary files removed)

re:
	@$(call color_printf,$(YELLOW),"Rebuild",🔄 rebuilding project...)
	$(Q)$(MAKE) fclean >/dev/null
	$(Q)$(MAKE) all >/dev/null
	@$(call color_printf,$(GREEN),"Rebuild",✅ project rebuilt successfully!)

.PHONY: all clean fclean re