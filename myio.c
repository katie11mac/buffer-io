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
    filePtr->fileOffset = 0;

    return filePtr; 
}

/*
* Closes the file descriptor
*/
int myclose(struct File *filePtr)
{

    int results; 

    myflush(filePtr); 

    if ((results = close(filePtr->fd)) == -1)
    {
        perror("close"); 
    } 

    return results; 
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
        //printf("CAN READ\n"); 
        canRead = 1;
    }

    if(canRead == 0)
    {
        //printf("CANNOT READ\n"); 
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

            filePtr->fileOffset += bytesRead;

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
            
            filePtr->fileOffset += bytesRead;

            if(bytesRead == -1)
            {
                perror("read");
                exit(3);
            }
            filePtr->bytesLeft = bytesRead; //bytes unread in readBuf = bytes just read into readBuf

            //printf("bytesLeft = %d\n", filePtr->bytesLeft);

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
        //printf("bytesLeft now = %d\n", filePtr->bytesLeft);
    }
    
    
    //second case: when readBuf isn't empty
    else
    {
        //printf("COUNT = %ld, BYTESLEFT= %d, readBuf = %p, readCP = %p\n", count, filePtr->bytesLeft, filePtr->readBuf, filePtr->readCP);
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

                filePtr->fileOffset += bytesRead;

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

                filePtr->fileOffset += bytesRead;
                
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
    //printf("bytesLeft now = %d\n", filePtr->bytesLeft);
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
        canWrite = 1;
    }

    if(canWrite == 0)
    {
        return 0;
    }

    bytesLeft = BUFF_SIZE - (filePtr->writeCP - filePtr->writeBuf);

    // if count is smaller than writeBuf we write to writeBuf until full
    if(count < BUFF_SIZE)
    {    
        // Case when count is less than size left in our writeBuf
        if(count <= bytesLeft)
        {
            // printf("RUNNING THE IF\n"); 
            memcpy(filePtr->writeCP, buf, count);
            //what is memcpys error check?
            // printf("***this is whats in the writeBuf at %p: %s***\n", filePtr->writeCP, filePtr->writeCP); 
            filePtr->writeCP += count;
            count = 0;
        }
        // Case when count is greater the size left in our writeBuf
        else
        {
            // printf("\nRUNNING THE ELSE\n"); 
            memcpy(filePtr->writeCP, buf, bytesLeft);

            bytesWritten = write(filePtr->fd, filePtr->writeBuf, BUFF_SIZE);
            if(bytesWritten == -1)
            {
                perror("write");
                exit(2);
            }

            filePtr->fileOffset += bytesWritten;

            filePtr->writeCP = filePtr->writeBuf;
            count -= bytesLeft; 
            memcpy(filePtr->writeCP, buf + bytesLeft, count);
            filePtr->writeCP += count;  
            count = 0;
        }
        return originalCount - count; 
    }
    // if count is bigger than writeBuf we do syscall automatically
    else
    {
        // write whats in writeBuf to file
        bytesWritten = write(filePtr->fd, filePtr->writeBuf, BUFF_SIZE-bytesLeft);
        
        if(bytesWritten == -1)
        {
            perror("write");
            exit(3);
        }

        filePtr->fileOffset += bytesWritten;

        // printf("this is how much we are writing: %d\n",BUFF_SIZE-bytesLeft);
        filePtr->writeCP = filePtr->writeBuf;

        //write count from buff to file
        bytesWritten = write(filePtr->fd, buf, count);
        if(bytesWritten == -1)
        {
            perror("write");
            exit(3);
        }

        filePtr->fileOffset += bytesWritten;

        return bytesWritten; 
    }
    
}

/*
* Forces any buffered data to the destination fd 
*
*/ 
void myflush(struct File *filePtr)
{
    int bytesWritten;

    //printf("CP - writeBuf: %ld\n", filePtr->writeCP - filePtr->writeBuf); 

    bytesWritten = write(filePtr->fd, filePtr->writeBuf, filePtr->writeCP - filePtr->writeBuf);
    
    if(bytesWritten == -1)
    {
        perror("write");
        exit(2);
    } 
    filePtr->fileOffset += bytesWritten;
}

int myseek(struct File *filePtr, int offset, int whence)
{
    //check whence
    if(whence == SEEK_CUR)
    {
        //printf("SEEK_CUR\n"); 
        //printf("bytesLeft now = %d\n", filePtr->bytesLeft);

        //check bounds for writeBuf
        if((filePtr->writeCP + offset > filePtr->writeBuf) && (filePtr->writeCP + offset < (filePtr->writeBuf + BUFF_SIZE)))
        {
            filePtr->writeCP += offset;
        }
        else
        {
            myflush(filePtr);
            filePtr->fileOffset += offset; 
            lseek(filePtr->fd, offset, whence); 
            filePtr->writeCP = filePtr->writeBuf; 
        }

        //check bounds for readBuf 
        if((filePtr->readCP + offset > filePtr->readBuf) && ((filePtr->readCP + offset) < (filePtr->readCP + filePtr->bytesLeft)))
        {
            printf("filePtr->readCP = %p and filePtr->bytesLeft=%d\n",filePtr->readCP, filePtr->bytesLeft);
            filePtr->readCP += offset;
            filePtr->bytesLeft -= offset;
            printf("filePtr->readCP = %p and filePtr->bytesLeft=%d\n",filePtr->readCP, filePtr->bytesLeft);
        }
        else 
        {
            filePtr->fileOffset += offset; 
            lseek(filePtr->fd, offset, whence); 
            read(filePtr->fd, filePtr->readBuf, BUFF_SIZE);
            filePtr->readCP = filePtr->readBuf; 
        }

    }
    else if(whence == SEEK_SET)
    {
        printf("SEEK_SET\n"); 
    }
    else
    {
        return -1;
    }

    return filePtr->fileOffset;
}