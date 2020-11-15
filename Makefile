# compile macro
CC		= g++
CFLAGS	= -w
LDFLAGS	= -lm
INCLUDES= -I./include

# user modify: target and objects
EXENAME = test.out
DAEMON = daemon.out
OBJMAIN = obj/main.o
OBJDAEMON = obj/daemon.o
OBJECTS	= obj/utils.o obj/packetio.o obj/core.o obj/device.o obj/ip.o obj/forward.o obj/myArp.o

# make target
all:${OBJECTS} ${OBJMAIN} ${OBJDAEMON}
	#${CC} -o ${EXENAME} ${OBJECTS} ${LDFLAGS} ${INCLUDES} -lpcap
	${CC} -o ${DAEMON} ${OBJECTS} ${OBJDAEMON} ${LDFLAGS} ${INCLUDES} -lpcap -lpthread
	sudo ./${DAEMON}

run:${OBJECTS} ${OBJMAIN}
	#${CC} -o ${EXENAME} ${OBJECTS} ${OBJMAIN} ${LDFLAGS} ${INCLUDES} -lpcap
	#sudo ./${EXENAME}
	${CC} -o ${DAEMON} ${OBJECTS} ${OBJDAEMON} ${LDFLAGS} ${INCLUDES} -lpcap -lpthread
	sudo ./${DAEMON}
daemon:${OBJECTS} ${OBJDAEMON}
	#rm -rf ${EXENAME} ${DAEMON} ${OBJECTS} ${OBJDAEMON} ${OBJMAIN};
	${CC} -o ${DAEMON} ${OBJECTS} ${OBJDAEMON} ${LDFLAGS} ${INCLUDES} -lpcap -lpthread
	sudo ./${DAEMON}
# make clean
clean:
	rm -rf ${EXENAME} ${DAEMON} ${OBJECTS} ${OBJDAEMON} ${OBJMAIN}

# dependence	
obj/daemon.o: src/daemon.cc src/core.cc src/utils.cc src/packetio.cc src/ip.cc include/ip.h
	${CC} ${CFLAGS} ${INCLUDES} -c src/daemon.cc -o obj/daemon.o -lpcap
obj/main.o: src/main.cc src/core.cc src/utils.cc src/packetio.cc src/ip.cc src/myArp.cc
	${CC} ${CFLAGS} ${INCLUDES} -c src/main.cc -o obj/main.o -lpcap
obj/packetio.o : src/packetio.cc src/core.cc
	${CC} ${CFLAGS} ${INCLUDES} -c src/packetio.cc -o obj/packetio.o -lpcap
obj/device.o : src/device.cc src/core.cc
	${CC} ${CFLAGS} ${INCLUDES} -c src/device.cc -o obj/device.o -lpcap
obj/core.o : src/core.cc
	${CC} ${CFLAGS} ${INCLUDES} -c src/core.cc -o obj/core.o -lpcap
obj/utils.o : src/utils.cc
	${CC} ${CFLAGS} ${INCLUDES} -c src/utils.cc -o obj/utils.o -lpcap
obj/ip.o : src/ip.cc src/core.cc
	${CC} ${CFLAGS} ${INCLUDES} -c src/ip.cc -o obj/ip.o
obj/myArp.o : src/myArp.cc src/ip.cc
	${CC} ${CFLAGS} ${INCLUDES} -c src/myArp.cc -o obj/myArp.o
obj/forward.o : src/forward.cc src/ip.cc src/core.cc
	${CC} ${CFLAGS} ${INCLUDES} -c src/forward.cc -o obj/forward.o

