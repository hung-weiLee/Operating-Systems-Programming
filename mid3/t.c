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
#include "string.c"
#include "queue.c"
#include "kbd.c"
#include "vid.c"
#include "exceptions.c"
#include "kernel.c"
#include "timer.c"

void copy_vectors(void) {
    extern u32 vectors_start;
    extern u32 vectors_end;
    u32 *vectors_src = &vectors_start;
    u32 *vectors_dst = (u32 *)0;

    while(vectors_src < &vectors_end)
       *vectors_dst++ = *vectors_src++;
}
int kprintf(char *fmt, ...);
int color;
void timer_handler();

TIMER *tp[4];

void IRQ_handler()
{
    int vicstatus, sicstatus;
    int ustatus, kstatus;

    // read VIC status register to find out which interrupt
    vicstatus = VIC_STATUS; // VIC_STATUS=0x10140000=status reg
    sicstatus = SIC_STATUS;  
    //kprintf("vicstatus=%x sicstatus=%x\n", vicstatus, sicstatus);

    if (vicstatus & (1<<4)){   // timer0,1=bit4
         timer_handler(0);
    }    

    if (vicstatus & 0x80000000){
       if (sicstatus & 0x08){
          kbd_handler();
       }
    }
}


char buf[8];
int head = 0;
int tail = 0;

SEMAPHORE empty, full, pmutex, cmutex; // declare 4 SEMAPHORE structure => (empty, full, pmutex, cmutex)

/***********************************************************/

int p1() /*producer*/
{
    //kprintf("p1");
    while(1)
    {
        char item = kgetc();
        P(&empty); // empty--
            P(&pmutex); // pmutex->value--
                kprintf("%c", item);
                buf[head] = item; // put char into buffer
                head++;
                head %= 8;
            V(&pmutex); // pmutex->value++
        V(&full); // full++
    }
}

int p2() /*consumer*/
{
    //kprintf("p2");
    while(1)
    {
        P(&full); // full--
            P(&cmutex); // cmutex->value--
                char item = buf[tail]; // get char out of buffer
                tail++;
                kprintf("%c", item);
                tail %= 8;
            V(&cmutex); // cmutex->value++
        V(&empty); // empty++
    }
}

/***********************************************************/

int body(int pid, int ppid, int func, int priority);

int main()
{ 
   empty.value = 8; full.value = 0; pmutex.value = 1; cmutex.value = 1; /* !!! */
   empty.queue = 0; full.queue = 0; /* null pointer */


   int i; 
   char line[128]; 
   u8 kbdstatus, key, scode;

   color = WHITE;
   row = col = 0; 

   fbuf_init();

    /* enable timer0,1, uart0,1 SIC interrupts */
   VIC_INTENABLE |= (1<<4);  // timer0,1 at bit4 
   VIC_INTENABLE |= (1<<5);  // timer2,3 at bit5 

   VIC_INTENABLE |= (1<<31); // SIC to VIC's IRQ31

   /* enable KBD IRQ */
   VIC_INTENABLE |= 1<<31;  // SIC to VIC's IRQ31
   SIC_ENSET |= 1<<3;       // KBD int=3 on SIC

   timer_init();
   kbd_init();

   timer_start(0);
   kprintf("Welcome to WANIX in Arm\n");
   
   init(); // pop P0 from freeList
   
   kfork((int)p1, 1); 
   kfork((int)p2, 1); 

   while(1)
   {
     if (readyQueue)
     /* tswitch will call (scheduler function) and (p1 function) !!! */
        tswitch();  // tswitch is the assemble code in (ts.s)
   }
}
