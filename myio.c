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
    results = myread(filePtr, buf, 15); // NEED TO TEST IT OUT WHEN YOU REQUEST MORE BYTES THAN THE FILE HAS 
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
    //filePtr->cp = filePtr->fileBuf; 
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

    /*bytesLeft = BUFF_SIZE - ((filePtr->cp) - (filePtr->fileBuf));*/
    // COMMENTED OUT CODE ABOVE BC filePtr->cp is initially null

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
        /*CASE 1: File buf is empty, 
        changed the check because when it refills the cp and bp will be in the same place*/
        if(filePtr->cp == NULL)
        {
            printf("\nFILLING IN FILE BUF\n"); 
            bytesRead = read(filePtr->fd, filePtr->fileBuf, BUFF_SIZE); 
            /*original value of third argument: count, would still store that amount of bytes in the File buf even though the size was smaller*/
            printf("\tRead %d bytes\n", bytesRead); 
            printf("\tFile buf value: \'%s\'\n", filePtr->fileBuf); 
            filePtr->cp = filePtr->fileBuf;
            //bytesLeft = BUFF_SIZE - ((filePtr->cp) - (filePtr->fileBuf));
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
        // MOVED THIS HERE BC filePtr->cp WOULD NO LONGER BE NULL 

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
        

        /*CASE 3: Can provide the bytes that are left in the File buf*/
        if (count <= bytesLeft) 
        /*check this because what if count is greater than buff size*/
        {
            printf("PROVIDE USER WITH CONTENTS THAT ARE IN FILE BUF (MEMCPY-ING)\n"); 
            if(memcpy(buf, filePtr->cp, count) == NULL)
            {
                printf("MEMCPY NULL (2)\n"); 
                return -1;
            }
            printf("\tCurrent User Buf Value (from %p): %s\n", buf, buf); 
            filePtr->cp += count;
            count -= count; 

            // NO LONGER NEED THE FOLLOW CODE BECAUSE THIS WHOLE SECTION WOULD RUN UNDER A CONDITIONAL
            // if(count <= BUFF_SIZE)
            // {
            //     printf("\tCOUNT <= BUFF_SIZE\n"); 
            //     filePtr->cp += count;
            //     count -= count; 
            // }
            // else
            // {
            //     printf("\tADDED BUFF SIZE\n"); 
            //     filePtr->cp += BUFF_SIZE;
            //     count -= BUFF_SIZE; 
            // }

            /*THE CONDITIONAL WILL STILL RUN WHEN COUNT < 0, maybe bc its size_t*/
            /*Can't have count be a negative number because it is seen as an unsigned int*/
            /*count -= BUFF_SIZE;*/
            printf("\ncount = %ld\n", count);
            printf("\n"); 
        }
        
    }

    if (count == 0)
    {
        return originalCount; 
    } 
    else 
    {
        return originalCount - (originalCount - count); 
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