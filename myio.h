struct File * myopen(const char *pathname, int flags);
int myclose(struct File *filePtr);
int myread(struct File *filePtr, char *buf, size_t count);
int mywrite(struct File *filePtr, char *buf, size_t count); 
void myflush(struct File *filePtr);
int myseek(struct File *filePtr, off_t offset, int whence); 
