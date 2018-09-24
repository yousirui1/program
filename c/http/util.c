




//
char *to_upper(char *str)
{
	char *start = str;
	
	while(*str)
	{
		if(*str == '-')
			*str = '_';  		
		else
			*str = toupper(*str);	 //小写转大写
		str++;
	}
	return start;
}
