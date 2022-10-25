/*
 * myio.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "myio.h"

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
    filePtr->CP = filePtr->hiddenBuf;
    filePtr->bytesLeft = 0;
    filePtr->fileOffset = 0;
    filePtr->haveWritten = 0;
    filePtr->haveRead = 0;

    return filePtr; 
}

/*
* Closes the file descriptor
*/
int myclose(struct File *filePtr)
{

    int results; 
    
    if(filePtr->haveWritten == 1)
    {
        myflush(filePtr); 
    }

    if ((results = close(filePtr->fd)) == -1)
    {
        perror("close"); 
    } 

    return results; 
}

/*
* Reads count bytes from fd into the buf and returns the amount of bytes read
*/
int myread(struct File *filePtr, char *buf, size_t count)
{ 
    
    int bytesRead, userBytesRead;
    
    userBytesRead = 0;

    if(!(((filePtr->flags & O_RDONLY) != 0) || ((filePtr->flags & O_RDWR) != 0)))
    {
        return 0;
    }

    //first case: when hiddenBuff is empty
    if(filePtr->hiddenBuf == filePtr->CP)
    {
        //if our count is bigger than hiddenBuff do a syscall right away to user buff (buf)
        if(count >= BUFF_SIZE)
        {
            bytesRead = read(filePtr->fd, buf, count);
            filePtr->fileOffset += count;

            if(bytesRead == -1)
            {
                perror("read");
                exit(3);
            }
            userBytesRead = bytesRead;
            filePtr->CP = filePtr->hiddenBuf; //reset CP after each new read
        }
        //if count is smaller than readBuf, fill readBuf then give bytes to user
        else
        {
            bytesRead = read(filePtr->fd, filePtr->hiddenBuf, BUFF_SIZE);

            if(bytesRead == -1)
            {
                perror("read");
                exit(3);
            }
            filePtr->bytesLeft = bytesRead; //bytes unread in readBuf = bytes just read into readBuf
            filePtr->CP = filePtr->hiddenBuf; //reset CP after each new read
            filePtr->haveRead = 1;

            //if bytesRead is less than count, give user bytesRead
            if(bytesRead < count)
            {
                memcpy(buf, filePtr->hiddenBuf, bytesRead);
                filePtr->fileOffset += bytesRead;
                filePtr->CP += bytesRead;
                filePtr->bytesLeft = 0;
                userBytesRead = bytesRead;
            }
            //if bytesRead is greater than/equal to count, give user count
            else
            {
                memcpy(buf, filePtr->hiddenBuf, count);
                filePtr->fileOffset += count;
                filePtr->CP += count;
                filePtr->bytesLeft -= count;
                userBytesRead = count;
            }
        }
    }
    
    
    //second case: when readBuf isn't empty
    else
    {
        if(filePtr->haveRead == 0) 
        {
            //move forward the amount we've read
            lseek(filePtr->fd, filePtr->fileOffset, SEEK_SET);
            //read bytesLeft amount from fileOffset to hiddenBuf
            printf("bytesLeft after write = %d\n",filePtr->bytesLeft);
            read(filePtr->fd, filePtr->CP, filePtr->bytesLeft);
            filePtr->haveRead = 1;
            printf("this is whats in our hiddenBuf = %s\n",filePtr->hiddenBuf);
        }
        printf("RUNNING THE ELSE\n"); 
       //normal case: when count is less than unread bytes in readBuf (bytesLeft) 
        if(count < filePtr->bytesLeft)
        {
            memcpy(buf, filePtr->CP, count);
            filePtr->fileOffset += count;
            filePtr->CP += count;
            filePtr->bytesLeft -= count;
            userBytesRead = count;
        }
        //special case: when count is greater than or equal to the unread bytes in readBuf (bytesLeft)
        else
        {
            memcpy(buf, filePtr->CP, filePtr->bytesLeft);
            filePtr->fileOffset += filePtr->bytesLeft;
            filePtr->CP += filePtr->bytesLeft;
            count -= filePtr->bytesLeft;
            userBytesRead += filePtr->bytesLeft;

            //if count is still greater than BUFF_SIZE, syscall straight to buf
            if(count >= BUFF_SIZE)
            {
                //if we have written already need to flush!
                if(filePtr->haveWritten == 1)
                {
                    myflush(filePtr);
                }
                bytesRead = read(filePtr->fd, buf+filePtr->bytesLeft, count);
                
                filePtr->fileOffset += count;

                if(bytesRead == -1)
                {
                    perror("read");
                    exit(3);
                }
                userBytesRead += bytesRead;
                filePtr->CP = filePtr->hiddenBuf; //reset CP after each new read
            }
            //if count is less than BUFF_SIZE, read then read and memcpy (normal protocal)
            else
            {
                //if we have written already need to flush!
                if(filePtr->haveWritten == 1)
                {
                    myflush(filePtr);
                }
                bytesRead = read(filePtr->fd, filePtr->hiddenBuf, BUFF_SIZE); //read from offset

                if(bytesRead == -1)
                {
                    perror("read");
                    exit(3);
                }
                filePtr->bytesLeft = bytesRead; //bytes unread in readBuf = bytes just read into readBuf
                filePtr->CP = filePtr->hiddenBuf; //reset CP after each new read
                filePtr->haveRead = 1;

                //if bytesRead is less than count, give user bytesRead
                if(bytesRead < count)
                {
                    memcpy(buf+userBytesRead, filePtr->CP, bytesRead);
                    filePtr->fileOffset += bytesRead;
                    filePtr->CP += bytesRead;
                    filePtr->bytesLeft = 0;
                    userBytesRead += bytesRead;
                }
                //if bytesRead is greater than/equal to count, give user count
                else
                {
                    memcpy(buf+userBytesRead, filePtr->CP, count);
                    filePtr->fileOffset += count;
                    filePtr->CP += count;
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
    int originalCount, bytesWritten;

    originalCount = count; 

    filePtr->bytesLeft = BUFF_SIZE - (filePtr->CP - filePtr->hiddenBuf); // need to initialize bytesLeft
    printf("bytes left: %d\n", filePtr->bytesLeft); 

    if(!(((filePtr->flags & O_WRONLY) == 0) || ((filePtr->flags & O_RDWR) == 0)))
    {
        return 0;
    }

    // if count is smaller than hiddenBuf we write to hiddenBuf until full
    if(count < BUFF_SIZE)
    {    
        // Case when count is less than size left in our hiddenBuf
        if(count <= filePtr->bytesLeft)
        { 
            memcpy(filePtr->CP, buf, count);
            filePtr->fileOffset += count;
            filePtr->CP += count;
            filePtr->haveWritten = 1;
            filePtr->bytesLeft -= count;
            count = 0;
        }
        // Case when count is greater the size left in our hiddenBuf
        else
        {
            printf("this is what is runnning\n");
            memcpy(filePtr->CP, buf, filePtr->bytesLeft);
            filePtr->fileOffset += filePtr->bytesLeft;
            filePtr->CP += filePtr->bytesLeft;
            filePtr->haveWritten = 1;

            myflush(filePtr);

            count -= filePtr->bytesLeft; 
            //if count is still greater than or equal to BUFF_SIZE, syscall straight to file
            if (count >= BUFF_SIZE)
            {
                bytesWritten = write(filePtr->fd, buf + filePtr->bytesLeft, count);
                filePtr->fileOffset += count;

                if(bytesWritten == -1)
                {
                    perror("write");
                    exit(3);
                }
            }
            //if count is now smaller than BUFF_SIZE, memcopy to buf
            else
            {
                memcpy(filePtr->CP, buf + filePtr->bytesLeft, count);
                filePtr->fileOffset += count;
                filePtr->CP += count;
                filePtr->haveWritten = 1;
                filePtr->bytesLeft -= count;
            }
            

            count = 0;
        }
        return originalCount - count; 
    }
    // if count is bigger than hiddenBuf we do syscall automatically
    else
    {
        myflush(filePtr);
        bytesWritten = write(filePtr->fd, buf, count);
        filePtr->fileOffset += count;

        if(bytesWritten == -1)
        {
            perror("write");
            exit(3);
        }

        return bytesWritten; 
    }
    
}

/*
* Forces any buffered data to the destination fd 
*/ 
void myflush(struct File *filePtr)
{
    int bytesWritten;
    // printf("Kernel Offset when flushing: %ld\n", lseek(filePtr->fd, 0, SEEK_CUR)); 
    // printf("Our Offset when flushing: %d\n", filePtr->fileOffset); 

    if(filePtr->haveWritten == 1 && filePtr->haveRead == 1)
    {
        printf("we are moving backwards = %ld\n",-(filePtr->CP - filePtr->hiddenBuf));
        lseek(filePtr->fd, -(filePtr->CP - filePtr->hiddenBuf), SEEK_CUR);
    }

    // printf("Kernel Offset when flushing: %ld\n", lseek(filePtr->fd, 0, SEEK_CUR)); 
    // printf("Our Offset when flushing: %d\n", filePtr->fileOffset); 

    bytesWritten = write(filePtr->fd, filePtr->hiddenBuf, filePtr->CP - filePtr->hiddenBuf);
    //printf("the size of our buff = %ld\n",filePtr->CP - filePtr->hiddenBuf); 

    if(bytesWritten == -1)
    {
        perror("write");
        exit(2);
    } 
    filePtr->CP = filePtr->hiddenBuf;
    filePtr->haveWritten = 0;
    filePtr->haveRead = 0;
}

int myseek(struct File *filePtr, int offset, int whence)
{
   int moveOffset;
   
   moveOffset = 0;
   
   //check whence
    if(whence == SEEK_CUR)
    {
        //check bounds for hiddeneBuf
        if((filePtr->CP + offset > filePtr->hiddenBuf) && (filePtr->CP + offset < (filePtr->CP +filePtr->bytesLeft)))
        {
            filePtr->CP += offset;
            filePtr->bytesLeft -= offset;
        }
        else
        {
            printf("SEEK_CUR out of bounds\n");
            if(filePtr->haveWritten == 1)
            {
                myflush(filePtr);
            }
            else
            {
                filePtr->CP = filePtr->hiddenBuf;
            }
            lseek(filePtr->fd, filePtr->fileOffset+offset, SEEK_SET); 
        }
        filePtr->fileOffset += offset;
    }
    else if(whence == SEEK_SET)
    {
        // determine amount offset moves our kernel offset
        moveOffset = offset - filePtr->fileOffset;
        printf("moveOffset: %d\n", moveOffset); 

        if((filePtr->CP + moveOffset > filePtr->hiddenBuf) && (filePtr->CP + moveOffset < (filePtr->CP + filePtr->bytesLeft)))
        {
            printf("WITHIN BOUNDS\n");            
            filePtr->CP += moveOffset;
            printf("bytesLeft= %d\n",filePtr->bytesLeft);
            filePtr->bytesLeft -= moveOffset;
        }
        else
        {
            if(filePtr->haveWritten == 1)
            {
                myflush(filePtr);
            }
            else
            {
                filePtr->CP = filePtr->hiddenBuf;
            }
            lseek(filePtr->fd, offset, whence); 
        }
        filePtr->fileOffset = offset;
    }
    else
    {
        return -1;
    }

    return filePtr->fileOffset;
}