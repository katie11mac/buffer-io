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

    buf = malloc(10); /*TRY IT WITH A LARGER NUMBER THAT MAKES IT GO THROUGH THE WHILE LOOP (ie. 15)*/
    filePtr = myopen("testfile",O_RDWR);
    printf("fd is %d buf pointer value is %p and cp pointer value is %p \n",filePtr->fd, filePtr->fileBuf, filePtr->cp);
    myread(filePtr, buf, 10); /*if change malloc size remember to change third argument as well*/

    printf("***this is whats in the buf: %s***\n", buf); 

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

    printf("\ninital buf: %p \tinitial count: %ld\n", buf, count);

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
        /*CASE 1: File buf is empty*/
        if(filePtr->fileBuf == filePtr->cp)
        {
            printf("\nFILLING IN FILE BUF\n"); 
            bytesRead = read(filePtr->fd, filePtr->fileBuf, BUFF_SIZE); 
            /*original value of third argument: count, would still store that amount of bytes in the File buf even though the size was smaller*/
            printf("\tRead %d bytes\n", bytesRead); 
            printf("\tFile buf value: \'%s\'\n", filePtr->fileBuf); 
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

        /*CASE 2: User requests more bytes than what is left in the File buf*/
        if(bytesLeft < count)
        {
            printf("\nUSER REQUESTED MORE BYTES THAN WHAT IS IN FILE BUF\n"); 
            if(memcpy(buf, filePtr->cp, bytesLeft) == NULL)
            {
                printf("MEMCPY NULL (1)\n"); 
                return -1;
            }
            printf("\tCurrent User Buf Value: %s\n", buf); 
            count = count - bytesLeft;
            buf += bytesLeft; /*change where we are writing to in the user's buf*/
            printf("\tupdated buf: %p \tupdated count: %ld\n", buf, count); 

            printf("\tREFILL FILE BUFF\n"); 
            bytesRead = read(filePtr->fd, filePtr->fileBuf, BUFF_SIZE);
            /*original value of third argument: count, would still store that amount of bytes in the File buf even though the size was smaller*/
            printf("\tRead %d bytes\n", bytesRead); 
            printf("\tFile buf value: \'%s\'\n", filePtr->fileBuf); 
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
            filePtr->cp = filePtr->fileBuf; /*need to reset the cp pointer*/
            printf("\t \n"); 
        }
        
        printf("PROVIDE USER WITH CONTENTS THAT ARE IN FILE BUF (MEMCPY-ING)\n"); 
        /*CASE 3: Can provide the bytes that are left in the File buf*/
        /*NEED TO LOOK OVER THIS PART, SHOULD IT BE AN ELSE IF TO THE bytesLeft < count CONDITION?*/
        if(memcpy(buf, filePtr->cp, count) == NULL)
        {
            printf("MEMCPY NULL (2)\n"); 
            return -1;
        }
        printf("%s\n", buf); 
        if(count < BUFF_SIZE)
        {
            printf("COUNT < BUFF_SIZE\n"); 
            filePtr->cp += count;
        }
        else
        {
            printf("ADDED BUFF SIZE\n"); 
            filePtr->cp += BUFF_SIZE;
        }

        printf("count= %ld\n", count);
        count -= BUFF_SIZE;
        printf("\n"); 
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