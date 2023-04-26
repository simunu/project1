APPNAME=main
CC = gcc
HEADPATH=~/xiongliling/zlog/build_linux/include
LIBPATH=~/xiongliling/zlog/build_linux/lib

CFLAGS+=-I ${HEADPATH}
LDFLAGS+=-L ${LIBPATH}


main:main.o get_zlog.o socket.o get_temp.o database.o data_pack.o
	${CC} main.o get_zlog.o socket.o get_temp.o database.o data_pack.o -o ${APPNAME}  ${CFLAGS} ${LDFLAGS} -lzlog -lpthread -lsqlite3 
main.o:main.c main.h
	${CC} -c main.c -o main.o
get_zlog.o:get_zlog.c zlog.h
	${CC} -c get_zlog.c -o get_zlog.o ${CFLAGS} ${LDFLAGS} -lzlog -lpthread
socket.o:socket.c socket.h
	${CC} -c socket.c -o socket.o
get_temp.o:get_temp.c get_temp.h
	${CC} -c get_temp.c -o get_temp.o
database.o:database.c database.h
	${CC} -c database.c -o database.o -lsqlite3
data_pack.o:data_pack.c data_pack.h
	${CC} -c data_pack.c -o data_pack.o

run:
	export LD_LIBRARY_PATH=$${LD_LIBRARY_PATH}:/home/iot23/xiongliling/zlog/build_linux/lib&&./${APPNAME}  -h 127.0.0.1 -p 8808 -t 10 

clean:
	rm -rf *.o main
