CC      ?= $(CROSS_COMPILE)gcc
DOXYGEN ?= doxygen
PLANTUML ?= plantuml

BINS = crinit crinit_parsecheck crinit-ctl

SRCDIR = $(realpath src)

OBJDIR = obj
LIBOBJDIR = lobj
LIBDIR = lib
INCDIR = inc
IMGDIR = images

SRCS =  crinit.c confparse.c taskdb.c procdip.c logio.c globopt.c minsetup.c thrpool.c notiserv.c rtimcmd.c rtimopmap.c
LIBSRCS = crinit-client.c logio.c rtimcmd.c rtimopmap.c globopt.c sockcom.c

OBJS = $(addprefix $(OBJDIR)/, ${SRCS:.c=.o})
LIBOBJS = $(addprefix $(LIBOBJDIR)/, ${LIBSRCS:.c=.o})

IMGS = $(addprefix $(IMGDIR)/, notiserv_sock_comm_seq.svg  sock_comm_str.svg)

CFLAGS += -O2 -std=c99 -D_DEFAULT_SOURCE -Wall -Wswitch-enum -Werror -pedantic -fstack-protector-strong -ffunction-sections -fdata-sections -D_FORTIFY_SOURCE=2 -I$(realpath $(INCDIR))
LDFLAGS += -L$(LIBDIR) -Wl,--gc-sections

GIT_REVISION = $(shell git rev-parse --short HEAD)
VERSION = $(shell cat VERSION)

LIBCFLAGS := -fPIC -fvisibility=hidden $(CFLAGS)
LIBLDFLAGS := $(LDFLAGS) -Wl,-soname,libcrinit-client.so.$(VERSION)

LIBS = -pthread

DIRS = $(OBJDIR) $(LIBOBJDIR) $(LIBDIR)
$(shell mkdir -p $(DIRS))

# If we're cross-compiling for aarch64, set the RPM target accordingly
RPM_TARGET = $(findstring aarch64, $(CC))
ifeq ($(RPM_TARGET),)
RPM_TARGET = x86_64
endif

export AR CC CFLAGS LDFLAGS SRCDIR

all: crinit crinit_parsecheck lib/libcrinit-client.so crinit-ctl

crinit: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

crinit_parsecheck: $(OBJDIR)/crinit_parsecheck.o $(OBJDIR)/confparse.o $(OBJDIR)/taskdb.o $(OBJDIR)/logio.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

crinit-ctl: $(OBJDIR)/crinit-ctl.o $(OBJDIR)/logio.o lib/libcrinit-client.so
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(filter-out %.so, $^) $(LIBS) -lcrinit-client

lib/libcrinit-client.so.$(VERSION): $(LIBOBJS)
	$(CC) -shared $(LIBCFLAGS) -o $@ $^ $(LIBLDFLAGS)
	@echo "Symbols in library:"
	@nm -D $@

lib/libcrinit-client.so: lib/libcrinit-client.so.$(VERSION)
	ln -sr lib/libcrinit-client.so.$(VERSION) lib/libcrinit-client.so

$(OBJDIR)/%.o : $(SRCDIR)/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

$(LIBOBJDIR)/%.o : $(SRCDIR)/%.c
	$(CC) -c -o $@ $< $(LIBCFLAGS)

tests:
	$(MAKE) -C test

$(IMGDIR)/%.svg : $(IMGDIR)/%.plantuml
	$(PLANTUML) -tsvg $<

doxygen: $(IMGS)
	mkdir -p doc/html/images
	cp images/*.svg doc/html/images
	$(DOXYGEN)

rpmbuild: clean
	mkdir -p packaging/rpmbuild/SOURCES
	tar czf packaging/rpmbuild/SOURCES/crinit-$(VERSION)git$(GIT_REVISION).tar.gz \
		src/ inc/ test/ config/ images/ VERSION CMakeLists.txt Doxyfile README.md
	cd packaging/rpmbuild && \
		rpmbuild --target $(RPM_TARGET) \
		    --define "_topdir $(shell pwd)/packaging/rpmbuild" \
		    --define "crinit_version_ $(VERSION)" \
		    --define "gitrev_ $(GIT_REVISION)" \
		    -v -ba SPECS/crinit-git.spec

clean:
	rm -rvf $(BINS) $(DIRS) $(IMGS) $(SRCDIR)/*.o
	rm -rf ./doc/*
	rm -rf \
		packaging/rpmbuild/SOURCES \
		packaging/rpmbuild/BUILD \
		packaging/rpmbuild/BUILDROOT \
		packaging/rpmbuild/RPMS \
		packaging/rpmbuild/SRPMS
	$(MAKE) -C test clean

.PHONY: all clean doxygen rpmbuild
