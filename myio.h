#define BUFF_SIZE 10
#include <stddef.h>

struct File * myopen(const char *pathname, int flags);
struct File
{
    int fd;
    char *readCP;
    char *writeCP;
    char readBuf[BUFF_SIZE];
    char writeBuf[BUFF_SIZE];
    int flags;
    int bytesLeft;
    int fileOffset;
};

int myclose(struct File *filePtr);
int myread(struct File *filePtr, char *buf, size_t count);
int mywrite(struct File *filePtr, char *buf, size_t count); 
void myflush(struct File *filePtr);
int myseek(struct File *filePtr, int offset, int whence); 


