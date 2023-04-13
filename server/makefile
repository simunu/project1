APPNAME=server_epoll2
HEADPATH=~/xiongliling/zlog/build_linux/include
LIBPATH=~/xiongliling/zlog/build_linux/lib

CFLAGS+=-I ${HEADPATH}
LDFLAGS+=-L ${LIBPATH}

CC=gcc

all:
	         ${CC} server_epoll2.c -o ${APPNAME} ${CFLAGS} ${LDFLAGS} -lzlog -lpthread -lsqlite3

run:
	        export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:/home/iot23/xiongliling/zlog/build_linux/lib
			        ./${APPNAME}  -p 8809
					
