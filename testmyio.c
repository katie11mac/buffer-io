#include "myio.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

void testMyRead(); 
void testMyWrite();

int main(int argc, char *argv[])
{
    struct File *filePtr;
    char *userReadBuf;
    char *userWriteBuf;
    int results; 

    userReadBuf = malloc(30); 
    filePtr = myopen("testfile",O_RDWR);
    //printf("fd is %d readBuf pointer value is %p and readCP pointer value is %p \n",filePtr->fd, filePtr->readBuf, filePtr->readCP);
    
    printf("at the beginning readCP: %p\n", filePtr->readCP);
    myread(filePtr, userReadBuf, 7); 
    printf("fileOffset: %d\n", filePtr->fileOffset); 
    printf("after read 7 readCP: %p\n", filePtr->readCP);
    myseek(filePtr, -2, SEEK_CUR);
    printf("fileOffset: %d\n", filePtr->fileOffset); 
    printf("after myseek readCP: %p\n", filePtr->readCP);
   
    myread(filePtr, userReadBuf + 7, 10); //we should get "should d we put s"
    printf("fileOffset: %d\n", filePtr->fileOffset); 
    printf("***this is whats in the userReadBuf at %p: %s***\n", userReadBuf, userReadBuf); 

    // results = myread(readFilePtr, userReadBuf, 40); 
    // //USE TO TEST WHEN BUFF=50 FOR COUNT<BUFF & COUNT>BYTES IN FILE
    // //USE TO TEST WHEN BUFF=30, COUNT>BUFF & BUFF>BYTES IN FILE
    // printf("***this is whats in the userReadBuf at %p: %s***\n", userReadBuf, userReadBuf); 
    // printf("bytes read: %d\n\n", results); 

    //use following two to test one read and then another from readBuf when BUFF=30
    // results = myread(readFilePtr, userReadBuf, 5); //USE TO TEST WHEN BUFF=10, FOR COUNT<BUFF_SIZE<BYTES IN FILE
    // printf("***this is whats in the userReadBuf at %p: %s***\n", userReadBuf, userReadBuf); 
    // printf("bytes read: %d\n\n", results); 

    // results = myread(readFilePtr, userReadBuf+5, 5); //USE TO TEST WHEN BUFF=10, FOR COUNT<BUFF_SIZE<BYTES IN FILE
    // printf("***this is whats in the userReadBuf at %p: %s***\n", userReadBuf, userReadBuf); 
    // printf("bytes read: %d\n\n", results); 
    
    //use following two to check if you want case where second call uses rest of readBuf then makes syscall straight to buf BUFF_SIZE =10, readUserBuf =30
    // results = myread(readFilePtr, userReadBuf, 7); 
    // printf("***this is whats in the userReadBuf at %p: %s***\n", userReadBuf, userReadBuf); 
    // printf("bytes read: %d\n\n", results); 
    // results = myread(readFilePtr, userReadBuf+7, 14); 
    // printf("***this is whats in the userReadBuf at %p: %s***\n", userReadBuf, userReadBuf); 
    // printf("bytes read: %d\n\n", results); 

    //use following two to check if you have want case where second call uses rest of readBuf then reads and memcpys per usual BUFF_SIZE =10, readUserBuf =30
    // results = myread(readFilePtr, userReadBuf, 7); 
    // //printf("***this is whats in the userReadBuf at %p: %s***\n", userReadBuf, userReadBuf);
    // printf("fileOffset is %d\n",readFilePtr->fileOffset); 

    // //printf("bytes read: %d\n\n", results); 
    // results = myread(readFilePtr, userReadBuf+7, 10); 
    // printf("fileOffset is %d\n",readFilePtr->fileOffset); 
    //printf("***this is whats in the userReadBuf at %p: %s***\n", userReadBuf, userReadBuf); 
   //printf("bytes read: %d\n\n", results); 

    //use following 2 to check, normal, end of readBuf, then refill readBuf but count>bytesLeft, BUFF_SIZE=20
    // results = myread(readFilePtr, userReadBuf, 19); 
    // printf("***this is whats in the userReadBuf at %p: %s***\n", userReadBuf, userReadBuf); 
    // printf("bytes read: %d\n\n", results); 
    // results = myread(readFilePtr, userReadBuf+19, 6); 
    // printf("***this is whats in the userReadBuf at %p: %s***\n", userReadBuf, userReadBuf); 
    // printf("bytes read: %d\n\n", results);
    // results = myread(readFilePtr, userReadBuf+25, 1); 
    // printf("***this is whats in the userReadBuf at %p: %s***\n", userReadBuf, userReadBuf); 
    // printf("bytes read: %d\n\n", results);
    // results = myread(readFilePtr, userReadBuf+26, 2); 
    // printf("***this is whats in the userReadBuf at %p: %s***\n", userReadBuf, userReadBuf); 
    // printf("bytes read: %d\n\n", results);

    // results = myread(readFilePtr, userReadBuf, 30); // NEED TO TEST IT OUT WHEN YOU REQUEST MORE BYTES THAN THE FILE HAS 
    // printf("***this is whats in the userReadBuf at %p: %s***\n", userReadBuf, userReadBuf); 
    // printf("bytes read: %d\n\n", results); 



    //TESTS FOR MYWRITE FUNCTION
    // userWriteBuf = "If we see this, we wrote correctly!";
    // writeFilePtr = myopen("writeTestFile",O_RDWR);
    // results = mywrite(writeFilePtr, userWriteBuf, 5);
    // printf("fileOffset is %d\n",writeFilePtr->fileOffset);
    // // printf("bytes written: %d\n", results); 

    // results = mywrite(writeFilePtr, userWriteBuf+5, 7);
    // // printf("bytes written: %d\n", results); 
    // printf("fileOffset is %d\n",writeFilePtr->fileOffset);

    // results = mywrite(writeFilePtr, userWriteBuf+12, 10);
    // // printf("bytes written: %d\n", results);
    // printf("fileOffset is %d\n",writeFilePtr->fileOffset); 
    // myclose(writeFilePtr); 

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

    printf("\n-----TESTING myread-----\n"); 
    userReadBuf = malloc(30); 

    // Open the testfile
    readFilePtr = myopen("testfile", O_RDWR); // testfile has 27 characters 
    
    // TEST 1: Request count smaller than buff size and smaller than file size, 
    // so count < BUFF_SIZE < total bytes in file 
    printf("TEST 1\n"); 
    results = myread(readFilePtr, userReadBuf, 7);
    total += results; 
    printf("\tbytes read: %d\n", results); 
    printf("\tvalue of userReadBuf starting at %p: \'%s\'\n\n", userReadBuf, userReadBuf); 

    // TEST 2: Request count that uses the rest of readBuf and then makes syscall straight to buf 
    // count > BUFF_SIZE and count > bytesLeft and count < total bytes in file 
    printf("TEST 2\n"); 
    results = myread(readFilePtr, userReadBuf + total, 14); 
    total += results; 
    printf("\tbytes read: %d\n", results); 
    printf("\tvalue of userReadBuf starting at %p: \'%s\'\n\n", userReadBuf, userReadBuf); 

    // 
    results = myread(readFilePtr, userReadBuf + total, 4); 
    total += results; 
    printf("\tbytes read: %d\n", results); 
    printf("\tvalue of userReadBuf starting at %p: \'%s\'\n\n", userReadBuf, userReadBuf); 

    // TEST 3: Request amount smaller than bytesLeft when close to the end of the file, 
    // so count < bytesLeft 
    printf("TEST 3\n"); 
    results = myread(readFilePtr, userReadBuf + total, 1); 
    total += results; 
    printf("\tbytes read: %d\n", results); 
    printf("\tvalue of userReadBuf starting at %p: \'%s\'\n\n", userReadBuf, userReadBuf); 

    // TEST 4: Request amount larger than bytesLeft when there should only be one byte left in the file, 
    // so count > bytesLeft
    printf("TEST 4\n"); 
    results = myread(readFilePtr, userReadBuf + total, 3); 
    total += results; 
    printf("\tbytes read: %d\n", results); 
    printf("\tvalue of userReadBuf starting at %p: \'%s\'\n\n", userReadBuf, userReadBuf); 

    // TEST 5: Try to read the file when all of the file has already been read, 
    // so count > bytesLeft 
    printf("TEST 5\n"); 
    results = myread(readFilePtr, userReadBuf + total, 2); 
    total += results; 
    printf("\tbytes read: %d\n", results); 
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

    writeFilePtr = myopen("writeFile", O_CREAT | O_RDWR); 

    // TEST 1: Write an amount of bytes smaller than the BUFF_SIZE, which will not complete a syscall
    printf("TEST 1\n"); 
    results = mywrite(writeFilePtr, userWriteBuf, 5);
    total += results; 
    printf("\tbytes written: %d\n", results); 
    printf("\tvalue of writeFilePtr starting at %p: \'%s\'\n\n", writeFilePtr->writeBuf, writeFilePtr->writeBuf); 

    // TEST 2: Write an amount of bytes larger than the space left in the file buffer
    // which will complete a syscall and still have some bytes remaining in the buffer
    printf("TEST 2\n"); 
    results = mywrite(writeFilePtr, userWriteBuf + total, 7);
    total += results; 
    printf("\tbytes written: %d\n", results); 
    printf("\tvalue of writeFilePtr starting at %p: \'%s\'\n\n", writeFilePtr->writeBuf, writeFilePtr->writeBuf); 

    results = mywrite(writeFilePtr, userWriteBuf + total, 23);
    total += results; 
    printf("\tbytes written: %d\n", results); 
    printf("\tvalue of writeFilePtr starting at %p: \'%s\'\n\n", writeFilePtr->writeBuf, writeFilePtr->writeBuf); 

    // TEST 3: Closing and flushing writeBuf  
    results = myclose(writeFilePtr); 
    printf("Closing writeFilePtr results: %d\n", results); 

    // DO WE HAVE TO TAKE INTO ACCOUNT IF THEY WANT TO WRITE MORE BYTES THAN THEY HAVE? 

    // results = mywrite(writeFilePtr, userWriteBuf+12, 10);
    // // printf("bytes written: %d\n", results);
    // printf("fileOffset is %d\n",writeFilePtr->fileOffset); 
}