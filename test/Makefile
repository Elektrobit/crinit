TESTS = utest-crinit-set-verbose

MOCKS_PATH = $(realpath ./mocks)

CFLAGS += -I$(MOCKS_PATH)
LDFLAGS += -L$(MOCKS_PATH)

LIBS += -lcmocka -lmockfunctions

export CFLAGS LDFLAGS CC SRC_DIR LIBS

all: $(TESTS)

$(TESTS): $(MOCKS_PATH)/libmockfunctions.a
	$(MAKE) -C $@

$(MOCKS_PATH)/libmockfunctions.a: $(MOCKS_PATH)/*.c
	$(MAKE) -C $(MOCKS_PATH)

clean:
	for subdir in $(TESTS) $(MOCKS_PATH); \
		do \
		    $(MAKE) -C $${subdir} clean; \
		done

.PHONY: all $(TESTS) clean
