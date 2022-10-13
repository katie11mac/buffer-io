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

#define BUFF_SIZE 10
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
    struct File *filePtr2;
    char *readBuf;
    char *writeBuf;
    int results; 

    readBuf = malloc(30); 
    filePtr = myopen("testfile",O_RDWR);
    printf("fd is %d readBuf pointer value is %p and readCP pointer value is %p \n",filePtr->fd, filePtr->readBuf, filePtr->readCP);
    
    // results = myread(filePtr, readBuf, 2); // NEED TO TEST IT OUT WHEN YOU REQUEST MORE BYTES THAN THE FILE HAS 
    // printf("***this is whats in the readBuf at %p: %s***\n", readBuf, readBuf); 
    // printf("bytes read: %d\n\n", results); 

    // results = myread(filePtr, readBuf, 30); // NEED TO TEST IT OUT WHEN YOU REQUEST MORE BYTES THAN THE FILE HAS 
    // printf("***this is whats in the readBuf at %p: %s***\n", readBuf, readBuf); 
    // printf("bytes read: %d\n\n", results); 
    
    // results = myread(filePtr, readBuf, 30); // NEED TO TEST IT OUT WHEN YOU REQUEST MORE BYTES THAN THE FILE HAS 
    // printf("***this is whats in the readBuf at %p: %s***\n", readBuf, readBuf); 
    // printf("bytes read: %d\n\n", results); 

    writeBuf = "If we see this, we wrote correctly!";
    filePtr2 = myopen("writeTestFile",O_RDWR);
    mywrite(filePtr2, writeBuf, 5);
    mywrite(filePtr2, writeBuf+5, 7);
    mywrite(filePtr2, writeBuf+12, 10);

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
    int canRead; 

    canRead = 0;

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

    
}
/*
* Writes count bytes from buf into the fd 
*/
int mywrite(struct File *filePtr, void *buf, size_t count)
{
    int canWrite, bytesLeft;

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

    bytesLeft = BUFF_SIZE - (filePtr->writeCP - filePtr->writeBuf);

    // if count is smaller than writeBuf we write to writeBuf until full
    if(count < BUFF_SIZE)
    {    
        printf("bytesLeft: %d\n", bytesLeft); 
        // Case when count is less than size left in our writeBuf
        if(count <= bytesLeft)
        {
            printf("RUNNING THE IF\n"); 
            memcpy(filePtr->writeCP, buf, count);
            printf("***this is whats in the writeBuf at %p: %s***\n", filePtr->writeCP, filePtr->writeCP); 
            filePtr->writeCP += count;
            count = 0;
        }
        // Case when count is greater the size left in our writeBuf
        else
        {
            printf("RUNNING THE ELSE\n"); 
            memcpy(filePtr->writeCP, buf, bytesLeft);
            write(filePtr->fd, filePtr->writeBuf, BUFF_SIZE);
            filePtr->writeCP = filePtr->writeBuf;
            count -= bytesLeft;
        }
    }
    // if count is bigger than writeBuf we do syscall automatically
    else
    {
        // write whats in writeBuf to file
        write(filePtr->fd, filePtr->writeBuf, BUFF_SIZE-bytesLeft);
        printf("this is how much we are writing: %d\n",BUFF_SIZE-bytesLeft);
        filePtr->writeCP = filePtr->writeBuf;

        //write count from buff to file
        write(filePtr->fd, buf, count);
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