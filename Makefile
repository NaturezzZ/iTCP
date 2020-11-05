# compile macro
CC		= g++
CFLAGS	= -w
LDFLAGS	= -lm
INCLUDES= -I./include

# user modify: target and objects
EXENAME = test.out
OBJECTS	= obj/utils.o obj/packetio.o obj/core.o obj/device.o obj/main.o 

# make target
all:${OBJECTS}
	${CC} -o ${EXENAME} ${OBJECTS} ${LDFLAGS} ${INCLUDES} -lpcap
run:${OBJECTS}
	${CC} -o ${EXENAME} ${OBJECTS} ${LDFLAGS} ${INCLUDES} -lpcap
	./${EXENAME}
# make clean
clean:
	rm -rf ${EXENAME} ${OBJECTS} 

# dependence	
obj/main.o: src/main.cc src/core.cc
	${CC} ${CFLAGS} ${INCLUDES} -c src/main.cc -o obj/main.o -lpcap
obj/packetio.o : src/packetio.cc src/core.cc
	${CC} ${CFLAGS} ${INCLUDES} -c src/packetio.cc -o obj/packetio.o -lpcap
obj/device.o : src/device.cc src/core.cc
	${CC} ${CFLAGS} ${INCLUDES} -c src/device.cc -o obj/device.o -lpcap
obj/core.o : src/core.cc
	${CC} ${CFLAGS} ${INCLUDES} -c src/core.cc -o obj/core.o -lpcap
obj/utils.o : src/utils.cc
	${CC} ${CFLAGS} ${INCLUDES} -c src/utils.cc -o obj/utils.o -lpcap

