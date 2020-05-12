int strlen(const char* str)
{
    int length = 0;
    while (*str++)
        ++length;
    return (length);
}

int strcmp(const char* str1, const char* str2)
{
    while ((*str1) && (*str1 == *str2)) {
        str1++;
        str2++;
    }

    if (*(unsigned char*)str1 > *(unsigned char*)str2) {
        return 1;
    } else if (*(unsigned char*)str1 < *(unsigned char*)str2) {
        return -1;
    } else {
        return 0;
    }
}

int strcpy(char* dest, const char* src)
{
    /*
    if(dest != NULL && src != NULL)
    {
        return 0;
    }
    */

    char* ret = dest;

    while ((*dest++ = *src++) != 0)
        ;

    return 1;
}

int strncpy(char* dest, const char* str, int n)
{
    // assert((dest != NULL) && (str != NULL));
    char* cp = dest;
    while (n && (*cp++ = *str++) != 0) {
        n--;
    }
    if (n) {
        while (--n)
            *cp++ = 0;
    }
    return 1;
}

int memcpy(char* dst, const char* src, int len)
{
    /*
 if(NULL == dst || NULL == src){
  return NULL;
 }
 */

    char* ret = dst;

    if (dst <= src || (char*)dst >= (char*)src + len) {
        //没有内存重叠，从低地址开始复制
        while (len--) {
            *(char*)dst = *(char*)src;
            dst = (char*)dst + 1;
            src = (char*)src + 1;
        }
    } else {
        //有内存重叠，从高地址开始复制
        src = (char*)src + len - 1;
        dst = (char*)dst + len - 1;
        while (len--) {
            *(char*)dst = *(char*)src;
            dst = (char*)dst - 1;
            src = (char*)src - 1;
        }
    }
    return 1;
}

int memset(char* dst, char val, int count)
{
    char* ret = dst;
    while (count--) {
        *(char*)dst = (char)val;
        dst = (char*)dst + 1; //移动一个字节
    }
    return 1;
}

int atoi(char* pstr)
{
	int Ret_Integer = 0;
	int Integer_sign = 1;

	/*
	* 判断指针是否为空
	*/
	if(pstr == 0)
	{
		printf("Pointer is NULL\n");
		return 0;
	}

	/*
	* 跳过前面的空格字符
	*/
	while(*pstr == ' ')
	{
		pstr++;
	}

	/*
	* 判断正负号
	* 如果是正号，指针指向下一个字符
	* 如果是符号，把符号标记为Integer_sign置-1，然后再把指针指向下一个字符
	*/
	if(*pstr == '-')
	{
		Integer_sign = -1;
	}
	if(*pstr == '-' || *pstr == '+')
	{
		pstr++;
	}

	/*
	* 把数字字符串逐个转换成整数，并把最后转换好的整数赋给Ret_Integer
	*/
	while(*pstr >= '0' && *pstr <= '9')
	{
		Ret_Integer = Ret_Integer * 10 + *pstr - '0';
		pstr++;
	}
	Ret_Integer = Integer_sign * Ret_Integer;

	return Ret_Integer;
}

int geti(char s[])
{
    //kgets(s);
    return (atoi(s));
}
