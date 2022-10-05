/*
 * myio.c
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "myio.h"

#include <fcntl.h> 

#define BUFF_SIZE 5

struct File
{
    int fd;
    void *fileBuf, *cp;
    int flags;

};

int main(int argc, char *argv[])
{
    struct File *filePtr;

    filePtr = myopen("/home/lschweitzer/CS315/assignment2/testfile",O_RDWR);
    printf("fd is %d buf pointer value is %p and cp pointer value is %p \n",filePtr->fd, filePtr->fileBuf, filePtr->cp);

    return 0; 
}

/*
* Opens a file located at the pathname and returns the file descriptor. 
*/
struct File * myopen(const char *pathname, int flags) 
{
    int fd;
    struct File *filePtr;

    fd = open(pathname, flags, 0666);

    if(fd == -1)
    {
        perror("open");
    }

    filePtr = malloc(sizeof(struct File));
    filePtr->fd = fd;
    filePtr->fileBuf = malloc(BUFF_SIZE);
    filePtr->cp = filePtr->fileBuf;
    filePtr->flags = flags;

    return filePtr; 
}

/*
* Closes the file descriptor
*/
int myclose(int fd) 
{
    return 0; 
}

/*
* Reads count bytes from fd into the buf and returns the amount of bytes read
*/
int myread(struct File *filePtr, void *buf, size_t count) 
{ 
    int flagRead, bytesRead;

    flagRead = 0;

    if(((filePtr->flags & O_RDONLY) != 0) | ((filePtr->flags & O_RDWR) != 0))
    {
        flagRead = 1;
    }

    if(flagRead == 0)
    {
        return 0;
    }

    if(filePtr->fileBuf == filePtr->cp)
    {
        bytesRead = read(filePtr->fd, filePtr->fileBuf, count);
        if(bytesRead == -1)
        {
            perror("read");
            return -1;
        }
        else if(bytesRead == 0)
        {
            printf("End of file\n");
            return 0;
        }
    }
    else if((BUFF_SIZE - filePtr->cp) < count){
        /*
        memcopy give the rest and read again
        */
    }
    return 0; 
}

/*
* Writes count bytes from buf into the fd 
*/
int mywrite(int fd, const void *buf, size_t count)
{
    return 0; 
}

/*
* Forces any buffered data to the destination fd 
*
* Note: Don't know if we need size 
*/ 
void myflush(int fd, void *buf, size_t size)
{

}