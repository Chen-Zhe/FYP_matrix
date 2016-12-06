/*
author: Chen Zhe
Customized libsocket libary for remote compilation to R-pi through VS
Removed unix stream

added conf.h file based on declaration in libinetsocket.c
edited libinetsocket.c to include conf.h
changed libinetsocket.c to cpp
*/


#define LIBSOCKET_VERSION 2.4
#define LIBSOCKET_LINUX 1
#define LIBSOCKET_FREEBSD 0
#define LIBSOCKET_SUNOS 0