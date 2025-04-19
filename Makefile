.PHONY: all debug release clean

all: build/svm-image-gen

LDFLAGS=-lm $(shell pkg-config --libs lua)
INCLUDE=
CFLAGS=${INCLUDE}
BUILD ?= debug

ifeq ($(BUILD),release)
    CFLAGS += -O3 -DNDEBUG
else
    CFLAGS += -g -O0 -Wall -Wextra
endif

debug:
	$(MAKE) BUILD=debug

release:
	$(MAKE) BUILD=release


build/svm-image-gen: main.o svm.o smo.o
	${CC} ${CFLAGS} -o $@ $^ ${LDFLAGS}

clean:
	rm -f *.o
	rm build/*
