/*
 * myio.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "myio.h"

/*
* Opens a file located at the pathname and returns the file struct pointer 
*/
struct File * myopen(const char *pathname, int flags)
{
    int fd;
    struct File *filePtr;

    fd = open(pathname, flags, 0666);

    if(fd == -1)
    {
        return NULL;
    }

    filePtr = malloc(sizeof(struct File));
    
    if(filePtr == NULL)
    {
        return NULL;
    }

    filePtr->fd = fd;
    filePtr->flags = flags;
    filePtr->currPtr = filePtr->hiddenBuf;
    filePtr->bytesLeftToRead = 0;
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
        free(filePtr);
        return -1;
    } 

    free(filePtr);

    return results;
}

/*
* Reads count bytes from fd into the buf and returns the amount of bytes read
*/
int myread(struct File *filePtr, char *buf, size_t count)
{ 
    
    int bytesRead, userBytesRead;
    
    userBytesRead = 0;

    // neither read write nor read only 
    // not read write and not write only check that theyre turned off
    // Check if it is just write only (does this come with assumptions tho?)
    if((filePtr->flags & O_WRONLY) != 0) 
    {
        errno = EBADF;
        printf("NOT ALLOWED TO READ\n");
        return -1;
    }

    //first case: when hiddenBuff is empty
    if(filePtr->hiddenBuf == filePtr->currPtr)
    {
        //if our count is bigger than hiddenBuff do a syscall right away to user buff (buf)
        if(count >= BUFF_SIZE)
        {

            //if we have written already need to flush! (originally not here)
            // if(filePtr->haveWritten == 1)
            // {
            //     myflush(filePtr);
            // }

            if((bytesRead = read(filePtr->fd, buf, count)) == -1)
            {
                return -1;
            }
            filePtr->fileOffset += bytesRead; // originally += count, but shouldn't it be += bytesRead in case can't read all of count?
            userBytesRead += bytesRead;
            //reset currPtr after each new read
            filePtr->currPtr = filePtr->hiddenBuf;
        }
        //if count is smaller than readBuf, fill readBuf then give bytes to user
        else
        {

            //if we have written already need to flush! (originally not here)
            // if(filePtr->haveWritten == 1)
            // {
            //     myflush(filePtr);
            // }

            if((bytesRead = read(filePtr->fd, filePtr->hiddenBuf, BUFF_SIZE)) == -1)
            {
                return -1;
            }
            //bytes unread in readBuf = bytes just read into readBuf
            filePtr->bytesLeftToRead = bytesRead;
            //reset currPtr after each new read
            filePtr->currPtr = filePtr->hiddenBuf;
            filePtr->haveRead = 1;

            userBytesRead += myReadMemcpy(buf, filePtr->hiddenBuf, filePtr, bytesRead, count);
        }
    }
    //second case: when readBuf isn't empty
    else
    {
        //if did not previously read before
        if(filePtr->haveRead == 0)
        {
            //move forward the amount we've written to our buffer
            if(lseek(filePtr->fd, filePtr->fileOffset, SEEK_SET) == -1)
            {
                printf("we are lseeking incorrectly\n");
                return -1;
            }
            //read bytesLeftToRead amount from fileOffset to hiddenBuf
            if((filePtr->bytesLeftToRead = read(filePtr->fd, filePtr->currPtr, BUFF_SIZE - (filePtr->currPtr - filePtr->hiddenBuf))) == -1)
            {
                return -1;
            }
            printf("bytes Left to read = %d\n", filePtr->bytesLeftToRead);
            filePtr->haveRead = 1;
        }

        //normal case: when count is less than unread bytes in readBuf (bytesLeftToRead) 
        if(count < filePtr->bytesLeftToRead)
        {
            memcpy(buf, filePtr->currPtr, count);
            updateFilePtrFields(filePtr, count, count, -(count));
            userBytesRead = count;
            filePtr->haveRead = 1; // why is haveRead 1 here? if we are just reading from the hiddenBuf and not to it
        }
        //special case: when count is greater than or equal to the unread bytes in readBuf (bytesLeftToRead)
        else
        {
            printf("SPECIAL CASE!!! \n");

            //first, read the rest of what is in the hiddenBuf to the user
            memcpy(buf, filePtr->currPtr, filePtr->bytesLeftToRead);
            userBytesRead += filePtr->bytesLeftToRead;
            count -= filePtr->bytesLeftToRead;
            updateFilePtrFields(filePtr, filePtr->bytesLeftToRead, filePtr->bytesLeftToRead, -(filePtr->bytesLeftToRead)); // should third parameter be 0 or be -( -> bytesLeftToRead)
            filePtr->haveRead = 1; // why is haveRead 1 here? if we are just reading from the hiddenBuf and not to it

            //if count is still greater than BUFF_SIZE, syscall straight to buf
            if(count >= BUFF_SIZE)
            {
                //if we have written already need to flush!
                if(filePtr->haveWritten == 1)
                {
                    myflush(filePtr);
                }

                if((bytesRead = read(filePtr->fd, buf + userBytesRead, count)) == -1)
                {
                    return -1;
                }

                filePtr->fileOffset += bytesRead; // originally += count, but what if it doesn't read all of count
                userBytesRead += bytesRead;
                //reset currPtr after each new read
                filePtr->currPtr = filePtr->hiddenBuf;
            }
            //if count is less than BUFF_SIZE, read then read and memcpy (normal protocal)
            else
            {
                //if we have written already need to flush!
                if(filePtr->haveWritten == 1)
                {
                    myflush(filePtr);
                }

                //read from offset (refill hiddenBuf)
                if((bytesRead = read(filePtr->fd, filePtr->hiddenBuf, BUFF_SIZE)) == -1)
                {
                    return -1;
                }

                //bytes unread in readBuf = bytes just read into readBuf
                filePtr->bytesLeftToRead = bytesRead;
                //reset currPtr after each new read
                filePtr->currPtr = filePtr->hiddenBuf;
                filePtr->haveRead = 1;

                userBytesRead += myReadMemcpy(buf+userBytesRead, filePtr->currPtr, filePtr, bytesRead, count);
            }
        }
    }
    
    return userBytesRead;
}


