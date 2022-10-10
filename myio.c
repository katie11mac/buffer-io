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
    int results; 

    buf = malloc(30); 
    filePtr = myopen("testfile",O_RDWR);
    printf("fd is %d buf pointer value is %p and cp pointer value is %p \n",filePtr->fd, filePtr->fileBuf, filePtr->cp);
    // results = myread(filePtr, buf, 2); // NEED TO TEST IT OUT WHEN YOU REQUEST MORE BYTES THAN THE FILE HAS 
    // printf("***this is whats in the buf at %p: %s***\n", buf, buf); 
    // printf("bytes read: %d\n\n", results); 
    results = myread(filePtr, buf, 30); // NEED TO TEST IT OUT WHEN YOU REQUEST MORE BYTES THAN THE FILE HAS 
    printf("***this is whats in the buf at %p: %s***\n", buf, buf); 
    printf("bytes read: %d\n\n", results); 
    results = myread(filePtr, buf, 30); // NEED TO TEST IT OUT WHEN YOU REQUEST MORE BYTES THAN THE FILE HAS 
    printf("***this is whats in the buf at %p: %s***\n", buf, buf); 
    printf("bytes read: %d\n\n", results); 

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
*
* NEED TO MAKE SURE WE DON'T READ MORE BYTES THAN WHAT THE FILE CONTAINS 
*/
int myread(struct File *filePtr, char *buf, size_t count)
{ 
    int flagRead, bytesRead;
    size_t bytesLeft, originalCount;
    originalCount = count; 

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

    do 
    {
        /*CASE 1: File buf is empty*/
        if(filePtr->cp == NULL)
        {
            printf("\nFILLING IN FILE BUF\n"); 
            bytesRead = read(filePtr->fd, filePtr->fileBuf, BUFF_SIZE); 
            printf("\tRead %d bytes\n", bytesRead); 
            printf("\tFile buf value: \'%s\'\n", filePtr->fileBuf); 
            filePtr->cp = filePtr->fileBuf;
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

        bytesLeft = BUFF_SIZE - ((filePtr->cp) - (filePtr->fileBuf)); 

        /*CASE 2: User requests more bytes than what is left in the File buf*/
        if(bytesLeft < count)
        {
            printf("\nUSER REQUESTED MORE BYTES THAN WHAT IS IN FILE BUF\n"); 
            if(memcpy(buf, filePtr->cp, bytesLeft) == NULL)
            {
                printf("MEMCPY NULL (1)\n"); 
                return -1;
            }
            printf("\tCurrent User Buf Value (from %p): %s\n", buf, buf); 
            count = count - bytesLeft;
            buf += bytesLeft; /*change where we are writing to in the user's buf*/
            printf("\tupdated buf: %p \tupdated count: %ld\n", buf, count); 
            printf("\tREFILL FILE BUFF\n"); 
            bytesRead = read(filePtr->fd, filePtr->fileBuf, BUFF_SIZE);
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
                //return 0; return original count - count 
            }
            filePtr->cp = filePtr->fileBuf; /*need to reset the cp pointer*/
            printf("\t \n"); 
        }
        

        /*CASE 3: Can provide the bytes that are left in the File buf*/
        if (count <= bytesLeft) 
        {
            printf("\nPROVIDE USER WITH CONTENTS THAT ARE IN FILE BUF (MEMCPY-ING)\n"); 
            
            if(bytesRead < BUFF_SIZE) // means you have read up to the end of the file because you weren't able to fill the entire buff
            {
                printf("\tbytesRead < BUFF_SIZE\n"); 
                if(memcpy(buf, filePtr->cp, bytesRead) == NULL)
                {
                    printf("MEMCPY NULL (2)\n");
                    return -1;
                }
                printf("\tCurrent User Buf Value (from %p): %s\n", buf, buf);
                filePtr->cp += bytesRead;
                count -= bytesRead;
                bytesRead -= bytesRead; 
            } 
            else 
            {
                if(memcpy(buf, filePtr->cp, count) == NULL)
                {
                    printf("MEMCPY NULL (2)\n");
                    return -1;
                }
                printf("\tCurrent User Buf Value (from %p): %s\n", buf, buf);
                filePtr->cp += count;
                count -= count;
                bytesRead -= count; 
            }

            printf("\ncount = %ld\n", count);
            printf("\n"); 
        }
        
    } 
    while(count > 0 && bytesRead == BUFF_SIZE); 


    if (count == 0)
    {
        
        return originalCount; 
    } 
    else // weren't able to reach requested amount, so try to mem cpy one last time with the number of bytes read 
    {
        printf("QUICK MEMCPY\n"); 
        printf("\tbytesRead: %d\n", bytesRead); 
        if(memcpy(buf, filePtr->cp, bytesRead) == NULL)
        {
            {
                printf("MEMCPY NULL (2)\n");
                return -1;
            }
        }
        printf("\tCurrent User Buf Value (from %p): %s\n", buf, buf);
        filePtr->cp += bytesRead; //do we need to change something here
        count -= bytesRead;
        return (originalCount - count); 
    }
    
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