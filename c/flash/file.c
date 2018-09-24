#include<stdio.h>

/* 数据文件存储方式 */
int init(void)
{
	FILE  *pFile;
	unsigned long  u32Ret;
   	unsigned char aucfg[100] = {0};
	unsigned char path[20] = {0};

	unsigned char * name = "ysr";
	pFile = fopen("./data.cfg", "r");
	if(NULL == pFile)
	{
		printf("\r\n create  cfg \r\n");
		pFile = fopen("./data.cfg", "w");
		if(NULL != pFile)
		{
			sprintf(aucfg,"name=%s\n", name);
			fputs(aucfg, pFile);
			fclose(pFile);
		}
		else
		{
			printf("\r\n init create cfg faild \r\n");
		}
	}	
	return 0;
}

int read()
{
	FILE *pFile;
	unsigned char * pFileBuf;
	unsigned char *pWork;
	unsigned int ulRet;
	unsigned long ulFileLen =0;
	
	unsigned char *name ={0};
	name = malloc(sizeof(char )*20);
	pFile = fopen("./data.cfg","r");
	/* goto file end */	
	fseek(pFile,0,SEEK_END);
	ulFileLen = ftell(pFile);
	/*goto file start */
	rewind(pFile);
	
	pFileBuf = malloc (ulFileLen + 1);	
	if(NULL == pFileBuf)
	{
		printf("\r\n buf malloc faild\r\n");
		goto faild;
	}
	
	ulFileLen = fread(pFileBuf, sizeof(char), ulFileLen, pFile);
	pFileBuf[ulFileLen] = "\0";

	pWork = strstr(pFileBuf, "name");
	if(NULL != pWork)
	{
		sscanf(pWork,"name=%s",name);
		printf("\r\n name= %s\r\n",name);
	}	
		
	fclose(pFile);
	return 0;
 faild:
	fclose(pFile);
	return 1;
 	

}

int write()
{
	FILE *pFile;
	unsigned long  u32Ret;
   	unsigned char aucfg[100] = {0};
	unsigned char path[20] = {0};
	unsigned char * name = "ysr1";
	pFile = fopen("./data.cfg", "r");
	if(NULL == pFile)
	{
		printf("\r\n cfg NULL\r\n");
		fclose(pFile);
		return 0;	
	}	
	sprintf(aucfg,"name=%s\n", &name);
	fputs(aucfg, pFile);
	fclose(pFile);
	return 0;

}

int main()
{
	init();
	read();
	write();
	read();	
	return 0;
}