/*
*
*/
int myReadMemcpy(char *buf, void *src, struct File *filePtr, int bytesRead, int count)
{
    //if bytesRead is less than count, give user bytesRead
    if(bytesRead < count)
    {
        memcpy(buf, src, bytesRead);
        updateFilePtrFields(filePtr, bytesRead, bytesRead, -(filePtr->bytesLeftToRead));
        return bytesRead;
    }
    //if bytesRead is greater than/equal to count, give user count
    else
    {
        memcpy(buf, src, count);
        updateFilePtrFields(filePtr, count, count, -count);
        return count;
    }
}

/*
* Update fields in the struct filePtr
*/
void updateFilePtrFields(struct File *filePtr, int incrementOffset, int incrementCurrPtr, int incrementbytesLeftToRead)
{
    filePtr->fileOffset += incrementOffset;
    filePtr->currPtr += incrementCurrPtr;
    filePtr->bytesLeftToRead += incrementbytesLeftToRead;
}

/*
* Writes count bytes from buf into the fd 
*/
int mywrite(struct File *filePtr, char *buf, size_t count)
{
    int originalCount, bytesWritten;

    originalCount = count;

    filePtr->bytesLeftToWrite = BUFF_SIZE - (filePtr->currPtr - filePtr->hiddenBuf);
    printf("bytesLeftToWrite = %d\n", filePtr->bytesLeftToWrite);

    // Check writing permissions
    if(!(((filePtr->flags & O_WRONLY) == 0) || ((filePtr->flags & O_RDWR) == 0)))
    {
        errno = EBADF;
        return -1;
    }

    // if count is smaller than hiddenBuf we write to hiddenBuf until full
    if(count < BUFF_SIZE)
    {   
        // Case when count is less than size left in our hiddenBuf
        if(count <= filePtr->bytesLeftToWrite)
        {
            // write count bytes to the hiddenBuf
            memcpy(filePtr->currPtr, buf, count);
            filePtr->fileOffset += count;
            filePtr->currPtr += count;
            filePtr->haveWritten = 1;
            filePtr->bytesLeftToWrite -= count;
            filePtr->bytesLeftToRead -= count;
            count = 0;
        }
        // Case when count is greater the size left in our hiddenBuf
        else
        {
            // fill in the remaining of the hiddenBuf
            memcpy(filePtr->currPtr, buf, filePtr->bytesLeftToWrite);
            filePtr->fileOffset += filePtr->bytesLeftToWrite;
            filePtr->currPtr += filePtr->bytesLeftToWrite;
            filePtr->haveWritten = 1;

            myflush(filePtr); //why are we flushing here?

            count -= filePtr->bytesLeftToWrite;
            //if count is still greater than or equal to BUFF_SIZE, syscall straight to file
            if (count >= BUFF_SIZE)
            {
                if((bytesWritten = write(filePtr->fd, buf + filePtr->bytesLeftToWrite, count)) == -1)
                {
                    return -1;
                }
                filePtr->bytesLeftToWrite -= filePtr->bytesLeftToWrite; //added this line
                filePtr->bytesLeftToRead -= filePtr->bytesLeftToWrite; //added this line
                filePtr->fileOffset += count;
            }
            //if count is now smaller than BUFF_SIZE, memcopy to buf
            else
            {
                memcpy(filePtr->currPtr, buf + filePtr->bytesLeftToWrite, count);
                filePtr->fileOffset += count;
                filePtr->currPtr += count;
                filePtr->haveWritten = 1;
                filePtr->bytesLeftToWrite -= count;
                filePtr->bytesLeftToRead -= count;
            }
            count = 0;
        }
        return originalCount - count;
    }
    // if count is bigger than hiddenBuf we do syscall automatically
    else
    {
        myflush(filePtr);
    
        if((bytesWritten = write(filePtr->fd, buf, count)) == -1)
        {
            return -1;
        }

        filePtr->fileOffset += count;

        return bytesWritten;
    }
    
}

