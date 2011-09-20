###############################################################################
# $Id: Makefile,v 1.7 2008/03/10 14:00:00 faust Exp $
###############################################################################

include ../Makefile.conf

.PHONY: all $(STG_LIBS)
.PHONY: clean install uninstall includes
all: $(STG_LIBS)

$(STG_LIBS):
	$(MAKE) $(MAKECMDGOALS) -C $@

includes: all

clean: all

install: all

uninstall: all
