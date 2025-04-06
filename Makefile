.PHONY: all debug release clean

all: build/svm-image-gen

OSQPDIR=/home/haroldo/opt/osqp
LDFLAGS=${OSQPDIR}/lib64/libosqpstatic.a -lm $(shell pkg-config --libs lua)
INCLUDE=-I${OSQPDIR}/include/osqp
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


build/svm-image-gen: main.o svm.o
	${CC} ${CFLAGS} ${OSQPLIB}  -o $@ $^ ${LDFLAGS}

clean:
	rm -f *.o
	rm build/*
