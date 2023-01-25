# This Makefile will build Moonbase Console using MinGW
# Simply typing "make" in the same directory as this file should build it

OBJS=Common/mbcmif.o MoonbaseConsole/Base64Encode.o MoonbaseConsole/crc.o MoonbaseConsole/KattonGen.o MoonbaseConsole/SpiffGen.o MoonbaseConsole/StdAfx.o MoonbaseConsole/MoonbaseConsole.o MoonbaseConsole/resource.o

UNAME := $(shell uname)

ifeq ($(UNAME), Linux)
	INCLUDE_DIRS = -I./MoonbaseConsole -I./Common
	CC = i686-w64-mingw32-g++
else
	INCLUDE_DIRS = -I.\MoonbaseConsole -I.\Common
	CC = g++
endif

WARNS = -Wall
CFLAGS = -O2 -D _WIN32_IE=0x0500 -D WINVER=0x500 ${WARNS}
LDFLAGS = -lcomctl32 -lws2_32 -lwininet -static -mwindows
#RC = windres
RC = i686-w64-mingw32-windres

all : MoonbaseConsole.exe

MoonbaseConsole.exe : ${OBJS}
	${CC} -o "$@" ${OBJS} ${LDFLAGS}

clean :
	rm *.exe MoonbaseConsole/*.o Common/*.o *.ini

%.o : %.cpp
	${CC} ${CFLAGS} ${INCLUDE_DIRS} -c $< -o $@

MoonbaseConsole/resource.o : MoonbaseConsole/MoonbaseConsole.rc
	${RC} -i $< -o $@
