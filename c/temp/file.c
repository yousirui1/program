#include <stdio.h>

#define BUFFER_SIZE 1024
int init()
{
	FILE *pFile;
	unsigned char buf[BUFFER_SIZE];
	unsigned char aucfg[100] = {0};
	unsigned char *name = "张三";
	
	
	pFile = fopen("./date.cfg","r");
	if(NULL == pFile)
	{
		pFile = fopen("./date.cfg","w");
		if(NULL != pFile)
		{
			sprintf(aucfg,"teacher=%s\n",name);
			fputs(aucfg,pFile);
			close(pFile);
		}
		else
		{
			printf("\r\n err \r\n");
		}
	}
}


int read()
{
	FILE *pFile;
	unsigned char buf[BUFFER_SIZE];
	unsigned long ulFileLen = 0;
	
	unsigned char *pBuf, *pWork;
	unsigned char name[20] = {0};	
 	pFile = fopen("./date.cfg","r");

	fseek(pFile,0, SEEK_END);
	ulFileLen = ftell(pFile);
	rewind(pFile);

	if(NULL != pFile)
	{
		pBuf = malloc(ulFileLen +1);	
		ulFileLen = fread(pBuf, sizeof(char), ulFileLen, pFile);
		pBuf[ulFileLen +1 ] = "\0";
		
		pWork = strstr(pBuf, "teacher");
		if(NULL != pWork)
		{
			sscanf(pWork, "teacher = %s",&name);
			printf("\r\ name = %s \r\n",name);
		}	
		
	}
	else
	{
		printf("\r\ read error");
	}
	close(pFile);
	return 0;
}
	
int main()
{
	init();
	read();
}

