BINS = libmockfunctions.a
OBJS = mock_globopt_set.o

all: $(BINS)

$(BINS): $(OBJS)
	$(AR) -crs $@ $^

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -fv $(BINS) *.o

.PHONY: all clean
