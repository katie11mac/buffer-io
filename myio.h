#ifndef __MYIO_H
#define __MYIO_H

#define BUFF_SIZE 10
#include <stddef.h>

struct File * myopen(const char *pathname, int flags);
struct File
{
    int fd;
    char *currPtr;
    char hiddenBuf[BUFF_SIZE];
    int flags;
    int bytesLeft;
    int fileOffset;
    int haveWritten; //to hiddenBuf
    int haveRead; //to hiddenBuf
};

int myclose(struct File *filePtr);
int myread(struct File *filePtr, char *buf, size_t count);
int mywrite(struct File *filePtr, char *buf, size_t count); 
int myflush(struct File *filePtr);
int myseek(struct File *filePtr, int offset, int whence); 

#endif /* __MYIO_H */
