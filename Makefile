all: build/svm-image-gen

DEBUG=-g
BUILDENV=${DEBUG}
OSQPDIR=/home/haroldo/opt/osqp
LDFLAGS=${OSQPDIR}/lib64/libosqpstatic.a -lm $(shell pkg-config --libs lua)
CFLAGS=-I${OSQPDIR}/include/osqp ${BUILDENV}

build/svm-image-gen: main.o svm.o
	${CC} ${CFLAGS} ${OSQPLIB}  -o $@ $^ ${LDFLAGS}

clean:
	rm -f *.o
	rm build/*
