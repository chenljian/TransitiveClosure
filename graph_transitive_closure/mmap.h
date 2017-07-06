#ifndef _MMAP_H
#define _MMAP_H

//#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
//#include <unistd.h>
#include <string.h>
#include <windows.h>
#define handle_error(msg) do { perror(msg); exit(EXIT_FAILURE); } while (0)

extern void *open_mmap(char *file_name);
extern void close_mmap(void *addr, char *file_name);

#endif
