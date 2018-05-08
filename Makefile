TOPDIR = $(shell echo $$PWD)

include $(TOPDIR)/Make.version
include $(TOPDIR)/Make.rules
include $(TOPDIR)/Make.defaults
include $(TOPDIR)/Make.coverity
include $(TOPDIR)/Make.scan-build

SUBDIRS := src docs

all : | bdsim.spec Make.version
all :
	@set -e ; for x in $(SUBDIRS) ; do \
		$(MAKE) -C $$x $@ ; \
	done

install :
	@set -e ; for x in $(SUBDIRS) ; do \
		$(MAKE) -C $$x $@ ; \
	done

abidw abicheck bdsim bdsim-static static:
	$(MAKE) -C src $@

$(SUBDIRS) :
	$(MAKE) -C $@

brick : all
	@set -e ; for x in $(SUBDIRS) ; do $(MAKE) -C $${x} test ; done

a :
	@if [ $${EUID} != 0 ]; then \
		echo no 1>&2 ; \
		exit 1 ; \
	fi

.PHONY: $(SUBDIRS) a brick

GITTAG = $(shell bash -c "echo $$(($(VERSION) + 1))")

bdsim.spec : | Makefile Make.version

clean :
	@set -e ; for x in $(SUBDIRS) ; do \
		$(MAKE) -C $$x $@ ; \
	done
	@rm -vf bdsim.spec

test-archive: abicheck bdsim.spec
	@rm -rf /tmp/bdsim-$(GITTAG) /tmp/bdsim-$(GITTAG)-tmp
	@mkdir -p /tmp/bdsim-$(GITTAG)-tmp
	@git archive --format=tar $(shell git branch | awk '/^*/ { print $$2 }') | ( cd /tmp/bdsim-$(GITTAG)-tmp/ ; tar x )
	@git diff | ( cd /tmp/bdsim-$(GITTAG)-tmp/ ; patch -s -p1 -b -z .gitdiff )
	@mv /tmp/bdsim-$(GITTAG)-tmp/ /tmp/bdsim-$(GITTAG)/
	@cp bdsim.spec /tmp/bdsim-$(GITTAG)/
	@dir=$$PWD; cd /tmp; tar -c --bzip2 -f $$dir/bdsim-$(GITTAG).tar.bz2 bdsim-$(GITTAG)
	@rm -rf /tmp/bdsim-$(GITTAG)
	@echo "The archive is in bdsim-$(GITTAG).tar.bz2"

bumpver :
	@echo VERSION=$(GITTAG) > Make.version
	@git add Make.version
	git commit -m "Bump version to $(GITTAG)" -s

tag:
	git tag -s $(GITTAG) refs/heads/master

archive: abicheck bumpver abidw tag bdsim.spec
	@rm -rf /tmp/bdsim-$(GITTAG) /tmp/bdsim-$(GITTAG)-tmp
	@mkdir -p /tmp/bdsim-$(GITTAG)-tmp
	@git archive --format=tar $(GITTAG) | ( cd /tmp/bdsim-$(GITTAG)-tmp/ ; tar x )
	@mv /tmp/bdsim-$(GITTAG)-tmp/ /tmp/bdsim-$(GITTAG)/
	@cp bdsim.spec /tmp/bdsim-$(GITTAG)/
	@dir=$$PWD; cd /tmp; tar -c --bzip2 -f $$dir/bdsim-$(GITTAG).tar.bz2 bdsim-$(GITTAG)
	@rm -rf /tmp/bdsim-$(GITTAG)
	@echo "The archive is in bdsim-$(GITTAG).tar.bz2"


