/*
 * myio.c
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "myio.h"

#include <fcntl.h> 

#define BUFF_SIZE 5

char *READ_BPTR; 
char *READ_CPTR; 

int main(int argc, char *argv[])
{
    printf("TESTING \n"); 
    myopen("/home/kmacalintal/CS315/assignment2/testfile", 0);  

    return 0; 
}

/*
* Opens a file located at the pathname and returns the file descriptor. 
*/
int myopen(const char *pathname, int flags) 
{
    READ_BPTR = malloc(BUFF_SIZE); 
    READ_CPTR = READ_BPTR; 
    open(pathname, flags);
    return 0; 
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
*/
int myread(int fd, void *buf, size_t count) 
{
    int amtRead; /*offset?*/

    buf = READ_CPTR; 

    if (READ_BPTR == READ_CPTR | READ_CPTR + count - READ_BPTR >= BUFF_SIZE) {
        amtRead = read(fd, READ_BPTR, count); 
    } 
    READ_CPTR += amtRead; 

    printf("%s", *(READ_BPTR));  
    

    return amtRead;
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