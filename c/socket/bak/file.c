#include<stdio.h>
#include "file.h"

/* 数据文件存储方式 */
int file_init(char *Path)
{
	FILE  *pFile;
	unsigned long  u32Ret;
   	unsigned char aucfg[100] = "file is init";

	pFile = fopen(Path, "r");
	if(NULL == pFile)
	{
		printf("\r\n create  cfg \r\n");
		pFile = fopen(Path, "w");
		if(NULL != pFile)
		{
			//sprintf(aucfg,"init", name);
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

char *file_read(char *Path, char *Name)
{
	FILE *pFile;
	unsigned char * pFileBuf;
	unsigned char *pWork;
	unsigned int ulRet;
	unsigned long ulFileLen =0;
	unsigned char buf[BUFFER_SIZE] = {0};
	
	pFile = fopen(Path,"r");
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
	pWork = strstr(pFileBuf, Name);
	if(NULL != pWork)
	{
		sscanf(pWork,"%s:%s",Name,&buf);
		printf("\r\n name= %s\r\n",buf);
	}	
	fclose(pFile);
	return buf;
 faild:
	fclose(pFile);
	return buf;
}

int file_write(char *Path, char *Name, char *Buf)
{
	FILE *pFile;
	unsigned long  u32Ret;
   	unsigned char aucfg[BUFFER_SIZE] = {0};
	pFile = fopen(Path, "r");
	if(NULL == pFile)
	{
		printf("\r\n cfg NULL\r\n");
		fclose(pFile);
		return 0;	
	}	
	sprintf(aucfg,"%s:%s\n", Name, Buf);
	fputs(aucfg, pFile);
	fclose(pFile);
	return 0;

}


