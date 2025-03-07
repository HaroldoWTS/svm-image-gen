all: svm

OSQPDIR=/home/haroldo/opt/osqp
LDFLAGS=${OSQPDIR}/lib64/libosqpstatic.a -lm $(shell pkg-config --libs lua)
CFLAGS=-I${OSQPDIR}/include/osqp

svm: luaosqp.o
	${CC} ${CFLAGS} ${OSQPLIB} $^ -o $@ ${LDFLAGS}

