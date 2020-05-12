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
#define N_SCAN 128

#include "keymap"
/********* byte offsets; for char *base *********/
#define KCNTL 0x00 // 7- 6-    5(0=AT)  4=RxIntEn 3=TxIntEn  2   1   0
#define KSTAT 0x04 // 7- 6=TxE 5=TxBusy 4=RXFull  3=RxBusy   2   1   0
#define KDATA 0x08 // data register;
#define KCLK 0x0C  // clock register;
#define KISTA 0x10 // int status register;

typedef volatile struct kbd { // base = 0x1000 6000
    char* base;               // base address of KBD, as char *
    char buf[128];
    int head, tail, data, room;
} KBD;
u16 isShift = 0, isRelease = 0;
u8 code = 0;
extern int color;
volatile KBD kbd;
int kputc(char);

int kbd_init()
{
    KBD* kp = &kbd;
    kp->base = (char*)0x10006000;
    *(kp->base + KCNTL) = 0x14; // 0001 0100
    *(kp->base + KCLK) = 8;
    kp->data = 0;
    kp->room = 128;
    kp->head = kp->tail = 0;
}

void kbd_handler()
{
    u8 scode, c;
    // volatile char *t, *tt;
    int i;
    KBD* kp = &kbd;
    color = YELLOW;
    scode = *(kp->base + KDATA);
    code = scode;
    //if (scode & 0x80)
    //return;

    //c = unsh[scode];
    /*
  if (c == 'r')
     kputc('\n');
  kputc(c);
  */
    /*********
  kprintf("kbd interrupt: c=");
  if (c != '\r')
    kprintf("%x %c\n", c, c); 
  else
    kprints("0x 0D <cr>\n\r");
  ****************
  kp->buf[kp->head++] = c;
  kp->head %= 128;
  kp->data++; kp->room--;*/
    if (scode == 0xf0) {
        isRelease = 1;
        // kprintf("Return %d\n",isRelease);
        return;
    }

    if (scode == 0x12) {
        // printf("I am shift");

        if (isShift == 1) {
            isShift = 0;
        } else {
            isShift = 1;
        }
            return;
    }
    if (isShift) {
        c = sh[scode];
    } else {
        c = unsh[scode];
    }
    // c = unsh[scode];
    // if (c >= 'a' && c <= 'z')
    //if (c != '\r' && isRelease == 0) {
    //    kprintf(c);
    //}

    // kprintf("isRelease Before Loop %d\n",isRelease);

    if (scode != 0x12 && isRelease == 0) {
        //kwakeup(&kbd);
        kp->buf[kp->head++] = c;
        kp->head %= 128;
        kp->data++;
        kp->room--;
        //kwakeup(&kp->data);
    } else {
        isRelease = 0;
        // kprintf("isRelease After else %d\n",isRelease);
    }
}

int kputc(char); // kputc() in vid.c driver

int kgetc()
{
    char c;
    KBD* kp = &kbd;
    while (kp->data <= 0)
    {
        //ksleep(&kp->data);
    }
    c = kp->buf[kp->tail++];
    kp->tail %= 128;

    // updating variables: must disable interrupts
    int_off();
    kp->data--;
    kp->room++;
    int_on();
    return c;
}

int kgets(char s[])
{
    char c;
    while ((c = kgetc()) != '\r') {
        kputc(c);
        *s++ = c;
    }
    *s = 0;
    return strlen(s);
}
