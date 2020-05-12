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

// kbd.c file
#define N_SCAN 64

#include "keymap"
/********* byte offsets; for char *base *********/
#define KCNTL    0x00 // 7- 6-    5(0=AT)  4=RxIntEn 3=TxIntEn  2   1   0
#define KSTAT    0x04 // 7- 6=TxE 5=TxBusy 4=RXFull  3=RxBusy   2   1   0
#define KDATA    0x08 // data register;
#define KCLK     0x0C // clock register;
#define KISTA    0x10 // int status register;


typedef volatile struct kbd{ // base = 0x1000 6000
  char *base;         // base address of KBD, as char *
  char buf[128];
  int head, tail, data, room;
}KBD;

extern int color;
volatile KBD kbd;
int kputc(char);

u16 isShift = 0, flag = 0; /* flag = 0 (press button) , flag = 1 (button up) !!! */

int kbd_init()
{
  KBD *kp = &kbd;
  kp->base = (char *)0x10006000;
  *(kp->base+KCNTL) = 0x14; // 0001 0100
  *(kp->base+KCLK)  = 8;
  kp->data = 0;
  kp->room = 128; 
  kp->head = kp->tail = 0;
}

void kbd_handler()
{
  //kwakeup((int)&kbd); // wake up keyboard

  u8 scode, c;
  // volatile char *t, *tt;
  int i;
  KBD *kp = &kbd;
  color=YELLOW;
  scode = *(kp->base+KDATA);

  /* (Lshift's scan code = 18) , (Rshift's scan code = 89) */
  if( scode == 18 || scode == 89 ) // (shift)
  {
      if(flag == 1) // Second time shift button up
      {
          isShift = 0; // restore
          flag = 0; // restore
          return;
      }
      else // (flag == 0)  ==> press shift
      {
          isShift = 1;
          return;
      }
  }
  if(flag == 1) // Second time button up(except shift)
  {
      flag = 0; // restore flag from 1 back to 0
      return;
  }
  if(scode == 240) // (first time button up scan code) = 240 , and (Second time button up scan code) same as press button scan code
  {
      flag = 1; // flag = 1 ==> (next time will be Second time button up) !!!
      return;
  }

  /*if(scode & 0x80) // skip scan code which is bigger then 0x80(128)
  return;*/
    
  if(isShift) 
      c = sh[scode]; // upper case (isShift == 1)
  else
      c = unsh[scode]; // lower case (isShift == 0)

  if(c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z')
  printf("kbd interrupt: c=%x %c\n", c, c); // print cahr


 // kwakeup(&kbd); // wakeup kbd
  kp->buf[kp->head++] = c; // put cahr into (donut buffer) , and then (pointer head)++
  kp->head %= 128; // where is the head location 
  kp->data++;
  kp->room--;

  kwakeup((int)&kbd); // wake up keyboard
  kwakeup(&kp->data);
}

int kputc(char);  // kputc() in vid.c driver

int kgetc()
{
  char c;
  KBD *kp = &kbd;

  unlock(); // enable IRQ interrupts
  while(kp->data <= 0)
  {
    //printf("sleep");
    ksleep(&kp->data); //(didn't type any char , sleep)
    //printf("proc wake");
  }
  lock(); // disable IRQ interrupts


  c = kp->buf[kp->tail++]; // get (donut buffer)'s cahr into c , and then (pointer tail)++
  kp->tail %= 128;
  kp->data--;
  // update with interrupts OFF
  // enable IRQ interrupts
  unlock();
  return c;
}

int kgets(char s[ ])
{
  char c;
  while((c=kgetc()) != '\r'){
    *s++ = c;
  }
  *s = 0;
  return strlen(s);
}
