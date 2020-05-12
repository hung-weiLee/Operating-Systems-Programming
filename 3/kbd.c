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

#include "keymap"
#define KCNTL 0x00
#define KSTAT 0x04
#define KDATA 0x08
#define KCLK 0x0C
#define KISTA 0x10

typedef volatile struct kbd //keyboard
{
    char* base;
    char buf[128];
    int head, tail, data, room;
} KBD;

u16 isShift = 0, flag = 0; /* flag = 0 (press button) , flag = 1 (button up) !!! */

volatile KBD kbd;

int kbd_init()
{
    KBD* kp = &kbd;
    kp->base = (char*)0x10006000;
    *(kp->base + KCNTL) = 0x14;
    *(kp->base + KCLK) = 8;
    kp->head = kp->tail = 0;
    kp->data = 0;
    kp->room = 128;
}


/* (press button) => excute 1 time kbd_handler() , (button up) => excute 2 times kbd_handler()  !!! */
void kbd_handler()
{
    u8 scode, c;
    KBD* kp = &kbd;
    color = YELLOW;
    scode = *(kp->base + KDATA); // get scan code
    //printf(scode); // print scan code
    

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

    kp->buf[kp->head++] = c; // put cahr into (donut buffer) , and then (pointer head)++
    kp->head %= 128; // where is the head location 
    kp->data++;
    kp->room--;
}


int kgetc() // get buffer's cahr out 
{
    char c;
    KBD* kp = &kbd;

    unlock();
    
    //printf("cahr = %c\n", kp->data); /*  (testing the code excution sequence) !  */
    
    /* (data == 0 , do nothing) ==> (didn't type any char , wait here) !!! */
    while (kp->data == 0)  /*  tyrpe any char(interrupt) => excute IRQ_handler() !!! */
        ;

    lock();
    c = kp->buf[kp->tail++]; // get (donut buffer)'s cahr into c , and then (pointer tail)++
    kp->tail %= 128; // where is the tail location
    kp->data--;
    kp->room++;
    unlock();
    return c;
}


int kgets(char s[])
{
    char c;
    while ((c = kgetc()) != '\r') // '\r' ==> (press Enter) , finsh while lo0p , return string
    {
        *s = c;
        //printf("cahr = %c\n", *s);
        s++;
    }
    
    *s = 0;
    return strlen(s);
}
