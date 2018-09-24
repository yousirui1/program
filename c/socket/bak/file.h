
#define BUFFER_SIZE 20


int file_init(char *Path);
char *file_read(char *Path, char *Name);
int file_write(char *Path, char *Name, char *Buf);
