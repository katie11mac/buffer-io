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
    char *readCP;
    char *writeCP;
    char readBuf[BUFF_SIZE];
    char writeBuf[BUFF_SIZE];
    int flags;

};

int main(int argc, char *argv[])
{
    struct File *filePtr;
    char * buf;
    int results; 

    buf = malloc(30); 
    filePtr = myopen("testfile",O_RDWR);
    printf("fd is %d buf pointer value is %p and readCP pointer value is %p \n",filePtr->fd, filePtr->readBuf, filePtr->readCP);
    
    results = myread(filePtr, buf, 2); // NEED TO TEST IT OUT WHEN YOU REQUEST MORE BYTES THAN THE FILE HAS 
    printf("***this is whats in the buf at %p: %s***\n", buf, buf); 
    printf("bytes read: %d\n\n", results); 

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
    filePtr->writeCP = filePtr->writeBuf;

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
    int canRead, bytesRead;
    size_t bytesLeft, originalCount;
    originalCount = count; 

    canRead = 0;

    printf("\ninital buf: %p \tinitial count: %ld\n", buf, count);

    if(((filePtr->flags & O_RDONLY) != 0) || ((filePtr->flags & O_RDWR) != 0))
    {
        printf("CAN READ\n"); 
        canRead = 1;
    }

    if(canRead == 0)
    {
        printf("CANNOT READ\n"); 
        return 0;
    }

    do 
    {
        /*CASE 1: File buf is empty*/
        if(filePtr->readCP == NULL)
        {
            printf("\nFILLING IN FILE BUF\n"); 
            bytesRead = read(filePtr->fd, filePtr->readBuf, BUFF_SIZE); 
            printf("\tRead %d bytes\n", bytesRead); 
            printf("\tFile buf value: \'%s\'\n", filePtr->readBuf); 
            filePtr->readCP = filePtr->readBuf;
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

        if(bytesRead < BUFF_SIZE)
        {
            bytesLeft = bytesRead - ((filePtr->readCP) - (filePtr->readBuf)); 
        }
        else
        {
            bytesLeft = BUFF_SIZE - ((filePtr->readCP) - (filePtr->readBuf));
        }
         
        /*CASE 2: User requests more bytes than what is left in the File buf*/
        if(bytesLeft < count)
        {
            printf("\nUSER REQUESTED MORE BYTES THAN WHAT IS IN FILE BUF\n"); 
            if(memcpy(buf, filePtr->readCP, bytesLeft) == NULL)
            {
                printf("MEMCPY NULL (1)\n"); 
                return -1;
            }
            printf("\tCurrent User Buf Value (from %p): %s\n", buf, buf); 
            count -= bytesLeft;
            buf += bytesLeft; /*change where we are writing to in the user's buf*/
            printf("\tupdated buf: %p \tupdated count: %ld\n", buf, count); 
            printf("\tREFILL FILE BUFF\n"); 
            bytesRead = read(filePtr->fd, filePtr->readBuf, BUFF_SIZE);
            printf("\tRead %d bytes\n", bytesRead); 
            printf("\tFile buf value: \'%s\'\n", filePtr->readBuf); 
            if(bytesRead == -1)
            {
                perror("read");
                return -1;
            }
            else if(bytesRead == 0)
            {
                printf("End of file\n");
                // bytesLeft = 0; 
                bytesLeft = bytesRead - ((filePtr->readCP) - (filePtr->readBuf)); 
                //return 0; return original count - count 
            }
            filePtr->readCP = filePtr->readBuf; /*need to reset the readCP pointer*/
            printf("\t \n"); 
        }
        

        /*CASE 3: Can provide the bytes that are left in the File buf*/
        if (count <= bytesLeft) 
        {
            printf("\nPROVIDE USER WITH CONTENTS THAT ARE IN FILE BUF (MEMCPY-ING)\n"); 
            printf("count = %ld\n",count);
            printf("bytesLeft = %ld\n",bytesLeft);
            printf("bytesRead = %d\n",bytesRead);
            
            // if(bytesRead < BUFF_SIZE) // means you have read up to the end of the file because you weren't able to fill the entire buff
            // {
            //     printf("\tbytesRead < BUFF_SIZE\n"); 
            //     if(memcpy(buf, filePtr->readCP, bytesRead) == NULL)
            //     {
            //         printf("MEMCPY NULL (2)\n");
            //         return -1;
            //     }
            //     printf("\tCurrent User Buf Value (from %p): %s\n", buf, buf);
            //     filePtr->readCP += bytesRead;
            //     count -= bytesRead;
            //     bytesRead -= bytesRead; 
            // } 
            // else 
            // {
                if(memcpy(buf, filePtr->readCP, count) == NULL)
                {
                    printf("MEMCPY NULL (2)\n");
                    return -1;
                }
                printf("\tCurrent User Buf Value (from %p): %s\n", buf, buf);
                filePtr->readCP += count;
                count -= count;
                bytesRead -= count; 
            // }

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
        // printf("QUICK MEMCPY\n"); 
        // printf("\tbytesRead: %d\n", bytesRead); 
        // if(memcpy(buf, filePtr->readCP, bytesRead) == NULL)
        // {
        //     {
        //         printf("MEMCPY NULL (2)\n");
        //         return -1;
        //     }
        // }
        // printf("\tCurrent User Buf Value (from %p): %s\n", buf, buf);
        // filePtr->readCP += bytesRead; //do we need to change something here
        // count -= bytesRead;
        return (originalCount - count); 
    }
    
}
/*
* Writes count bytes from buf into the fd 
*/
int mywrite(struct File *filePtr, const void *buf, size_t count)
{
    int canWrite;


    canWrite = 0;

    if(((filePtr->flags & O_WRONLY) != 0) || ((filePtr->flags & O_RDWR) != 0))
    {
        printf("CAN WRITE\n"); 
        canWrite = 1;
    }

    if(canWrite == 0)
    {
        printf("CANNOT WRITE\n"); 
        return 0;
    }
    
    while(count > 0)
    {
    





    }
    
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