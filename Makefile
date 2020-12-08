# compile macro
CC		= g++
CFLAGS	= -w -g
LDFLAGS	= -lm -g
INCLUDES= -I./include

# user modify: target and objects
EXENAME = test.out
DAEMON = daemon.out
SENDER = sender.out
RECVER = recver.out

OBJMAIN = obj/main.o
OBJDAEMON = obj/daemon.o
OBJSENDER = obj/sender.o
OBJRECVER = obj/recver.o
OBJECTS	= obj/utils.o obj/packetio.o obj/core.o obj/device.o obj/ip.o obj/forward.o obj/myArp.o obj/tcp.o obj/socket.o

# make target
all:${OBJECTS} ${OBJMAIN} ${OBJDAEMON} ${OBJSENDER} ${OBJRECVER}
	${CC} -o ${DAEMON} ${OBJECTS} ${OBJDAEMON} ${LDFLAGS} ${INCLUDES} -lpcap -lpthread
	${CC} -o ${SENDER} ${OBJECTS} ${OBJSENDER} ${LDFLAGS} ${INCLUDES} -lpcap -lpthread
	${CC} -o ${RECVER} ${OBJECTS} ${OBJRECVER} ${LDFLAGS} ${INCLUDES} -lpcap -lpthread

test_l4_run: ${OBJECTS} ${OBJMAIN} ${OBJDAEMON} ${OBJSENDER} ${OBJRECVER}
	${CC} -o ${DAEMON} ${OBJECTS} ${OBJDAEMON} ${LDFLAGS} ${INCLUDES} -lpcap -lpthread
	${CC} -o ${SENDER} ${OBJECTS} ${OBJSENDER} ${LDFLAGS} ${INCLUDES} -lpcap -lpthread
	${CC} -o ${RECVER} ${OBJECTS} ${OBJRECVER} ${LDFLAGS} ${INCLUDES} -lpcap -lpthread
	./execNS ns3 ./recver.out &
	./execNS ns2 ./sender.out &

test_l4: ${OBJECTS} ${OBJMAIN} ${OBJDAEMON} ${OBJSENDER} ${OBJRECVER}
	${CC} -o ${DAEMON} ${OBJECTS} ${OBJDAEMON} ${LDFLAGS} ${INCLUDES} -lpcap -lpthread
	${CC} -o ${SENDER} ${OBJECTS} ${OBJSENDER} ${LDFLAGS} ${INCLUDES} -lpcap -lpthread
	${CC} -o ${RECVER} ${OBJECTS} ${OBJRECVER} ${LDFLAGS} ${INCLUDES} -lpcap -lpthread

clean:
	rm -rf ${EXENAME} ${DAEMON} ${SENDER} ${OBJECTS} ${OBJDAEMON} ${OBJMAIN}

# dependence	
obj/daemon.o: src/daemon.cc src/core.cc src/utils.cc src/packetio.cc src/ip.cc include/ip.h
	${CC} ${CFLAGS} ${INCLUDES} -c src/daemon.cc -o obj/daemon.o -lpcap
obj/recver.o: src/recver.cc src/core.cc src/utils.cc src/packetio.cc src/ip.cc include/ip.h
	${CC} ${CFLAGS} ${INCLUDES} -c src/recver.cc -o obj/recver.o -lpcap
obj/main.o: src/main.cc src/core.cc src/utils.cc src/packetio.cc src/ip.cc src/myArp.cc
	${CC} ${CFLAGS} ${INCLUDES} -c src/main.cc -o obj/main.o -lpcap
obj/sender.o: src/sender.cc src/core.cc src/utils.cc src/packetio.cc src/ip.cc include/ip.h
	${CC} ${CFLAGS} ${INCLUDES} -c src/sender.cc -o obj/sender.o -lpcap


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
obj/tcp.o : src/core.cc
	${CC} ${CFLAGS} ${INCLUDES} -c src/tcp.cc -o obj/tcp.o
obj/socket.o : src/socket.cc
	${CC} ${CFLAGS} ${INCLUDES} -c src/socket.cc -o obj/socket.o

