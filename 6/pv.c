
#define enterCR  ps=int_off()
#define exitCR   int_on(ps)

extern PROC *readyQueue;

typedef struct semaphore{
  int value;
  struct proc *queue;
}SEMAPHORE;

void P(struct semaphore *s)
{
  int ps;

  ps = int_off();

   s->value--;
   if (s->value < 0){
     //printf("P: block %d\n", running->pid); 
      running->status = BLOCK;
      //running->sem = s;       /* PROC's sem pointer->this semaphore */
      enqueue(&s->queue, running);
      //int_on();
      tswitch();
   }
   int_on(ps);
}

void V(struct semaphore *s)
{
  PROC *p; int ps;
   
    ps=int_off();
    s->value++;
    if (s->value <= 0){
        p = (PROC *)dequeue(&s->queue);
        p->status = READY;
        enqueue(&readyQueue, p);
	//printf("V: unblock %d\n", p->pid);
        //schedule(p);
    }
    int_on(ps);
}
