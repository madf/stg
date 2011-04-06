all: book
	xsltproc --stringparam base.dir book/ --xinclude --xincludestyle /usr/share/sgml/docbook/stylesheet/xsl/docbook-xsl/html/chunk.xsl API.xml

book:
	mkdir book

clean:
	rm book/*
