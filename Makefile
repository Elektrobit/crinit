CC      ?= $(CROSS_COMPILE)gcc
DOXYGEN ?= doxygen
PLANTUML ?= plantuml

BINS = crinit crinit_parsecheck crinit-ctl

SRCDIR = src

OBJDIR = obj
LIBOBJDIR = lobj
LIBDIR = lib
INCDIR = inc
IMGDIR = images

SRCS =  crinit.c confparse.c taskdb.c procdip.c logio.c globopt.c minsetup.c thrpool.c notiserv.c rtimcmd.c rtimopmap.c
LIBSRCS = $(SRCS) crinit-client.c sockcom.c

OBJS = $(addprefix $(OBJDIR)/, ${SRCS:.c=.o})
LIBOBJS = $(addprefix $(LIBOBJDIR)/, ${LIBSRCS:.c=.o})

IMGS = $(addprefix $(IMGDIR)/, notiserv_sock_comm_seq.svg  sock_comm_str.svg)

CFLAGS += -O2 -std=c99 -D_DEFAULT_SOURCE -Wall -Werror -pedantic -fstack-protector-strong -ffunction-sections -fdata-sections -D_FORTIFY_SOURCE=2 -I$(INCDIR)
LDFLAGS += -L$(LIBDIR) -Wl,--gc-sections

LIBCFLAGS := -fPIC -fvisibility=hidden $(CFLAGS)
LIBLDFLAGS := $(LDFLAGS)

LIBS = -pthread

GIT_REVISION = $(shell git rev-parse --short HEAD)

DIRS = $(OBJDIR) $(LIBOBJDIR) $(LIBDIR)
$(shell mkdir -p $(DIRS))

all: crinit crinit_parsecheck lib/libcrinit-client.so crinit-ctl

crinit: ${OBJS}
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

crinit_parsecheck: $(OBJDIR)/crinit_parsecheck.o $(OBJDIR)/confparse.o $(OBJDIR)/taskdb.o $(OBJDIR)/logio.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

crinit-ctl: $(OBJDIR)/crinit-ctl.o $(OBJDIR)/logio.o lib/libcrinit-client.so
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(filter-out %.so, $^) $(LIBS) -lcrinit-client

lib/libcrinit-client.so: ${LIBOBJS}
	$(CC) -shared $(LIBCFLAGS) -o $@ $^ $(LIBLDFLAGS)
	@echo "Symbols in library:"
	@nm -D $@

$(OBJDIR)/%.o : $(SRCDIR)/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

$(LIBOBJDIR)/%.o : $(SRCDIR)/%.c
	$(CC) -c -o $@ $< $(LIBCFLAGS)

$(IMGDIR)/%.svg : $(IMGDIR)/%.plantuml
	$(PLANTUML) -tsvg $<

doxygen: $(IMGS)
	mkdir -p doc/html/images
	cp images/*.svg doc/html/images
	$(DOXYGEN)

rpmbuild: clean
	mkdir -p packaging/rpmbuild/SOURCES
	tar czf packaging/rpmbuild/SOURCES/crinit-git-$(GIT_REVISION).tar.gz src/ inc/ config/ images/ Makefile Doxyfile README.md
	cd packaging/rpmbuild && rpmbuild --define "_topdir $(shell pwd)/packaging/rpmbuild" --define "gitrev_ $(GIT_REVISION)" -v -ba SPECS/crinit-git.spec

clean:
	rm -rvf $(BINS) $(DIRS) $(IMGS)
	rm -rf ./doc/*
	rm -rf \
		packaging/rpmbuild/SOURCES \
		packaging/rpmbuild/BUILD \
		packaging/rpmbuild/BUILDROOT \
		packaging/rpmbuild/RPMS \
		packaging/rpmbuild/SRPMS
.PHONY: all clean doxygen rpmbuild
