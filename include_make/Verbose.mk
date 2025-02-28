# all verbose
AQ := $(if $(filter 1,$(NOTALL)),,@)
AQ_color_printf := $(if $(filter @,$(AQ)),,color_printf)

# verbose
Q := $(if $(filter 1,$(V) $(VERBOSE)),,@)