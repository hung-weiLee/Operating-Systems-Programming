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

#include "type.h"
#include "vid.c"
#include "string.c"
#include "timer.c"
#include "queue.c"
#include "kernel.c"
#include "kbd.c"
#include "uart.c"
#include "exceptions.c"
#include "pipe.c"
//#include "kernel.c"

PIPE *kpipe;			// global PIPE pointer

void copy_vectors(void) {
    extern u32 vectors_start;
    extern u32 vectors_end;
    u32 *vectors_src = &vectors_start;
    u32 *vectors_dst = (u32 *)0;

    while(vectors_src < &vectors_end)
       *vectors_dst++ = *vectors_src++;
}
int kprintf(char *fmt, ...);
void IRQ_handler()
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

int pipe_writer() // pipe writer task code
{
  struct uart *up = &uart[0];
  char line[128];
  while(1)
  {
      uprintf(up, "Enter a line for task1 to get : ");
      printf("task%d waits for line from UART0\n", running->pid);
      ugets(up, line);
      uprints(up, "\r\n");
      printf("task%d writes line=[%s] to pipe\n", running->pid, line);
      write_pipe(kpipe, line, strlen(line)); // call write_pipe function
  }
}

int pipe_reader() // pipe reader task code
{
  char line[128];
  int i, n;
  while(1)
  {
      printf("task%d reading from pipe\n", running->pid);
      n = read_pipe(kpipe, line, 20); // call read_pipe function
      printf("task%d read n = %d bytes from pipe : [", running->pid, n);
      for (i=0; i<n; i++)
          kputc(line[i]);
      printf("]\n");
  }
}

int body();
	
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

   pipe_init();
   kpipe = create_pipe(); // kpipe point to pipe[0]

   printf("kpipe data: %d\n", kpipe->data);
   
   kfork((int)pipe_writer, 1); // call pipe_writer function 
   kfork((int)pipe_reader, 1); // call pipe_reader function
   
   while(1){
     if (readyQueue)
        tswitch();
   }
}
