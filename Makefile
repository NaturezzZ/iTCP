# compile macro
CC		= g++
CFLAGS	= -Wall
LDFLAGS	= -lm
INCLUDES= -I./include

# user modify: target and objects
EXENAME = test
OBJECTS	= obj/packetio.o obj/core.o obj/device.o

# make target
all:${OBJECTS}
	${CC} -o ${EXENAME} ${OBJECTS} ${LDFLAGS} ${INCLUDES}
	
# make clean
clean:
	rm -rf ${EXENAME} ${OBJECTS} 

# dependence	
obj/packetio.o : src/packetio.cc
	${CC} ${CFLAGS} ${INCLUDES} -c src/packetio.cc -o obj/packetio.o
obj/device.o : src/device.cc src/core.cc
	${CC} ${CFLAGS} ${INCLUDES} -c src/device.cc -o obj/device.o
obj/core.o : src/core.cc
	${CC} ${CFLAGS} ${INCLUDES} -c src/core.cc -o obj/core.o

