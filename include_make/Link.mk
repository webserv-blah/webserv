link_files::
	@mkdir -p $(dst_dir);
	$(Q)$(foreach file,$(files), $(call color_printf,$(CYAN),$(file),🔗 linking file\n)ln -sf $(src_dir)/$(file) $(dst_dir);)
	$(Q)$(foreach file,$(files), ln -sf $(src_dir)/$(file) $(dst_dir)/$(file);)

unlink_files::
	$(Q)$(foreach file,$(files), $(call color_printf,$(GRAY),$(file),🚫 unlinking file\n)$(RM) $(dst_dir)/$(file);)
	$(Q)$(foreach file,$(files), $(RM) $(dst_dir)/$(file);)
