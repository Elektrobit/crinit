BINS = utest-crinit-set-verbose
OBJS = utest-crinit-set-verbose.o \
	   utest-crinit-set-verbose-success.o \
	   utest-crinit-set-verbose-globopt-error.o \
	   $(SRCDIR)/crinit-client.o \
	   $(SRCDIR)/globopt.o \
	   $(SRCDIR)/logio.o

LDFLAGS += -Wl,--wrap=EBCL_globOptSet
CFLAGS += -DEBCL_LIB_CONSTRUCTOR="" -DEBCL_LIB_DESTRUCTOR="" -Wno-unused-function

all: $(BINS)

$(BINS): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -fv $(BINS) *.o

.PHONY: all clean
