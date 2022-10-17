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

#define BUFF_SIZE 20
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
    int bytesLeft;

};

int main(int argc, char *argv[])
{
    struct File *filePtr;
    struct File *filePtr2;
    char *userReadBuf;
    char *userWriteBuf;
    int results; 

    userReadBuf = malloc(30); 
    filePtr = myopen("testfile",O_RDWR);
    printf("fd is %d readBuf pointer value is %p and readCP pointer value is %p \n",filePtr->fd, filePtr->readBuf, filePtr->readCP);
    
    // results = myread(filePtr, userReadBuf, 2); // NEED TO TEST IT OUT WHEN YOU REQUEST MORE BYTES THAN THE FILE HAS 
    // printf("***this is whats in the userReadBuf at %p: %s***\n", userReadBuf, userReadBuf); 
    // printf("bytes read: %d\n\n", results); 

    
    // results = myread(filePtr, userReadBuf, 40); 
    // //USE TO TEST WHEN BUFF=50 FOR COUNT<BUFF & COUNT>BYTES IN FILE
    // //USE TO TEST WHEN BUFF=30, COUNT>BUFF & BUFF>BYTES IN FILE
    // printf("***this is whats in the userReadBuf at %p: %s***\n", userReadBuf, userReadBuf); 
    // printf("bytes read: %d\n\n", results); 

    //use following two to test one read and then another from readBuf when BUFF=30
    // results = myread(filePtr, userReadBuf, 5); //USE TO TEST WHEN BUFF=10, FOR COUNT<BUFF_SIZE<BYTES IN FILE
    // printf("***this is whats in the userReadBuf at %p: %s***\n", userReadBuf, userReadBuf); 
    // printf("bytes read: %d\n\n", results); 

    // results = myread(filePtr, userReadBuf+5, 5); //USE TO TEST WHEN BUFF=10, FOR COUNT<BUFF_SIZE<BYTES IN FILE
    // printf("***this is whats in the userReadBuf at %p: %s***\n", userReadBuf, userReadBuf); 
    // printf("bytes read: %d\n\n", results); 
    
    //use following two to check if you have want case where second call uses rest of readBuf then makes syscall straight to buf BUFF_SIZE =10, readUserBuf =30
    // results = myread(filePtr, userReadBuf, 7); 
    // printf("***this is whats in the userReadBuf at %p: %s***\n", userReadBuf, userReadBuf); 
    // printf("bytes read: %d\n\n", results); 
    // results = myread(filePtr, userReadBuf+7, 14); 
    // printf("***this is whats in the userReadBuf at %p: %s***\n", userReadBuf, userReadBuf); 
    // printf("bytes read: %d\n\n", results); 

    //use following two to check if you have want case where second call uses rest of readBuf then reads and memcpys per usual BUFF_SIZE =10, readUserBuf =30
    // results = myread(filePtr, userReadBuf, 7); 
    // printf("***this is whats in the userReadBuf at %p: %s***\n", userReadBuf, userReadBuf); 
    // printf("bytes read: %d\n\n", results); 
    // results = myread(filePtr, userReadBuf+7, 10); 
    // printf("***this is whats in the userReadBuf at %p: %s***\n", userReadBuf, userReadBuf); 
    // printf("bytes read: %d\n\n", results); 

    //use following 2 to check, normal, end of readBuf, then refill readBuf but count>bytesLeft, BUFF_SIZE=20
    // results = myread(filePtr, userReadBuf, 19); 
    // printf("***this is whats in the userReadBuf at %p: %s***\n", userReadBuf, userReadBuf); 
    // printf("bytes read: %d\n\n", results); 
    // results = myread(filePtr, userReadBuf+19, 6); 
    // printf("***this is whats in the userReadBuf at %p: %s***\n", userReadBuf, userReadBuf); 
    // printf("bytes read: %d\n\n", results);
    // results = myread(filePtr, userReadBuf+25, 1); 
    // printf("***this is whats in the userReadBuf at %p: %s***\n", userReadBuf, userReadBuf); 
    // printf("bytes read: %d\n\n", results);
    // results = myread(filePtr, userReadBuf+26, 2); 
    // printf("***this is whats in the userReadBuf at %p: %s***\n", userReadBuf, userReadBuf); 
    // printf("bytes read: %d\n\n", results);



    // results = myread(filePtr, userReadBuf, 30); // NEED TO TEST IT OUT WHEN YOU REQUEST MORE BYTES THAN THE FILE HAS 
    // printf("***this is whats in the userReadBuf at %p: %s***\n", userReadBuf, userReadBuf); 
    // printf("bytes read: %d\n\n", results); 

    //TESTS FOR MYWRITE FUNCTION
    userWriteBuf = "If we see this, we wrote correctly!";
    filePtr2 = myopen("writeTestFile",O_RDWR);
    results = mywrite(filePtr2, userWriteBuf, 5);
    printf("bytes written: %d\n", results); 
    results = mywrite(filePtr2, userWriteBuf+5, 7);
    printf("bytes written: %d\n", results); 
    results = mywrite(filePtr2, userWriteBuf+12, 10);
    printf("bytes written: %d\n", results); 

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
    filePtr->readCP = filePtr->readBuf;
    filePtr->bytesLeft = 0;

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
    
    int canRead, bytesRead, userBytesRead;

    userBytesRead = 0;
    
    //check read permissions
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

    //first case: when readBuf is empty
    if(filePtr->readBuf == filePtr->readCP)
    {
        //printf("WE REACHED THE First IF STATEMENT \n");

        //if our count is bigger than readBuf do a syscall right away to user buff (buf)
        if(count >= BUFF_SIZE)
        {
            bytesRead = read(filePtr->fd, buf, count);
            if(bytesRead == -1)
            {
                perror("read");
                exit(3);
            }
            userBytesRead = bytesRead;
            filePtr->readCP = filePtr->readBuf; //reset CP after each new read
            //printf("WE REACHED THE RIGHT IF STATEMENT \n");
        }
        //if count is smaller than readBuf, fill readBuf then give bytes to user
        else
        {
            bytesRead = read(filePtr->fd, filePtr->readBuf, BUFF_SIZE);
            if(bytesRead == -1)
            {
                perror("read");
                exit(3);
            }
            filePtr->bytesLeft = bytesRead; //bytes unread in readBuf = bytes just read into readBuf
            filePtr->readCP = filePtr->readBuf; //reset CP after each new read

            //if bytesRead is less than count, give user bytesRead
            if(bytesRead < count)
            {
                memcpy(buf, filePtr->readBuf, bytesRead);
                filePtr->readCP += bytesRead;
                filePtr->bytesLeft = 0;
                userBytesRead = bytesRead;
            }
            //if bytesRead is greater than/equal to count, give user count
            else
            {
                memcpy(buf, filePtr->readBuf, count);
                filePtr->readCP += count;
                filePtr->bytesLeft -= count;
                userBytesRead = count;
            }
        }
    }
    
    //second case: when readBuf isn't empty
    else
    {
        printf("COUNT = %ld, BYTESLEFT= %d, readBuf = %p, readCP = %p\n", count, filePtr->bytesLeft, filePtr->readBuf, filePtr->readCP);
        //normal case: when count is less than unread bytes in readBuf (bytesLeft) 
        if(count < filePtr->bytesLeft)
        {
            memcpy(buf, filePtr->readCP, count);
            filePtr->readCP += count;
            filePtr->bytesLeft -= count;
            userBytesRead = count;
        }
        //special case: when count is greater than or equal to the unread bytes in readBuf (bytesLeft)
        else
        {
            memcpy(buf, filePtr->readCP, filePtr->bytesLeft);
            count -= filePtr->bytesLeft;
            userBytesRead += filePtr->bytesLeft;

            //if count is still greater than BUFF_SIZE, syscall straight to buf
            if(count >= BUFF_SIZE)
            {
                // printf("entered the right case\n");
                bytesRead = read(filePtr->fd, buf+filePtr->bytesLeft, count);
                if(bytesRead == -1)
                {
                    perror("read");
                    exit(3);
                }
                userBytesRead += bytesRead;
                filePtr->readCP = filePtr->readBuf; //reset CP after each new read
            }
            //if count is less than BUFF_SIZE, read then read and memcpy (normal protocal)
            else
            {
                bytesRead = read(filePtr->fd, filePtr->readBuf, BUFF_SIZE);
                
                if(bytesRead == -1)
                {
                    perror("read");
                    exit(3);
                }
                filePtr->bytesLeft = bytesRead; //bytes unread in readBuf = bytes just read into readBuf
                filePtr->readCP = filePtr->readBuf; //reset CP after each new read

                //if bytesRead is less than count, give user bytesRead
                if(bytesRead < count)
                {
                    memcpy(buf+userBytesRead, filePtr->readCP, bytesRead);
                    filePtr->readCP += bytesRead;
                    filePtr->bytesLeft = 0;
                    userBytesRead += bytesRead;
                }
                //if bytesRead is greater than/equal to count, give user count
                else
                {
                    memcpy(buf+userBytesRead, filePtr->readCP, count);
                    filePtr->readCP += count;
                    filePtr->bytesLeft -= count;
                    userBytesRead += count;
                }
            }

        }

    }
    return userBytesRead;
    
}
/*
* Writes count bytes from buf into the fd 
*/
int mywrite(struct File *filePtr, char *buf, size_t count)
{
    int canWrite, bytesLeft, originalCount, bytesWritten;

    originalCount = count; 
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
            //what is memcpys error check?
            printf("***this is whats in the writeBuf at %p: %s***\n", filePtr->writeCP, filePtr->writeCP); 
            filePtr->writeCP += count;
            count = 0;
        }
        // Case when count is greater the size left in our writeBuf
        else
        {
            printf("\nRUNNING THE ELSE\n"); 
            memcpy(filePtr->writeCP, buf, bytesLeft);

            if(write(filePtr->fd, filePtr->writeBuf, BUFF_SIZE) == -1)
            {
                perror("write");
                exit(2);
            }
            filePtr->writeCP = filePtr->writeBuf;
            count -= bytesLeft; 
            memcpy(filePtr->writeCP, buf + bytesLeft, count); 
            count = 0;
        }
        return originalCount - count; 
    }
    // if count is bigger than writeBuf we do syscall automatically
    else
    {
        // write whats in writeBuf to file
        if(write(filePtr->fd, filePtr->writeBuf, BUFF_SIZE-bytesLeft) == -1)
        {
            perror("write");
            exit(3);
        }
        printf("this is how much we are writing: %d\n",BUFF_SIZE-bytesLeft);
        filePtr->writeCP = filePtr->writeBuf;

        //write count from buff to file
        if((bytesWritten = write(filePtr->fd, buf, count)) == -1)
        {
            perror("write");
            exit(3);
        }

        return bytesWritten; 
    }
    
}

/*
* Forces any buffered data to the destination fd 
*
* Note: Don't know if we need size 
*/ 
void myflush(int fd, void *buf, size_t size)
{

}