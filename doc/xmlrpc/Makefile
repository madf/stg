all:
	xsltproc --stringparam chunker.output.encoding "utf-8" -o book/ --xinclude /usr/share/sgml/docbook/xsl-stylesheets/html/chunk.xsl API.xml

clean:
	rm book/*html
