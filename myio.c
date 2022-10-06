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
/*
 buf_size and struct file should be in the header
*/
struct File
{
    int fd;
    char *cp;
    char fileBuf[BUFF_SIZE];
    int flags;

};

int main(int argc, char *argv[])
{
    struct File *filePtr;
    char * buf;

    buf = malloc(10);
    filePtr = myopen("testfile",O_RDWR);
    printf("fd is %d buf pointer value is %p and cp pointer value is %p \n",filePtr->fd, filePtr->fileBuf, filePtr->cp);
    myread(filePtr, buf, 10);

    printf("this is whats in the buf: %s\n", buf); 

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
        return NULL;
    }

    filePtr = malloc(sizeof(struct File));
    
    if(filePtr == NULL)
    {
        return NULL;
    }

    filePtr->fd = fd;
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
int myread(struct File *filePtr, char *buf, size_t count) 
{ 
    int flagRead, bytesRead;
    size_t bytesLeft;

    bytesLeft = BUFF_SIZE - ((filePtr->cp) - (filePtr->fileBuf));

    flagRead = 0;

    if(((filePtr->flags & O_RDONLY) != 0) || ((filePtr->flags & O_RDWR) != 0))
    {
        printf("CAN READ\n"); 
        flagRead = 1;
    }

    if(flagRead == 0)
    {
        printf("CANNOT READ\n"); 
        return 0;
    }

    while(count > 0)
    {
        if(filePtr->fileBuf == filePtr->cp)
        {
            printf("SYSCALL READ\n"); 
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

        if(bytesLeft < count)
        {
            if(memcpy(buf, filePtr->cp, bytesLeft) == NULL)
            {
                printf("MEMCPY NULL (1)\n"); 
                return -1;
            }
            count = count - bytesLeft;
            buf += bytesLeft;

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
            filePtr->cp = filePtr->fileBuf;
        }
        
        printf("MEMCPY-ING\n"); 

        if(memcpy(buf, filePtr->cp, count) == NULL)
        {
            printf("MEMCPY NULL (2)\n"); 
            return -1;
        }
        if(count < BUFF_SIZE)
        {
            printf("COUNT < BUFF_SIZE\n"); 
            filePtr->cp += count;
        }
        else
        {
            printf("ADDED BUFF SIZE"); 
            filePtr->cp += BUFF_SIZE;
        }

        printf("count= %ld\n", count);
        count -= BUFF_SIZE;
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