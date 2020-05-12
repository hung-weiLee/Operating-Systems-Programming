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

/*    #define  SSIZE 1024
      #define  NPROC  9
      #define  FREE   0
      #define  READY  1
      #define  SLEEP  2
      #define  BLOCK  3
      #define  ZOMBIE 4
      #define  printf  kprintf
    
      typedef struct proc{
        struct proc *next;
        int    *ksp;
        int    status;
        int    pid;

        int    priority;
        int    ppid;
        struct proc *parent;
        int    event;
        int    exitCode;
        int    kstack[SSIZE];
      }PROC;                   */

#define NPROC 9 
PROC proc[NPROC], *running, *freeList, *readyQueue; // (pointer)

int procsize = sizeof(PROC);
int body();

/***********************************************************/

int init()
{
  int i, j; 
  PROC *p;
  kprintf("kernel_init()\n");
  for (i = 0; i < NPROC; i++) // 0 ~ 8 process
  {
    p = &proc[i];
    p->pid = i;
    p->status = READY;
    p->next = p + 1;
  }
  
  proc[NPROC - 1].next = 0; // circular proc list (0 ==> null)
  
  freeList = &proc[0]; // freeList point to P0
  
  readyQueue = 0; // null pointer

  printf("create P0 as initial running process\n");
  p = dequeue(&freeList); /* pop P0 out from freeList , (p piont to P0) */
  
  p->priority = 0;
  p->ppid = 0; // ppid ==> parent's pid , P0 ppid = 0(itself)
  p->parent = p;  // P0's parent is itself
  
  running = p; // running process is P0(running pointer point to P0)
  kprintf("running = %d\n", running->pid);
  printList("freeList", freeList);
}

/***********************************************************/

PROC *kfork(int func, int priority) /* pop freeList's frist one , and than enqueue it into readyQueue  !!! */
{
  int i;
  PROC *p = dequeue(&freeList); // pop freeList's frist one 
  if (p == 0) // null
  {
    printf("no more PROC, kfork failed\n");
    return 0;
  }
  p->status = READY;
  p->priority = priority;
  p->ppid = running->pid;
  p->parent = running; /*   !!!   */
  
  // set kstack to resume to body
  // stack = r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,r14
  //         1  2  3  4  5  6  7  8  9  10 11  12  13  14
  for (i=1; i<15; i++)
      p->kstack[SSIZE-i] = 0; // clean stack

  p->kstack[SSIZE-14] = p->pid;
  p->kstack[SSIZE-13] = p->ppid;
  p->kstack[SSIZE-12] = func;
  p->kstack[SSIZE-11] = priority;

  p->kstack[SSIZE-1] = (int)func;  // in dec reg=address ORDER !!!
  p->ksp = &(p->kstack[SSIZE-14]);

  enqueue(&readyQueue, p); // enqueue freeList's frist one into readyQueue
  
  printf("%d kforked a child %d\n", running->pid, p->pid);
  printList("readyQueue", readyQueue);

 /*--------p's parent is running--------*/

  /* p is the process which is going to become a child of running(parent)  !!! */
  if(running->child == 0) // process's parent is first time fork
  {
    running->child = p;
  }
  else
  {
    PROC *h_sib = running->child; // running->child is the child_List's first one

    while(1) 
    {
      if(h_sib->sibling == 0) // search the child_List to the last one !
      {
        h_sib->sibling = p; // let process become the child_List's last one !
        break;
      }

      h_sib = h_sib->sibling;
    }
  }
  
  print_child_List("child_List", running->child);
  return p;
}

/***********************************************************/

int scheduler() /* enqueue running process into readyQueue , pop readyQueue's first one , and than running point to it !!! */
{
  kprintf("proc %d in scheduler ", running->pid);

  /* (only the status is READY , can put running process into readyQueue)  !!! */
  if (running->status == READY)
      enqueue(&readyQueue, running); 
  
  running = dequeue(&readyQueue); // pop readyQueue's first one
  
  kprintf("next running = %d\n", running->pid);
}  

/***********************************************************/

int ksleep(int event)
{
  //kprintf("(no ZOMBIE child , sleep process: %d)\n", running->pid);
  int SR = int_off(); // disable IRQ and return CPSR
  
  running->event = event; // store the address of the process
  running->status = SLEEP;
  /* switch process , but it will not enqueue into readyQueue , because only the status is READY can enqueue */
  tswitch(); 
  
  int_on(SR); // restore original CPSR
}
int kwakeup(int event)
{
  int SR = int_off(); // disable IRQ and return CPSR
  
  int i;
  PROC *p;

  for(i=1; i< NPROC; i++)
  {
    if (p->status == SLEEP && p->event == event)
    {
      event = 0;
      p->status = READY;
      enqueue(&readyQueue, p); // status become READY , put into readyQueue
    }
  }

  int_on(SR); // restore original CPSR
}

