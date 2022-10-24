#include "myio.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void testMyRead(); 
void testMyWrite();
void testWriteRead(); 
void testReadWrite(); 
void testMySeek(); 

int main(int argc, char *argv[])
{
    testMyRead(); //file offset lines up
    testMyWrite(); 
    testWriteRead(); 
    // testReadWrite();
    //testMySeek(); 
   
    return 0; 
}

/*
    Tests for myread() in myio.c

    ASSUMPTIONS: BUFF_SIZE = 10, userReadBuf = 30, 
    and testfile has 27 bytes
*/
void testMyRead()
{
    struct File *readFilePtr;
    char *userReadBuf;
    int results; 
    int total; 

    total = 0; 

    printf("\n-----TESTING myread()-----\n"); 
    userReadBuf = malloc(30); 

    // Open the testfile
    readFilePtr = myopen("testfile", O_RDWR); // testfile has 27 characters 
    
    // TEST 1: Request count smaller than buff size and smaller than file size, 
    // so count < BUFF_SIZE < total bytes in file 
    printf("TEST 1\n"); 
    results = myread(readFilePtr, userReadBuf, 7);
    total += results; 
    printf("\trequested: 7, expected: 7\n\tbytes read: %d\n", results); 
    printf("\tour fileOffset = %d and kernels fileOffset = %ld\n",readFilePtr->fileOffset, lseek(readFilePtr->fd,0,SEEK_CUR));
    printf("\tvalue of userReadBuf starting at %p: \'%s\'\n\n", userReadBuf, userReadBuf); 

    // TEST 2: Request count that uses the rest of readBuf and then makes syscall straight to buf 
    // count > BUFF_SIZE and count > bytesLeft and count < total bytes in file 
    printf("TEST 2\n"); 
    results = myread(readFilePtr, userReadBuf + total, 14); 
    total += results; 
    printf("\trequested: 14, expected: 14\n\tbytes read: %d\n", results); 
    printf("\tour fileOffset = %d and kernels fileOffset = %ld\n",readFilePtr->fileOffset, lseek(readFilePtr->fd,0,SEEK_CUR));
    printf("\tvalue of userReadBuf starting at %p: \'%s\'\n\n", userReadBuf, userReadBuf); 

    // 
    results = myread(readFilePtr, userReadBuf + total, 4); 
    total += results; 
    printf("\trequested: 4, expected: 4\n\tbytes read: %d\n", results); 
    printf("\tour fileOffset = %d and kernels fileOffset = %ld\n",readFilePtr->fileOffset, lseek(readFilePtr->fd,0,SEEK_CUR));
    printf("\tvalue of userReadBuf starting at %p: \'%s\'\n\n", userReadBuf, userReadBuf); 

    // TEST 3: Request amount smaller than bytesLeft when close to the end of the file, 
    // so count < bytesLeft 
    printf("TEST 3\n"); 
    results = myread(readFilePtr, userReadBuf + total, 1); 
    total += results; 
    printf("\trequested: 1, expected: 1\n\tbytes read: %d\n", results); 
    printf("\tour fileOffset = %d and kernels fileOffset = %ld\n",readFilePtr->fileOffset, lseek(readFilePtr->fd,0,SEEK_CUR));
    printf("\tvalue of userReadBuf starting at %p: \'%s\'\n\n", userReadBuf, userReadBuf); 

    // TEST 4: Request amount larger than bytesLeft when there should only be one byte left in the file, 
    // so count > bytesLeft
    printf("TEST 4\n"); 
    results = myread(readFilePtr, userReadBuf + total, 3); 
    total += results; 
    printf("\trequested: 3, expected: 1\n\tbytes read: %d\n", results); 
    printf("\tour fileOffset = %d and kernels fileOffset = %ld\n",readFilePtr->fileOffset, lseek(readFilePtr->fd,0,SEEK_CUR));
    printf("\tvalue of userReadBuf starting at %p: \'%s\'\n\n", userReadBuf, userReadBuf); 

    // TEST 5: Try to read the file when all of the file has already been read, 
    // so count > bytesLeft 
    printf("TEST 5\n"); 
    results = myread(readFilePtr, userReadBuf + total, 2); 
    total += results; 
    printf("\trequested: 2, expected: 0\n\tbytes read: %d\n", results); 
    printf("\tour fileOffset = %d and kernels fileOffset = %ld\n",readFilePtr->fileOffset, lseek(readFilePtr->fd,0,SEEK_CUR));
    printf("\tvalue of userReadBuf starting at %p: \'%s\'\n\n", userReadBuf, userReadBuf); 

    // Close the testfile 
    results = myclose(readFilePtr);
    printf("Closing readFilePtr results: %d\n", results); 
}

