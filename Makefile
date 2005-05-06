all:
	$(MAKE) -C src

tags:
	( cd src && ctags -R )

clean:
	$(MAKE) -C src clean

distclean: clean
	-rm src/tags
