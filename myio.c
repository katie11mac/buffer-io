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
*/
int myread(struct File *filePtr, char *buf, size_t count)
{ 
    
    int canRead, bytesRead, userBytesRead, canWrite;
    
    //have a flag if you can write and only flush when you can write

    canWrite = 0;
    userBytesRead = 0;
    
    //check read permissions
    canRead = 0;

    
    if(((filePtr->flags & O_WRONLY) != 0) || ((filePtr->flags & O_RDWR) != 0))
    {
        canWrite = 1;
    }

    if(((filePtr->flags & O_RDONLY) != 0) || ((filePtr->flags & O_RDWR) != 0))
    {
        canRead = 1;
    }

    if(canRead == 0)
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
                // if(canWrite == 1)
                // {
                //     // lseek(filePtr->fd, -(filePtr->CP-filePtr->hiddenBuf), SEEK_CUR);
                //     // myflush(filePtr);
                // }
                
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
                // if(canWrite == 1)
                // {
                //     myflush(filePtr); //write what we've written/read into buff moves kernel's offset forward
                // }

                bytesRead = read(filePtr->fd, filePtr->hiddenBuf, BUFF_SIZE); //read from offset

                // if(canWrite == 1)
                // {
                //     lseek(filePtr->fd,-BUFF_SIZE, SEEK_CUR); //move offset backwards for future writes
                // }
                
                if(bytesRead == -1)
                {
                    perror("read");
                    exit(3);
                }
                filePtr->bytesLeft = bytesRead; //bytes unread in readBuf = bytes just read into readBuf
                filePtr->CP = filePtr->hiddenBuf; //reset CP after each new read

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
    int canWrite, originalCount, bytesWritten, canRead;

    originalCount = count; 
    canWrite = 0;
    canRead = 0;
    filePtr->bytesLeft = BUFF_SIZE - (filePtr->CP - filePtr->hiddenBuf); // need to initialize bytesLeft
    printf("bytes left: %d\n", filePtr->bytesLeft); 

    if(((filePtr->flags & O_WRONLY) != 0) || ((filePtr->flags & O_RDWR) != 0))
    {
        canWrite = 1;
    }
    if(((filePtr->flags & O_RDONLY) != 0) || ((filePtr->flags & O_RDWR) != 0))
    {
        canRead = 1;
    }

    if(canWrite == 0)
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
            count = 0;
        }
        // Case when count is greater the size left in our hiddenBuf
        else
        {
            printf("this is what is runnning\n");
            memcpy(filePtr->CP, buf, filePtr->bytesLeft);
            filePtr->fileOffset += filePtr->bytesLeft;
            filePtr->CP += filePtr->bytesLeft;

            // if(canRead == 1)
            // {
            //     lseek(filePtr->fd,-BUFF_SIZE, SEEK_CUR);
            // }

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

    //lseek(filePtr->fd, filePtr->fileOffset, SEEK_SET);

    bytesWritten = write(filePtr->fd, filePtr->hiddenBuf, filePtr->CP - filePtr->hiddenBuf);
    //printf("the size of our buff = %ld\n",filePtr->CP - filePtr->hiddenBuf); 

    if(bytesWritten == -1)
    {
        perror("write");
        exit(2);
    } 
    filePtr->CP = filePtr->hiddenBuf;
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
            myflush(filePtr);
            lseek(filePtr->fd, offset, whence); 
        }
        filePtr->fileOffset += offset;
    }
    else if(whence == SEEK_SET)
    {

        moveOffset = offset - (lseek(filePtr->fd, 0, SEEK_CUR)-filePtr->bytesLeft);
        printf("moveOffset: %d\n", moveOffset); 

        if((filePtr->CP + moveOffset > filePtr->hiddenBuf) && (filePtr->CP + moveOffset < (filePtr->CP +filePtr->bytesLeft)))
        {
            printf("WITHIN BOUNDS\n");            
            filePtr->CP += moveOffset;
            printf("bytesLeft= %d\n",filePtr->bytesLeft);
            filePtr->bytesLeft -= moveOffset;
        }
        else
        {
            myflush(filePtr);
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