/*
    Small test for myopen and more tests for mywrite() in myio.c 

    ASSUMPTIONS: BUFF_SIZE is 10
*/
void testMyWrite()
{
    struct File *writeFilePtr;
    char *userWriteBuf;
    int results; 
    int total; 

    total = 0; 

    printf("\n-----TESTING mywrite()-----\n"); 

    userWriteBuf = "If we see this, we wrote correctly!"; 

    writeFilePtr = myopen("writefile", O_CREAT | O_RDWR); 

    // TEST 1: Write an amount of bytes smaller than the BUFF_SIZE, which will not complete a syscall
    printf("TEST 1\n"); 
    results = mywrite(writeFilePtr, userWriteBuf, 5);
    total += results; 
    printf("\tbytes written: %d\n", results); 
    printf("\tour fileOffset = %d and kernels fileOffset = %ld\n",writeFilePtr->fileOffset, lseek(writeFilePtr->fd,0,SEEK_CUR));
    printf("\tvalue of writeFilePtr starting at %p: \'%s\'\n\n", writeFilePtr->hiddenBuf, writeFilePtr->hiddenBuf); 

    // TEST 2: Write an amount of bytes larger than the space left in the file buffer
    // which will complete a syscall and still have some bytes remaining in the buffer
    printf("TEST 2\n"); 
    results = mywrite(writeFilePtr, userWriteBuf + total, 7);
    total += results; 
    printf("\tbytes written: %d\n", results); 
    // do we care about what is in the buffer tho? bc it can do a syscall during the function
    printf("\tour fileOffset = %d and kernels fileOffset = %ld\n",writeFilePtr->fileOffset, lseek(writeFilePtr->fd,0,SEEK_CUR));
    printf("\tvalue of writeFilePtr starting at %p: \'%s\'\n\n", writeFilePtr->hiddenBuf, writeFilePtr->hiddenBuf); 

    results = mywrite(writeFilePtr, userWriteBuf + total, 23);
    total += results; 
    printf("\tbytes written: %d\n", results); 
    printf("\tour fileOffset = %d and kernels fileOffset = %ld\n",writeFilePtr->fileOffset, lseek(writeFilePtr->fd,0,SEEK_CUR));
    printf("\tvalue of writeFilePtr starting at %p: \'%s\'\n\n", writeFilePtr->hiddenBuf, writeFilePtr->hiddenBuf); 

    // TEST 3: Closing and flushing writeBuf  
    results = myclose(writeFilePtr); 
    printf("Closing writeFilePtr results: %d\n", results); 

    // DO WE HAVE TO TAKE INTO ACCOUNT IF THEY WANT TO WRITE MORE BYTES THAN THEY HAVE? 
}

/*
    Tests for switching between mywrite and myread

    ASSUMPTIONS: BUFF_SIZE = 10, writereadfile holds: 
    "should we put stuff in it?!"
*/
void testWriteRead()
{
    struct File *readFilePtr;
    char *userReadBuf;
    int results; 
    int total; 

    char *userWriteBuf;

    total = 0; 
    userWriteBuf = "If we see this, we wrote correctly!"; 

    printf("\n-----TESTING mywrite() and then myread()-----\n"); 
    userReadBuf = malloc(30); 

    // Open the readwritefile
    readFilePtr = myopen("writereadfile", O_RDWR); // writereadfile has 27 characters 
    
    // TEST 1: Write an amount of bytes smaller than the BUFF_SIZE, which will not complete a syscall
    printf("TEST 1: WRITE \n"); 
    results = mywrite(readFilePtr, userWriteBuf, 5);
    total += results; 
    printf("\tbytes written: %d\n", results); 
    printf("\tour fileOffset = %d and kernels fileOffset = %ld\n",readFilePtr->fileOffset, lseek(readFilePtr->fd,0,SEEK_CUR));
    printf("\tvalue of readFilePtr starting at %p: \'%s\'\n\n", readFilePtr->hiddenBuf, readFilePtr->hiddenBuf); 

    // TEST 2: Request count that uses the rest of readBuf and then makes syscall straight to buf 
    //count > BUFF_SIZE and count > bytesLeft and count < total bytes in file 
    printf("TEST 2: READ\n"); 
    results = myread(readFilePtr, userReadBuf + total, 14); 
    total += results; 
    printf("\tbytes read: %d\n", results); 
    printf("\tour fileOffset = %d and kernels fileOffset = %ld\n",readFilePtr->fileOffset, lseek(readFilePtr->fd,0,SEEK_CUR));
    printf("\tvalue of userReadBuf starting at %p: \'%s\'\n\n", userReadBuf, userReadBuf); 

    // TEST 3: Try writing again
    // printf("TEST 3: WRITE \n"); 
    // results = mywrite(readFilePtr, userWriteBuf, 5);
    // total += results; 
    // printf("\tbytes written: %d\n", results); 
    // printf("\tvalue of readFilePtr starting at %p: \'%s\'\n\n", readFilePtr->hiddenBuf, readFilePtr->hiddenBuf); 
    //looks like it's not moving the write cp or something like that 

    // Close the testfile 
    results = myclose(readFilePtr);
    printf("Closing readFilePtr results: %d\n", results); 
    printf("writereadfile contents should be: If wed we put stuff in it?!\n"); 

    // Expected outcome with TEST 3: If wed we put stuff see  in it?! 

}

