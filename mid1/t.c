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
int color;
#include "type.h"
#include "vid.c"
#include "string.c"
#include "timer.c"
#include "queue.c"
#include "kernel.c"
#include "kbd.c"
#include "uart.c"
#include "exceptions.c"
//#include "kernel.c"

UART *up = &uart[0];

void copy_vectors(void) {
    extern u32 vectors_start;
    extern u32 vectors_end;
    u32 *vectors_src = &vectors_start;
    u32 *vectors_dst = (u32 *)0;

    while(vectors_src < &vectors_end)
       *vectors_dst++ = *vectors_src++;
}
int kprintf(char *fmt, ...);

void IRQ_handler() /*(interrupt)*/
{
    int vicstatus, sicstatus;
    int ustatus, kstatus;

    if (vicstatus & (1<<4)){   // timer0,1=bit4
         timer_handler(0);
    }
    if (vicstatus & (1<<12))
       uart_handler(&uart[0]);

    // read VIC status register to find out which interrupt
    vicstatus = VIC_STATUS; // VIC_STATUS=0x10140000=status reg
    sicstatus = SIC_STATUS;  
    //kprintf("vicstatus=%x sicstatus=%x\n", vicstatus, sicstatus);
    if (vicstatus & 0x80000000){
       if (sicstatus & 0x08){
          kbd_handler();
       }
    }
}

int body(int pid, int ppid,int priority, int func);
int kbd_task();
int uart_task();

int uart_task()
{
   char line[128];
   while(1)
   {
     uprintf(up, "uart_task %d sleep for line from UART\n", running->pid);

     ksleep((int)&uart[0]); 
    
     uprintf(up, "uart_task %d running\n", running->pid);
     ugets(up,line);
     uprintf(up, "line = %s\n", line);
  }
}

int kbd_task()
{
	char line[128];
	while(1)
  {
    printf("KBD task %d sleep for a line from KBD\n", running->pid);
    
    ksleep((int)&kbd);
    
    printf("KBD task %d running\n", running->pid);
    kgets(line);
    printf("line = %s\n", line);
	}
}

int main()
{ 
   int i; 
   char line[128]; 
   u8 kbdstatus, key, scode;

   color = WHITE;
   row = col = 0; 

   VIC_INTENABLE |= (1<<4);  // timer0,1 at bit4
   VIC_INTENABLE |= (1<<5);  // timer2,3 at bit5
   VIC_INTENABLE |= (1<<12); // UART0 at bit12
   VIC_INTENABLE |= (1<<13); // UART1 at bit13
   fbuf_init();
   timer_init();
   timer_start(0);
   kbd_init();
   uart_init();
  
   /* enable KBD IRQ */
   VIC_INTENABLE |= 1<<31;  // SIC to VIC's IRQ31
   SIC_ENSET |= 1<<3;       // KBD int=3 on SIC

   kprintf("Welcome to WANIX in Arm\n");

   init();
   //kfork((int)body, 1);
   kfork((int)uart_task,2);
   kfork((int)kbd_task,1);
   
   while(1){
     if (readyQueue)
        tswitch();
   }
}
