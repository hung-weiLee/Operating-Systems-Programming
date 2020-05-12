/********************************************************************
Copyright 2010-2017 K.C. Wang, <kwang@eecs.wsu.edu>
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
********************************************************************/
// write YOUR OWN
strlen(), strcmp(), strcpy(), memcpy(), geti();

int geti(char *s)
{
    //char *s = kgets(s);
    //return atoi(s);
    int sum = 0;
    while(*s != '\r')
    {
        sum = sum * 10;
        sum += (*s-'0');
        s++;
    }
    return sum;
}

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


int strncmp(const char *str1, const char *str2, int n) // compare str1 , str2 for n bytes
{
    while( n && (*str1 && *str2 != 0) )
    {
        if(*str1 != *str2)
        {
            return 1; // not same string
        }
        n--;
        str1++;
        str2++;
    }

    return 0; // same string
}