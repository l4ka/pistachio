.EXPORT_ALL_VARIABLES:


-include Makeconf.local

all $(filter-out all, $(MAKECMDGOALS)):
	@$(MAKE) -f $(SRCDIR)/Makefile $@

