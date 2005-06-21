# $Id: Makefile,v 1.4 2005/06/21 22:25:13 chris Exp $

include Includes.mk

# Build SKF binary and any related programs for installation.
all:
	$(MAKE) -C src

# Build SKF binary suitable for testing in-place (doesn't require
# installation).
test:
	$(MAKE) -C src test

# Install to the system.  Make sure to look at Includes.mk first.
install: all
	install -d $(BINDIR)
	install -m 755 src/skf $(BINDIR)
	install -m 755 -d $(IMGDIR)
	install -m 644 graphics/*.png $(IMGDIR)

# Return source tree to its clean state.
clean:
	$(MAKE) -C src clean
