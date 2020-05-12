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

/********************
typedef struct proc{
  struct proc *next;
  int    *ksp;
  int    status;
  int    pid;
  int    priority;
  int    ppid;
  int    event;
  int    exitCode;
  
  struct proc *parent;
  struct proc *child;
  struct proc *sibling;

  int    kstack[SSIZE];
}PROC;
***************************/
#define NPROC 9
PROC proc[NPROC], *running, *freeList, *readyQueue;
int procsize = sizeof(PROC);
int body();

int block(SEMAPHORE *s)
{
	running->status = BLOCK;
	enqueue(&s->queue, running);
	tswitch();
}

int signal(SEMAPHORE *s)
{
	PROC *p = dequeue(&s->queue);
	p->status = READY;
	enqueue(&readyQueue, p);
}

int P(SEMAPHORE *s)
{
	int SR = int_off();
    //printf("value = %d",s->value);
	s->value--;
	if (s->value < 0)
		block(s);
	int_on(SR);
}

int V(SEMAPHORE *s)
{
	int SR = int_off();
	s->value++;
	if (s->value <= 0)
		signal(s);
	int_on(SR);
}

int init()
{
  int i, j; 
  PROC *p;
  kprintf("kernel_init()\n");
  for (i=0; i<NPROC; i++){
    p = &proc[i];
    p->pid = i;
    p->status = READY;
    p->next = p + 1;
  }
  proc[NPROC-1].next = 0; // circular proc list
  freeList = &proc[0];
  readyQueue = 0;

  printf("create P0 as initial running process\n");
  p = dequeue(&freeList);
  p->priority = 0;
  p->ppid = 0; p->parent = p;  // P0's parent is itself
  running = p;
  kprintf("running = %d\n", running->pid);
  printList("freeList", freeList);
}

void ksleep(int event)
{ 
  int SR = int_off();
  running->event = event;
  running->status = SLEEP;
  tswitch();
  int_on(SR);
}

int kwakeup(int event)
{
  int SR = int_off();
  PROC *p;
  int i;
  for(i=1; i<NPROC; i++)
  {
      p = &proc[i];
      if(p->status == SLEEP && p->event == event)
      {
          p->event = 0;
          p->status == READY;
          enqueue(&readyQueue, p);
      }
  }
  int_on(SR);
}

/*
void kexit()
{
  printf("proc %d kexit\n", running->pid);
  running->status = FREE;
  running->priority = 0;
  enqueue(&freeList, running);   // putproc(running);
  tswitch();
}
*/

int kexit()
{
  int i, wakeupP1 = 0;
  char s[128];
  printf("please input exitValue:");
  kgets(s);
  int exitValue = geti(s); 
  if(running->pid == 1){
      printf("P1 never dies!!!\n\r");
      return -1;
  }
  if(running->child){
      PROC *p1,*temp;
      p1 = &proc[1];
      temp = running->child;
      if(!(p1->child)){
          p1->child = running->child;
      }
      else{
          PROC *temp1;
          temp1 = p1->child;
          while(temp1->sibling) temp1 = temp1->sibling;
          temp1->sibling = temp;
      }
      printProcInfo(p1->child);
      while(temp){
          temp->ppid = 1;
          temp->parent = p1;
          wakeupP1++;
          temp = temp->sibling;
      }
  }
  running->exitCode = exitValue;
  running->status = ZOMBIE;
  kwakeup(running->parent);
  if(wakeupP1) 
      kwakeup(&proc[1]);
  tswitch();
}

/*
int kexit()
{
  int i, wakeupP1 = 0;
  char s[128];
  printf("please input exitValue:");
  kgets(s);
  int exitValue = geti(s); 
  if (running->pid==1){ 			// nproc = number of active PROCs
      printf("p1 never dies!!! \n\r");
      return -1;
  }
  // send children (dead or alive) to P1's orphanage 
  for (i = 1; i < NPROC; i++){
      PROC *p;
      p = &proc[i];
      if (p->status != FREE && p->ppid == running->pid){
          p->ppid = 1;
          p->parent = &proc[1];
          wakeupP1++;
      }
  }
  // record exitValue and become a ZOMBIE 
  running->exitCode = exitValue;
  running->status = ZOMBIE;
  // wakeup parent and also P1 if necessary 
  kwakeup(running->parent); 			// parent sleeps on its PROC address
  if (wakeupP1)
      kwakeup(&proc[1]);
  tswitch();				
}
*/
/*
int kwait(int *status)
{  
  if(!(running->child)){
      printf("No more child!!");
      return -1;
  }
  if(running->child->status == ZOMBIE){ // child is ZOMBIE
      status = running->child->exitCode;
      //delete
      if(running->sibling)
          running->child == running->child->sibling;
      else
          running->child = 0;
      //release
      running->child->status = FREE;
      enqueue(&freeList, running->child);
      //return pid
      return running->child->pid;
  }
  PROC *temp = running->child;
  while(temp->sibling->sibling){
      if(temp->sibling->status == ZOMBIE){ // middle sibling is ZOMBIE
          status = temp->sibling->exitCode;
          PROC *temp1 = temp->sibling;
          PROC *temp2 = temp->sibling->sibling;
          temp->sibling = temp2;
          temp1->status = FREE;
          enqueue(&freeList, temp1);
          return temp1->pid;
      }
      else 
          temp = temp->sibling;
  }
  if(temp->sibling->status == ZOMBIE){
      status = temp->sibling->exitCode;
      PROC *temp1 = temp->sibling; 
      temp->sibling = 0;
      temp1->status = FREE;
      enqueue(&freeList, temp1);
      return temp1->pid;
  }
  else
      ksleep(running);
}
*/


