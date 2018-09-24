#include <stdio.h>
#include <fcntl.h>


#define TEST 0
#define TEST_SIZE 100

#define ACG  TEST+TEST_SIZE
#define ACG_SIZE 10

#define GAME ACG + ACG_SIZE
#define GAME_SIZE 20


void t_read(int fd)
{

	lseek(fd, ACG , SEEK_CUR);
	
	char buf[100] = {0};

	read(fd, buf, sizeof(buf));

	printf("%s\n", buf);
}

void t_write(int fd)
{
	char *p = "This is a c test code ";
	lseek(fd, ACG , SEEK_CUR);
	write(fd, p, strlen(p));
	
	lseek(fd, 0, SEEK_SET);

}



int main()
{
	//int len = 0;

	int fd = open("flash", O_RDWR|O_CREAT, 606);

	t_write(fd);
	
	t_read(fd);	

	close(fd);

	return 0;

}
