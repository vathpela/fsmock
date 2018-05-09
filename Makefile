TOPDIR = $(shell echo $$PWD)

include $(TOPDIR)/Make.version
include $(TOPDIR)/Make.rules
include $(TOPDIR)/Make.defaults
include $(TOPDIR)/Make.coverity
include $(TOPDIR)/Make.scan-build

SUBDIRS := src docs

all : | fsmock.spec Make.version
all :
	@set -e ; for x in $(SUBDIRS) ; do \
		$(MAKE) -C $$x $@ ; \
	done

install :
	@set -e ; for x in $(SUBDIRS) ; do \
		$(MAKE) -C $$x $@ ; \
	done

abidw abicheck fsmock fsmock-static static:
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

fsmock.spec : | Makefile Make.version

clean :
	@set -e ; for x in $(SUBDIRS) ; do \
		$(MAKE) -C $$x $@ ; \
	done
	@rm -vf fsmock.spec

test-archive: abicheck fsmock.spec
	@rm -rf /tmp/fsmock-$(GITTAG) /tmp/fsmock-$(GITTAG)-tmp
	@mkdir -p /tmp/fsmock-$(GITTAG)-tmp
	@git archive --format=tar $(shell git branch | awk '/^*/ { print $$2 }') | ( cd /tmp/fsmock-$(GITTAG)-tmp/ ; tar x )
	@git diff | ( cd /tmp/fsmock-$(GITTAG)-tmp/ ; patch -s -p1 -b -z .gitdiff )
	@mv /tmp/fsmock-$(GITTAG)-tmp/ /tmp/fsmock-$(GITTAG)/
	@cp fsmock.spec /tmp/fsmock-$(GITTAG)/
	@dir=$$PWD; cd /tmp; tar -c --bzip2 -f $$dir/fsmock-$(GITTAG).tar.bz2 fsmock-$(GITTAG)
	@rm -rf /tmp/fsmock-$(GITTAG)
	@echo "The archive is in fsmock-$(GITTAG).tar.bz2"

bumpver :
	@echo VERSION=$(GITTAG) > Make.version
	@git add Make.version
	git commit -m "Bump version to $(GITTAG)" -s

tag:
	git tag -s $(GITTAG) refs/heads/master

archive: abicheck bumpver abidw tag fsmock.spec
	@rm -rf /tmp/fsmock-$(GITTAG) /tmp/fsmock-$(GITTAG)-tmp
	@mkdir -p /tmp/fsmock-$(GITTAG)-tmp
	@git archive --format=tar $(GITTAG) | ( cd /tmp/fsmock-$(GITTAG)-tmp/ ; tar x )
	@mv /tmp/fsmock-$(GITTAG)-tmp/ /tmp/fsmock-$(GITTAG)/
	@cp fsmock.spec /tmp/fsmock-$(GITTAG)/
	@dir=$$PWD; cd /tmp; tar -c --bzip2 -f $$dir/fsmock-$(GITTAG).tar.bz2 fsmock-$(GITTAG)
	@rm -rf /tmp/fsmock-$(GITTAG)
	@echo "The archive is in fsmock-$(GITTAG).tar.bz2"