int kwait(int *status) // wait for ZOMBIE child
{
  PROC *p; int i, hasChild = 0;
  while(1){ 					// search PROCs for a child
      for (i=1; i<NPROC; i++){ 			// exclude P0
          p = &proc[i];
          if (p->status != FREE && p->ppid == running->pid){
              hasChild = 1; 			// has child flag
              if (p->status == ZOMBIE){ 	// lay the dead child to rest
                  printf("Zombei pid %d\n", p->pid);
                  *status = p->exitCode; 	// collect its exitCode
                  //delete p
                  PROC *temp;
                  temp = p->parent;
                  if(temp->child == p)
                      temp->child = p->sibling;
            	  else{
                      temp = temp->child;
                      while(temp->sibling != p)temp = temp->sibling;
                      temp->sibling = p->sibling;
                  }
                  p->status = FREE; 		// free its PROC
                  enqueue(&freeList, p); 	// to freeList
                  return(p->pid); 		// return its pid
              }
          }
      }
      if (!hasChild) return -1; 		// no child, return ERROR
      ksleep(running);          		// still has kids alive: sleep on PROC address
  }
}


PROC *kfork(int func, int priority)
{
  int i;
  PROC *p = dequeue(&freeList);
  if (p==0){
    printf("no more PROC, kfork failed\n");
    return 0;
  }
  p->status = READY;
  p->priority = priority;
  p->ppid = running->pid;
  p->parent = running;//pointer parent
  //child pointer
  if(!(running->child)){
      running->child = p;
      p->sibling = 0;
  }  
  else{
      PROC* temp = running->child;
      while(temp->sibling)
          temp = temp->sibling;
      temp->sibling = p;
      p->sibling = 0;
      p->child = 0;
  }	
  
  // set kstack to resume to body
  // stack = r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,r14
  //         1  2  3  4  5  6  7  8  9  10 11  12  13  14
  for (i=1; i<15; i++)
      p->kstack[SSIZE-i] = 0;
  p->kstack[SSIZE-1] = (int)func;  // in dec reg=address ORDER !!!
  p->ksp = &(p->kstack[SSIZE-14]);

  p->kstack[SSIZE-14] = p->pid;
  p->kstack[SSIZE-13] = p->ppid;
  p->kstack[SSIZE-12] = (int)func;
  p->kstack[SSIZE-11] = p->priority;
 
  enqueue(&readyQueue, p);
  printf("%d kforked a child %d\n", running->pid, p->pid);
  kprintf("proc info: pid %d, ppid %d, func %d, priority %d\n\r", running->pid, running->ppid, func, running->priority);
  printf("child:");
  printProcInfo(running->child); 
  printList("readyQueue", readyQueue);
  return p;
}

int scheduler()
{
  kprintf("proc %d in scheduler ", running->pid);
  if (running->status == READY)
      enqueue(&readyQueue, running);
  running = dequeue(&readyQueue);
  kprintf("next running = %d\n", running->pid);
}  

int body(int pid, int ppid, int func, int priority)
{
  char c; char line[64];
  int status;
  kprintf("proc %d resume to body()\n", running->pid);
  while(1){
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
    
    printList("readyQueue", readyQueue);
    kprintf("proc info: pid %d, ppid %d, func %d, priority %d\n\r", pid, ppid, func, priority);
    printf("child:");
    printProcInfo(running->child); 
    kprintf("\n\rinput a char [s|f|q|w] : ");
    c = kgetc();
    printf("%c\n", c);

    switch(c){
      case 's': tswitch();           break;
      case 'f': kfork((int)body, 1); break;
      case 'q': kexit();             break;
      case 'w': pid = kwait(&status);
                if(pid == -1)printf("erroe no child!!!\n\n");      
                break;
    }
  }
}
