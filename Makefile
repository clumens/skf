# $Id: Makefile,v 1.3 2005/06/12 02:39:54 chris Exp $

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
	install -o root -g root -m 755 src/skf $(BINDIR)
	install -o root -g root -m 755 -d $(IMGDIR)
	install -o root -g root -m 644 graphics/*.png $(IMGDIR)

# Return source tree to its clean state.
clean:
	$(MAKE) -C src clean
