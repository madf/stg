.PHONY: all clean

all: book/index.html help.pdf

book/index.html: ch*.xml help.xml
	mkdir -p book/images
	cp images/*.png book/images/
	xsltproc -o book/ --xinclude ../xslt/html/chunk.xsl help.xml

help.pdf: ch*.xml help.xml
	dblatex -b xetex -I. help.xml

clean:
	rm -r book
	rm help.pdf
