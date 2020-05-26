#!/bin/sh

INCLUDES="-I include/
          -I stglibs/common.lib/include/
          -I stglibs/conffiles.lib/include/
          -I stglibs/crypto.lib/include/
          -I stglibs/dotconfpp.lib/include/
          -I stglibs/ia.lib/include/
          -I stglibs/ibpp.lib/include/
          -I stglibs/logger.lib/include/
          -I stglibs/pinger.lib/include/
          -I stglibs/scriptexecuter.lib/include/
          -I stglibs/smux.lib/include/
          -I stglibs/srvconf.lib/include/"

PROJECTS="projects/libs
          projects/rscriptd
          projects/sgauth
          projects/sgconf
          projects/sgconf_xml
          projects/sgconv
          projects/stargazer"

cppcheck --std=c++03 ${INCLUDES} --enable=all -q ${PROJECTS}
