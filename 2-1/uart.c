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
#define DR 0x00
#define FR 0x18

#define RXFE 0x10
#define TXFF 0x20

typedef struct uart 
{
    char *base;
    int n;
} UART;

UART uart[4];

int uart_init()
{
    int i;
    UART* up;

    for (i = 0; i < 4; i++) // uart[0] ~ uart[3]
    {
        up = &uart[i];
        up->base = (char*)(0x10009000 + i * 0x1000); // base address + i*0x1000
        up->n = i;
    }
}

int ugetc(UART* up) // get char
{
    while (*(up->base + FR) & RXFE)
        ;
    return *(up->base + DR);
}

int uputc(UART* up, char c) // put char
{
    while (*(up->base + FR) & TXFF)
        ;
    *(up->base + DR) = c;
}

int ugets(UART* up, char* s) // get string
{
    while ((*s = (char)ugetc(up)) != '\r') 
    {
        uputc(up, *s);
        s++;
    }
    *s = 0;
}

int uprints(UART* up, char* s) // put string
{
    while (*s)
        uputc(up, *s++);
}

int uputs(UART *up, char *s) // put string
{
  while(*s)
  {
    uputc(up, *s++);
    if (*s=='\n')
      uputc(up,'\r');
  }
}

int urpx(UART *up, int x) // 16
{
  char c;
  if (x)
  {
     c = tab[x % 16];
     urpx(up, x / 16);
  }
  uputc(up, c);
}

int uprintx(UART *up, int x) // 16
{
  uprints(up, "0x");
  if (x==0)
    uputc(up, '0');
  else
    urpx(up, x);
  uputc(up, ' ');
}

int urpu(UART *up, int x) // unsigned char
{
  char c;
  if (x)
  {
     c = tab[x % 10]; // tab = "0123456789ABCDEF";
     urpu(up, x / 10);
  }
  uputc(up, c);
}

int uprintu(UART *up, int x) // unsigned char(0~255)
{
  if (x==0)
    uputc(up, '0');
  else
    urpu(up, x);
  uputc(up, ' ');
}

int uprinti(UART *up, int x) // int
{
  if (x<0)
  {
    uputc(up, '-');
    x = -x;
  }
  uprintu(up, x);
}

int fuprintf(UART *up, char *fmt,...)
{
  int *ip;
  char *cp;
  cp = fmt;
  ip = (int *)&fmt + 1; // ip point to next bytes

  while(*cp)
  {
    if (*cp != '%')
    {
      uputc(up, *cp);
      
      if (*cp=='\n') // next line
      {
        uputc(up, '\r'); // Enter
      }
      
      cp++; // point to next 
     
      continue;
    }
  //else => get '%' 
   
    cp++; // point to next
   
    switch(*cp)
    {
      case 'c': uputc(up, (char)*ip);      break;  // put char
      case 's': uprints(up, (char *)*ip);  break; // print string
      case 'd': uprinti(up, *ip);          break; // print int
      case 'u': uprintu(up, *ip);          break; // print unsign char
      case 'x': uprintx(up, *ip);          break; // print  16(0x)
    }
    
    cp++; // point to next
    ip++; // ip point to next bytes
  }
}
