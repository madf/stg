.PHONY: all clean

all: book/index.html help.pdf

book/index.html: ch*.xml help.xml
	mkdir -p book/images
	cp images/*.png book/images/
	xsltproc --stringparam chunker.output.encoding "utf-8" -o book/ --xinclude /usr/share/sgml/docbook/xsl-stylesheets/html/chunk.xsl help.xml

help.pdf: ch*.xml help.xml
	dblatex -b xetex -I. help.xml

clean:
	rm -r book
	rm help.pdf
