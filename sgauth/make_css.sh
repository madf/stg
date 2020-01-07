#!/bin/sh

OUT_FILE=css.h


echo "const char * css =" > $OUT_FILE
echo "\"/*------*/\\\\n\"" >> $OUT_FILE
sed -e 's/$/\\n"/g' -e 's/^/"/g' sgauth.css >> $OUT_FILE
echo ";" >> $OUT_FILE

