SUBDIRS = fig installation user
docbookdir = $(datadir)/scenic/docbook
docbook_DATA = \
	glossary.xml \
	installation-manual.xml \
	user-manual.xml

EXTRA_DIST = glossary.xml

html-local:
	mkdir -p html/fig ; cp -r fig/*.png fig/*.jpg html/fig/ ; \
	for doc in installation-manual user-manual ; \
	do \
		mkdir -p html/$${doc} ; \
		gnome-doc-tool html -o html/$${doc}/$${doc}.html $${doc}.xml ; \
		sed -i 's!src="fig/!src="../fig/!' html/$${doc}/*.html ; \
		( cd html/$${doc} ; ln -sf $${doc}.html index.html ) ; \
	done

clean-local:
	rm -rf html installation-manual.xml user-manual.xml

xmlcheck: installation-manual.xml user-manual.xml
	xmllint --noout --noent --xinclude --postvalid $^

