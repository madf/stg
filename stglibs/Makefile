###############################################################################
# $Id: Makefile,v 1.7 2008/03/10 14:00:00 faust Exp $
###############################################################################

include ../Makefile.conf

.PHONY: all $(STG_LIBS)
.PHONY: clean install uninstall includes
all: $(STG_LIBS)
	for LIB in $(STG_LIBS); do\
	    $(MAKE) $(MAKECMDGOALS) -C $$LIB;\
	done

includes: all

clean: all

install: all

uninstall: all