/*
    Tests for switching between myread and mywrite 

    ASSUMPTIONS: BUFF_SIZE = 10, readwritefile holds: 
    "should we put stuff in it?!"
*/
void testReadWrite()
{
    struct File *readFilePtr;
    char *userReadBuf;
    int results; 
    int total; 

    char *userWriteBuf;

    total = 0; 
    userWriteBuf = "If we see this, we wrote correctly!"; 

    printf("\n-----TESTING myread() and then mywrite()-----\n"); 
    userReadBuf = malloc(30); 

    // Open the readwritefile
    readFilePtr = myopen("readwritefile", O_RDWR); // readwritefile has 27 characters 
    
    // TEST 1: Request count smaller than buff size and smaller than file size, 
    // so count < BUFF_SIZE < total bytes in file 
    printf("TEST 1: READ\n"); 
    results = myread(readFilePtr, userReadBuf, 7);
    total += results; 
    printf("\tbytes read: %d\n", results); 
    printf("\tvalue of userReadBuf starting at %p: \'%s\'\n\n", userReadBuf, userReadBuf); 

    // TEST 2: Write an amount of bytes smaller than the BUFF_SIZE, which will not complete a syscall
    //we should get: "If wed we put stuff"
    printf("TEST 1: WRITE \n"); 
    results = mywrite(readFilePtr, userWriteBuf, 5);
    total += results; 
    printf("\tbytes written: %d\n", results); 
    printf("\tvalue of readFilePtr starting at %p: \'%s\'\n\n", readFilePtr->hiddenBuf, readFilePtr->hiddenBuf); 

    // TEST 3: Request count that uses the rest of readBuf and then makes syscall straight to buf 
    //count > BUFF_SIZE and count > bytesLeft and count < total bytes in file 
    // printf("TEST 3: READ\n"); 
    // results = myread(readFilePtr, userReadBuf + total, 14); // SHOULD WE BE PUTTING TOTAL WITH THE WRITE COUNT THERE?
    // total += results; 
    // printf("\tbytes read: %d\n", results); 
    // printf("\tvalue of userReadBuf starting at %p: \'%s\'\n\n", userReadBuf, userReadBuf); 

    // Close the testfile 
    results = myclose(readFilePtr);
    printf("Closing readFilePtr results: %d\n", results); 
    printf("readwritefile contents should be: should If wet stuff in it?!\n"); 

    // Expected outcome with TEST 3: should If wet stuff in it?!

}

void testMySeek()
{
    struct File *filePtr;
    char *userReadBuf;

    printf("\n-----TESTING myseek()-----\n"); 

    userReadBuf = malloc(30); 
    filePtr = myopen("testfile",O_RDWR);
    
    printf("\nfileOffset: %ld\n", lseek(filePtr->fd, 0, SEEK_CUR));
    printf("at the beginning readCP: %p\n", filePtr->CP);
    
    printf("Request to read 7 bytes\n");
    myread(filePtr, userReadBuf, 7); 
    printf("\tafter read 7 readCP: %p\n", filePtr->CP);
    printf("\tfileOffset: %ld\n", lseek(filePtr->fd, 0, SEEK_CUR)); 
   
    // CHANGE THIS SECTION OF CODE ACCORDINGLY FOR TESTS
    //when myseek SEEK_SET 2:  "should ould we pu"
    //when myseek SEEK_CUR 2:  "should  put stuff"
    //when myseek SEEK_CUR -2: "should d we put s"
    // NEED TO TEST WHEN SEEK_SET OR SEEK_CUR ARE SET TO VALUES OUTSIDE BUFF
    printf("myseek\n"); 
    myseek(filePtr, 12, SEEK_SET);
    printf("\tafter myseek readCP: %p\n", filePtr->CP);
    printf("\tfileOffset: %ld\n", lseek(filePtr->fd, 0, SEEK_CUR)); 
    printf("\tbytesLeft (after): %d\n", filePtr->bytesLeft); 
    // -------------------------------------------------

    printf("Request to read 10 bytes\n");
    myread(filePtr, userReadBuf + 7, 10); 
    printf("\tafter read 10 readCP: %p\n", filePtr->CP);
    printf("\tfileOffset: %ld\n", lseek(filePtr->fd, 0, SEEK_CUR)); 

    printf("\n***this is whats in the userReadBuf at %p: %s***\n", userReadBuf, userReadBuf); 
}