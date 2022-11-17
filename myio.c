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
    // Check if it is just write only 
    if((filePtr->flags & O_WRONLY) == 0) 
    {
        errno = EBADF;
        return -1;
    }

    //first case: when hiddenBuff is empty
    if(filePtr->hiddenBuf == filePtr->currPtr)
    {
        //if our count is bigger than hiddenBuff do a syscall right away to user buff (buf)
        if(count >= BUFF_SIZE)
        {
            bytesRead = read(filePtr->fd, buf, count);
            filePtr->fileOffset += count;

            if(bytesRead == -1)
            {
                return -1;
            }
            userBytesRead = bytesRead;
            //reset currPtr after each new read
            filePtr->currPtr = filePtr->hiddenBuf;
        }
        //if count is smaller than readBuf, fill readBuf then give bytes to user
        else
        {
            bytesRead = read(filePtr->fd, filePtr->hiddenBuf, BUFF_SIZE);

            if(bytesRead == -1)
            {
                return -1;
            }
            //bytes unread in readBuf = bytes just read into readBuf
            filePtr->bytesLeft = bytesRead;
            //reset currPtr after each new read
            filePtr->currPtr = filePtr->hiddenBuf;
            filePtr->haveRead = 1;

            //if bytesRead is less than count, give user bytesRead
            if(bytesRead < count)
            {
                memcpy(buf, filePtr->hiddenBuf, bytesRead);
                updateFilePtrFields(filePtr, bytesRead, bytesRead, -(filePtr->bytesLeft));
                userBytesRead = bytesRead;
            }
            //if bytesRead is greater than/equal to count, give user count
            else
            {
                memcpy(buf, filePtr->hiddenBuf, count);
                updateFilePtrFields(filePtr, count, count, -count);
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
            if(lseek(filePtr->fd, filePtr->fileOffset, SEEK_SET) == -1)
            {
                return -1;
            }
            //read bytesLeft amount from fileOffset to hiddenBuf
            if(read(filePtr->fd, filePtr->currPtr, filePtr->bytesLeft) == -1)
            {
                return -1;
            }
            filePtr->haveRead = 1;
        }
       //normal case: when count is less than unread bytes in readBuf (bytesLeft) 
        if(count < filePtr->bytesLeft)
        {
            memcpy(buf, filePtr->currPtr, count);
            updateFilePtrFields(filePtr, count, count, -(count));
            userBytesRead = count;
        }
        //special case: when count is greater than or equal to the unread bytes in readBuf (bytesLeft)
        else
        {
            memcpy(buf, filePtr->currPtr, filePtr->bytesLeft);
            updateFilePtrFields(filePtr, filePtr->bytesLeft, filePtr->bytesLeft, 0);
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

                if(bytesRead == -1)
                {
                    return -1;
                }

                filePtr->fileOffset += count;
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

                //read from offset
                bytesRead = read(filePtr->fd, filePtr->hiddenBuf, BUFF_SIZE);

                if(bytesRead == -1)
                {
                    return -1;
                }

                //bytes unread in readBuf = bytes just read into readBuf
                filePtr->bytesLeft = bytesRead;
                //reset currPtr after each new read
                filePtr->currPtr = filePtr->hiddenBuf;
                filePtr->haveRead = 1;

                //if bytesRead is less than count, give user bytesRead
                if(bytesRead < count)
                {
                    memcpy(buf+userBytesRead, filePtr->currPtr, bytesRead);
                    updateFilePtrFields(filePtr, bytesRead, bytesRead, -(filePtr->bytesLeft));
                    userBytesRead += bytesRead;
                }
                //if bytesRead is greater than/equal to count, give user count
                else
                {
                    memcpy(buf+userBytesRead, filePtr->currPtr, count);
                    updateFilePtrFields(filePtr, count, count, -(count));
                    userBytesRead += count;
                }
            }
        }
    }
    
    return userBytesRead;
}

/*
* Update fields in the struct filePtr
*/
void updateFilePtrFields(struct File *filePtr, int incrementOffset, int incrementCurrPtr, int incrementBytesLeft)
{
    filePtr->fileOffset += incrementOffset;
    filePtr->currPtr += incrementCurrPtr;
    filePtr->bytesLeft += incrementBytesLeft;
}

/*
* Writes count bytes from buf into the fd 
*/
int mywrite(struct File *filePtr, char *buf, size_t count)
{
    int originalCount, bytesWritten;

    originalCount = count;

    filePtr->bytesLeft = BUFF_SIZE - (filePtr->currPtr - filePtr->hiddenBuf); // need to initialize bytesLeft

    if(!(((filePtr->flags & O_WRONLY) == 0) || ((filePtr->flags & O_RDWR) == 0)))
    {
        errno = EBADF;
        return -1;
    }

    // if count is smaller than hiddenBuf we write to hiddenBuf until full
    if(count < BUFF_SIZE)
    {    
        // Case when count is less than size left in our hiddenBuf
        if(count <= filePtr->bytesLeft)
        { 
            memcpy(filePtr->currPtr, buf, count);
            filePtr->fileOffset += count;
            filePtr->currPtr += count;
            filePtr->haveWritten = 1;
            filePtr->bytesLeft -= count;
            count = 0;
        }
        // Case when count is greater the size left in our hiddenBuf
        else
        {
            memcpy(filePtr->currPtr, buf, filePtr->bytesLeft);
            filePtr->fileOffset += filePtr->bytesLeft;
            filePtr->currPtr += filePtr->bytesLeft;
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
                    return -1;
                }
            }
            //if count is now smaller than BUFF_SIZE, memcopy to buf
            else
            {
                memcpy(filePtr->currPtr, buf + filePtr->bytesLeft, count);
                filePtr->fileOffset += count;
                filePtr->currPtr += count;
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
            return -1;
        }

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

    //move kernel offset backwards if we have written and read to the buffer
    if(filePtr->haveWritten == 1 && filePtr->haveRead == 1)
    {
        if(lseek(filePtr->fd, -(filePtr->currPtr - filePtr->hiddenBuf), SEEK_CUR) == -1)
        {
            return -1;
        }
    }

    //this writes to the file and moves kernel offset back forwards
    bytesWritten = write(filePtr->fd, filePtr->hiddenBuf, filePtr->currPtr - filePtr->hiddenBuf);

    if(bytesWritten == -1)
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
        if((filePtr->currPtr + offset > filePtr->hiddenBuf) && (filePtr->currPtr + offset < (filePtr->currPtr +filePtr->bytesLeft)))
        {
            filePtr->currPtr += offset;
            filePtr->bytesLeft -= offset;
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

        if((filePtr->currPtr + moveOffset > filePtr->hiddenBuf) && (filePtr->currPtr + moveOffset < (filePtr->currPtr + filePtr->bytesLeft)))
        {  
            filePtr->currPtr += moveOffset;
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