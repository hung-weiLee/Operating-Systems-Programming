// queue.c file
int kprintf(char *fmt,...);

extern PROC *freeList;

/* enter p into *queue by priority; PROCs with the same priority by FIFO */

/*   (*queue is the head pointer) ==>   *queue point to the readyQueue's first one  !!!  */
/*                 queue pointer receive readyQueue pointer's address   */
int enqueue(PROC **queue, PROC *p)
{
  int SR = int_off();  // IRQ interrupts off, return CPSR
  
  // (readyQueue don't have process)
  if(*queue == 0)
  {
    *queue = p; // point to process
  }
  // (readyQueue has process)
  else
  {
    if( p->priority > (*queue)->priority ) /* p->priority (bigger) than head priority */
    {
      // add head
      p->next = *queue;
      *queue = p;
    }
    else /* p->priority (smaller) than head priority   or   (same as) head priority */
    {
      PROC *k = *queue; // k point to the head of readyQueue
      
      while(k->next != 0)
      {
        if(p->priority > k->next->priority) // p compare pripority with k->next
        {
          p->next = k->next;
          k->next = p;
          
          int_on(SR); //  restore CPSR
          return;
        }
        else
        {
          k = k->next;
        } 
      }
      
      k->next = p; // add last
    }
  }
  int_on(SR); //  restore CPSR 
}				     


PROC *dequeue(PROC **queue) /* queue pointer receive freeList pointer's address */
{
  int SR = int_off();  // IRQ interrupts off, return CPSR
    
  // remove the FISRT element from *queue;
  PROC *k = *queue;
  (*queue) = (*queue)->next;
  k->next = 0; /* (null) !!! */
  
  /*kprintf("queue = %p\n", queue);
  kprintf("*queue = %p\n", *queue);
  kprintf("k = %p\n", k);*/

  int_on(SR); //  restore CPSR 
  return k;  // return pointer to dequeued PROC;
}

int printList(char *name, PROC *p)
{
   kprintf("%s = ", name);
   while(p){
     kprintf("[%d%d]->", p->pid, p->priority);
     p = p->next;
  }
  kprintf("NULL\n"); 
}

int print_child_List(char *name, PROC *p)
{
   kprintf("%s = ", name);
   while(p){
     kprintf("[%d%d]->", p->pid, p->priority);
     p = p->sibling;
  }
  kprintf("NULL\n"); 
}