/*
* Forces any buffered data to the destination fd 
* ------ only flush if have written to buff (haveWritten == 1)
*/ 
int myflush(struct File *filePtr)
{
    int bytesWritten;

    printf("haveWritten: %d haveRead: %d\n", filePtr->haveWritten, filePtr->haveRead);
    //move kernel offset backwards if we have written and read to the buffer
    if(filePtr->haveWritten == 1 && filePtr->haveRead == 1)
    {
        printf("kernels offset = %ld moving backward =  %ld\n", lseek(filePtr->fd, 0, SEEK_CUR), (filePtr->currPtr - filePtr->hiddenBuf) + filePtr->bytesLeftToRead);
        printf("filePtr->currPtr - filePtr->hiddenBuf: %ld\n", filePtr->currPtr - filePtr->hiddenBuf);
        printf("filePtr->bytesLeftToRead: %d\n", filePtr->bytesLeftToRead);
        if(lseek(filePtr->fd, -((filePtr->currPtr - filePtr->hiddenBuf) + filePtr->bytesLeftToRead), SEEK_CUR) == -1)
        {
            return -1;
        }
    }

    printf("kernel's file offset = %ld\n", lseek(filePtr->fd, 0, SEEK_CUR));
    //this writes to the file and moves kernel offset back forwards
    

    if((bytesWritten = write(filePtr->fd, filePtr->hiddenBuf, filePtr->currPtr - filePtr->hiddenBuf)) == -1)
    {
        return -1;
    }

    filePtr->currPtr = filePtr->hiddenBuf;
    filePtr->haveWritten = 0;
    filePtr->haveRead = 0;

    return 0;
}

/*
* Changes where we are in our file accordingly- moves currPtr and fileOffset or kernel offset and fileOffset
*/
int myseek(struct File *filePtr, int offset, int whence)
{
   int moveOffset;
   
   moveOffset = 0;
   
    //check whence
    if(whence == SEEK_CUR)
    {
        //check bounds for hiddeneBuf
        if((filePtr->currPtr + offset > filePtr->hiddenBuf) && (filePtr->currPtr + offset < (filePtr->currPtr + filePtr->bytesLeftToRead)))
        {
            filePtr->currPtr += offset;
            filePtr->bytesLeftToRead -= offset;
        }
        //out of bounds for hiddenbuf
        else
        {
            //if we've written to the hiddenBuf, flush before moving kernel offset
            if(filePtr->haveWritten == 1)
            {
                myflush(filePtr);
            }
            else
            {
                //reset currPtr to the beginning of the hiddenBuf
                filePtr->currPtr = filePtr->hiddenBuf;
            }
            if(lseek(filePtr->fd, filePtr->fileOffset+offset, SEEK_SET) == -1)
            {
                return -1;
            }
        }
        filePtr->fileOffset += offset;
    }
    else if(whence == SEEK_SET)
    {
        // determine amount offset moves our kernel offset
        moveOffset = offset - filePtr->fileOffset;

        if((filePtr->currPtr + moveOffset > filePtr->hiddenBuf) && (filePtr->currPtr + moveOffset < (filePtr->currPtr + filePtr->bytesLeftToRead)))
        {  
            filePtr->currPtr += moveOffset;
            filePtr->bytesLeftToRead -= moveOffset;
        }
        else
        {
            if(filePtr->haveWritten == 1)
            {
                myflush(filePtr);
            }
            else
            {
                filePtr->currPtr = filePtr->hiddenBuf;
            }
            if(lseek(filePtr->fd, offset, whence) == -1)
            {
                return -1;
            }
        }
        filePtr->fileOffset = offset;
    }
    else
    {
        return -1;
    }

    return filePtr->fileOffset;
}