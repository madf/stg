###############################################################################
# $Id: Makefile,v 1.1 2007/09/26 13:55:45 faust Exp $
###############################################################################

include ../../../Makefile.conf

.PHONY: clean install uninstall
.PHONY: all $(PLUGINS)

all: $(PLUGINS)

$(PLUGINS):
	$(MAKE) $(MAKECMDGOALS) -C $@

clean: all

install: all

uninstall: all

