int strlen(const char *str)
{
    int length = 0;
    while (*str != '\0') // string have data
    {
      str++;
      length++;
    }

    return length; // return string length
}

int strcmp(const char *str1, const char *str2) // compare str1 , str2
{
    if( strlen(str1) != strlen(str2) ) // compare string length
    {
        return 1; // not same string
    }

    while(*str1 != '\0')
    {
        if(*str1 != *str2)
        {
            return 1; // not same string
        }
        str1++;
        str2++;
    }

    return 0; // same string
}

int strcpy(char *dest, const char *src) // copy src to dest
{
    while(*src != '\0')
    {
      *dest = *src;
        
      dest++;
      src++;
    }

    *dest = '\0';
}

int strncpy(char *dest, const char *str, int n) // copy src to dest for n bytes
{
    char *cp = dest;

    /*  n!=0 &&   copy str to cp , cp != '\0' */
    while( n && ( (*cp = *str) != '\0') ) 
    {
      n--;
      str++;
      cp++;
    }
    
    if(n) // n still have space => (n != 0)
    {
        while(n--)
        {
            *cp = '\0'; // input '\0' to remain space
            cp++;
        }
    }
    return 1;
}

int memcpy(char *dst, char *src, int n) // copy n bytes from src to dst
{
    char *r = dst;
    const char *s = src;

    while(n--)
    {
        *r = *s;
        r++;
        s++;
    }

    return 1;
}

int memset(char *dst, char c, int n) // copy n times c into dst
{
    char *r = dst;
    while (n > 0)
    {
        n--;
        *r = c;
        r++;
    }

    return 1;
}
