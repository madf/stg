all:
	mkdir book
	xsltproc --xinclude --xincludestyle /usr/share/sgml/docbook/xsl-stylesheets/html/chunk.xsl API.xml
	mv *html book/

clean:
	rm -r book
