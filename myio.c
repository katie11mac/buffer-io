/*
 * myio.c
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <myio.h> 

#include <fcntl.h> 

#define BYTES_PER_ITERATION 10240

int main(int argc, char *argv[])
{
    printf("TESTING"); 
    return 0; 
}

/*
* Opens a file located at the pathname and returns the file descriptor. 
*/
int myopen(const char *pathname, int flags) 
{
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
* Reads count bytes from fd into the buf 
*/
int myread(int fd, void *buf, size_t count) 
{
    return 0; 
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