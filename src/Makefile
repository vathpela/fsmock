SRCDIR = $(realpath .)
TOPDIR = $(realpath ..)

include $(TOPDIR)/Make.version
include $(TOPDIR)/Make.rules
include $(TOPDIR)/Make.defaults

LIBTARGETS=libbdsim.so
STATICLIBTARGETS=libbdsim.a
BINTARGETS=
STATICBINTARGETS=
PCTARGETS=bdsim.pc
TARGETS=$(LIBTARGETS) $(BINTARGETS) $(PCTARGETS)
STATICTARGETS=$(STATICLIBTARGETS) $(STATICBINTARGETS)

LIBBDSIM_SOURCES = error.c
LIBBDSIM_OBJECTS = $(patsubst %.S,%.o,$(patsubst %.c,%.o,$(LIBBDSIM_SOURCES)))
ALL_SOURCES=$(LIBBDSIM_SOURCES) $(wildcard *.h)

$(call deps-of,$(ALL_SOURCES)) : | deps
-include $(call deps-of,$(ALL_SOURCES))

all : $(TARGETS)

static : $(STATICTARGETS)

abidw : $(patsubst %.so,%.abixml,$(LIBTARGETS))
	git commit -s --amend $^

abicheck : $(patsubst %.so,%.abicheck,$(LIBTARGETS))

libbdsim.a : | $(GENERATED_SOURCES)
libbdsim.a : $(patsubst %.o,%.static.o,$(LIBBDSIM_OBJECTS))

libbdsim.so : $(LIBBDSIM_OBJECTS)
libbdsim.so : | $(GENERATED_SOURCES) libbdsim.map
libbdsim.so : LIBS=dl
libbdsim.so : MAP=libbdsim.map

deps : $(ALL_SOURCES)
	$(MAKE) -f $(SRCDIR)/Make.deps deps SOURCES="$(ALL_SOURCES)"

clean : 
	@rm -rfv *~ *.o *.a *.E *.so *.so.* *.pc *.bin .*.d *.map \
		$(TARGETS) $(STATICTARGETS)
	@# remove the deps files we used to create, as well.
	@rm -rfv .*.P .*.h.P *.S.P

install : all
	$(INSTALL) -d -m 755 $(DESTDIR)$(libdir)
	$(foreach x,$(LIBTARGETS), \
		$(INSTALL) -m 755 $(x) $(DESTDIR)$(libdir)/$(x).1.$(VERSION) ;\
		ln -fs $(x).1.$(VERSION) $(DESTDIR)$(libdir)/$(x).1 ;\
		ln -fs $(x).1.$(VERSION) $(DESTDIR)$(libdir)/$(x) ;\
		)
	$(INSTALL) -d -m 755 $(DESTDIR)$(PCDIR)
	$(foreach x, $(PCTARGETS), $(INSTALL) -m 644 $(x) $(DESTDIR)$(PCDIR) ;)
	#$(INSTALL) -d -m 755 $(DESTDIR)$(bindir)
	$(foreach x, $(BINTARGETS), $(INSTALL) -m 755 $(x) $(DESTDIR)$(bindir);)

.PHONY: test deps
.SECONDARY : libbdsim.so.1.$(VERSION) libbdsim.so.1
