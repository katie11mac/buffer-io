struct File * myopen(const char *pathname, int flags);
int myclose(int fd); 
int myread(struct File *filePtr, char *buf, size_t count);
int mywrite(struct File *filePtr, const void *buf, size_t count); 
void myflush(int fd, void *buf, size_t size); 