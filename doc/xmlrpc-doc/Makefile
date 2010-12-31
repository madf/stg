all:
	mkdir book
	xsltproc --xinclude --xincludestyle /usr/share/sgml/docbook/stylesheet/xsl/docbook-xsl/html/chunk.xsl API.xml
	mv *html book/

clean:
	rm -r book
