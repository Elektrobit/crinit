CC      ?= $(CROSS_COMPILE)gcc
DOXYGEN ?= doxygen

BINS = crinit crinit_parsecheck

INC_DIR = ./inc

CFLAGS += -O2 -std=c99 -D_DEFAULT_SOURCE -Wall -Werror -pedantic -fstack-protector-strong -ffunction-sections -fdata-sections -D_FORTIFY_SOURCE=2 -I$(INC_DIR)
LDFLAGS += -Wl,--gc-sections

LIBS = -pthread

GIT_REVISION = $(shell git rev-parse --short HEAD)

all: crinit crinit_parsecheck

crinit: src/crinit.o src/confparse.o src/taskdb.o src/procdip.o src/logio.o src/globopt.o src/minsetup.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

crinit_parsecheck: src/crinit_parsecheck.o src/confparse.o src/taskdb.o src/logio.o src/globopt.o src/minsetup.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

doxygen:
	mkdir -p doc/html
	cp -a images doc/html
	$(DOXYGEN)

rpmbuild: clean
	mkdir -p packaging/rpmbuild/SOURCES
	tar czf packaging/rpmbuild/SOURCES/crinit-git-$(GIT_REVISION).tar.gz src/ inc/ config/ images/ Makefile Doxyfile README.md
	cd packaging/rpmbuild && rpmbuild --define "_topdir $(shell pwd)/packaging/rpmbuild" --define "gitrev_ $(GIT_REVISION)" -v -ba SPECS/crinit-git.spec

clean:
	rm -rvf $(BINS) ./src/*.o
	rm -rf ./doc/*
	rm -rf \
		packaging/rpmbuild/SOURCES \
		packaging/rpmbuild/BUILD \
		packaging/rpmbuild/BUILDROOT \
		packaging/rpmbuild/RPMS \
		packaging/rpmbuild/SRPMS
.PHONY: all clean doxygen rpmbuild