/***********************************************************/

void kexit(int exitCode) /* (running process become ZOMBIE)  !!! */
{
  //printf("exitCode = %d", exitCode);
  printf("\nproc %d kexit\n", running->pid);

  // process 1 never die ==> P1's status cannot be ZOMBIE !!!
  if( running->pid != 1)
  {
    running->exitCode = exitCode;
    running->status = ZOMBIE; /*  !!!  */

    PROC *p1 = &proc[1]; // p1 point to process 1
    if(p1->status == SLEEP)
    {
      kwakeup(p1); // wakeup process 1 => (p1->status = READY)
    }

    PROC *last = p1->child;
    while(last->sibling != 0)
    {
      last = last->sibling; // (last) is the process 1's child_List's last one
    }

    if(running->child != 0)
    {
      last->sibling = running->child; // move the ZOMBIE's child_List to process 1's child_List

      running->child = 0; // null
    }

    /* wake up ZOMBIE's parent  !!! */
    if(running->parent->status == SLEEP)
    {
      kwakeup(running->parent);
    }

    /* ZOMBIE process would not put in readyQueue , because only the status is (READY) can enqueue into readyQueue !!! */
    tswitch();
  }
}

/***********************************************************/

int enfreeList(PROC *k) /* k is ZOMBIE , put ZOMBIE process back to freeList !!! */
{
  k->sibling = 0;
  
  PROC *t = freeList;
  if(t == 0) // freeList is null
  {
    freeList = k;
    k->next = 0;
  }
  else
  {
    while(t->next != 0)
    {
      t = t->next;
    }
    t->next = k; // add k to the freeList's last on
    k->next = 0;
  }
}
int kwait(int *status)
{
  if(running->child == 0)
  {
    return -1; // no child
  }
  else
  {
    PROC *head = running->child; // (pointer head) is the (head) of the running's child_List !

    // running only have one child
    if(head->sibling == 0)
    {
      if(head->status == ZOMBIE)
      {
        *status = head->exitCode;
        running->child = 0;
        enfreeList(head);
        return head->pid;  
      }
    }
    // running has more than one child
    else
    {
      if(head->status == ZOMBIE) // (delete head)
      {
        *status = head->exitCode;
        running->child = head->sibling;
        enfreeList(head);
        return head->pid;  
      }
      else // (delete middle, tail)
      {
        PROC *k = head;
        while(k->sibling != 0)
        {
          if(k->sibling->status == ZOMBIE) // check (k->sibling) is ZOMBIE or not !!!
          {
            PROC *z = k->sibling; // pointer z point to the ZOMBIE process
            k->sibling = k->sibling->sibling;
            *status = z->exitCode;
            enfreeList(z);
            return z->pid;
          }
          else
          {
            k = k->sibling;
          }
        }

        ksleep(running); // no ZOMBIE child in the running's child_List !!!
      }
    }
  }
}

/***********************************************************/

int body(int pid1, int ppid, int func, int priority)
{
  printf("%d %d %x %d \n",pid1,ppid,func,priority);
  
  char c; char line[64];
  int pid;
  kprintf("proc %d resume to body()\n", running->pid);
  while(1)
  {
    pid = running->pid;
    if (pid==0) color=BLUE;
    if (pid==1) color=WHITE;
    if (pid==2) color=GREEN;
    if (pid==3) color=CYAN;
    if (pid==4) color=YELLOW;
    if (pid==5) color=WHITE;
    if (pid==6) color=GREEN;   
    if (pid==7) color=WHITE;
    if (pid==8) color=CYAN;
    print_child_List("child_List", running->child);
    printList("readyQueue", readyQueue);
    kprintf("proc %d running, parent = %d  ", running->pid, running->ppid);
    kprintf("input a char [s|f|q|w] : ");
    c = kgetc(); 
    printf("%c\n", c);
   
    char arr[100]; // store the exit code 
    int i = 0, status1, pid1;
    
    switch(c)
    {
      case 's': tswitch();           break; /* (call body function , call scheduler function)  !!! */
      
      case 'f': kfork((int)body, 1); break; // fork , priority is always 1
      
      case 'q': kprintf("Enter exit code: "); c = kgetc();
                while(c != '\r')
                {
                  kprintf("%c", c);
                  arr[i] = c;
                  c = kgetc();
                  i++;
                }
                arr[i] = '\r';
                int sum = geti(arr);
                kexit(sum);            break; // let running process become ZOMBIE
      
      case 'w': pid1 = kwait(&status1); // if running's child_List have ZOMBIE process , than delete ZOMBIE , let ZOMBIE back to freeList
                if(pid1 == -1) { kprintf("no child , exit code: %d\n", status1); }     
                else { kprintf(" ZOMBIE child pid : %d , exit code: %d\n", pid1, status1); }
                break;
    }
  }
}
