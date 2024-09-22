
void LTrimZeroChr(char* str)
{
	char *z = NULL;
	char *p = NULL;

	//找小数点
	for (; *str ; str++)
	{
		if(*str == '.')
		{
			p = str++;
			break;
		}
	}
	//
	if(p) //存在小数点
	{
		for (; *str; str++)
		{
			if (*str == '0')
			{
				if(!z)
				{
					z = str;
				}
			}
			else
			{
				z = NULL;
			}
		}
		//
		if (z)
		{
			
			if(z == p + 1) //以小数点结尾则删除小数点
			{
				*p = '\0';
			}
			else
			{
				*z = '\0';
			}
		}
	}
}