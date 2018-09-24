#include <stdio.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#define BUFFER_SIZE 30

/*管道通信方式 */
void pipe()
{
	int x;
	int fd[2];
	char buf[BUFFER_SIZE];
	char s[BUFFER_SIZE];
	pipe(fd);	//创建管道
	while((x=fork) == -1);
	
	if(0 ==x )
	{
		sprintf(buf, "This is an example of pipe!/n");
		write(fd[1], buf,BUFFER_SIZE);	
		exit(0);
	}
	else
	{
		wait(0);
		read(fd[0],s,BUFFER_SIZE);	
		printf("%s",s);
	}
	return 0;
}

/*信号量通信*/
void sig_alarm(int sig)
{
	printf("\r\ signal received\r\n");
	signal(SIGINT,SIG_DFL);  //SIG_DFL中断恢复行为	
}



int main()
{
	signal(SIGINT, sig_alarm); //捕捉终端中断信号
	while(1)
	{
		printf("\r\n waiting \r\n");
		sleep(1);
	}
	return 0;
}


/* 共享内存通信*/
#define TEXT_SZ 2048
struct share_use_st
{
	int writeen_by_you;
	char some_text[TEXT_SZ];
};

int main()
{
	int running = 1;
 	void *shared_memory = (void *)0;
	struct share_use_st *shared_stuff;
	char buff[BUFSIZ];
	int shmid;	
	
	shmid = shmget(key_t)1234, sizeof(struct shared_use_st, 0666|IPC_CREAT);
	
	if(shmid == -1)
	{
		fprintf(stderr,"shmget failed /n");	
		exit(EXIT_FAILURE);
	}
	
	shared_memory = shmat(shmid,(void *)0, 0);//指向共享内存第一个字节的指针
	if(shared_memory == (void *)-1)
	{

	}
	shared_stuff = (struct shared_use_st * )shared_memory;
	
	while(running)
	{
		while(shared_stuff->written_by_you == 1)
		{	
			sleep(1);
			printf("waiting for client /n");
		}
	 printf("Enter some text:");
	 fgets(buffer,BUFSIZ,stdin);
	 strncpy(shared_stuff->some_text, buffer, TEXT_SZ);
	 shared_stuff->written_by_you = 1;
	 if(strncmp(buffer, "end", 3) == 0)
	{
		running = 0;
	}
	}
	
}


