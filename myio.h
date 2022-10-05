struct File * myopen(const char *pathname, int flags);
int myclose(int fd); 
int myread(struct File *filePtr, void *buf, size_t count);
int mywrite(int fd, const void *buf, size_t count); 
void myflush(int fd, void *buf, size_t size); 