#include <stdio.h>

/*
	冒泡排序取最大值
*/
#if 0
int max(int s1[], int s2[], int n)
{
	int str[20];
	int temp = 0;
	strcpy(str,s1);
	strcat(str,s2);

	for(int i=0; i<n-1; ++i)
		for(int j=0; j<n-i-1; ++j)	
		{
			temp= str[j];
			str[j] = str[j+1];
			str[j+1] = temp;
		}
	printf("\r\n temp  = %d \r\n ",temp);
	
}
#endif


unsigned num(int a[])
{
	unsigned n = 0;
	while(a[n] != '\0')
	{
		n++;
	}
	
	return n+1;
}

void max(int str[], int n)
{
	for(int i=0;i<n-1;++i)
		for(int j=0; j<n-i-1;j++)
		{
			if(str[j+1] >str[j])
			{
				int temp = str[j+1];
				str[j+1] = str[j];
				str[j] = temp;
			}
		}	

}

int main()
{
	int a[] = {2,7,10,33,42};
	int b[] = {1,3,5,6,7};
	char c[] = "gdble";
	char d[] = "view";
	char *str = malloc(sizeof(c) +sizeof(d));
	
	strcpy(str,c);
	strcat(str,d);
	printf("\r\n %s \r\n",str);
#if 0
	int n = num(str);
	max(str,n);
	for(int i =0;i<n;i++)
	{
		printf("\r\n str[%d] = %d\r\n",i,str[i]);
	}
#endif
	return 0;
}
