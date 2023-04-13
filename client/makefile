APPNAME=client_test2
HEADPATH=~/xiongliling/zlog/build_linux/include
LIBPATH=~/xiongliling/zlog/build_linux/lib

CFLAGS+=-I ${HEADPATH}
LDFLAGS+=-L ${LIBPATH}

CC=gcc

all:
	 ${CC} client_test2.c -o ${APPNAME} ${CFLAGS} ${LDFLAGS} -lzlog -lpthread -lsqlite3

run:
	export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:/home/iot23/xiongliling/zlog/build_linux/lib
	./${APPNAME} -h 127.0.0.1 -p 8809 -t 10

