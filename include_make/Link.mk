link_files::
	@mkdir -p $(dst_dir);
	$(foreach file,$(files), $(call color_printf,$(CYAN),$(file),🔗 linking file\n)ln -sf $(src_dir)/$(file) $(dst_dir);)
	$(foreach file,$(files), ln -sf $(src_dir)/$(file) $(dst_dir)/$(file);)

unlink_files::
	$(foreach file,$(files), $(call color_printf,$(GRAY),$(file),🚫 unlinking file\n)$(RM) $(dst_dir)/$(file);)
	$(foreach file,$(files), $(RM) $(dst_dir)/$(file);)