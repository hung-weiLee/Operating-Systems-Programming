#define NPIPE 9
#define PSIZE 8
#define FREE  0
#define BUSY  1

typedef struct pipe{
  char buf[PSIZE]; 	// circular data buffer
  int head, tail;	// circular buf index
  int data, room; 	// number of data & room in pipe
  int status; 		// FREE or BUSY
}PIPE;

PIPE pipe[NPIPE]; 	// global PIPE objects

//---------------Init pipe struct---------------
int pipe_init()
{
  printf("pipe_init()\n");
  int i;
  for(i=0; i<NPIPE; i++){
      pipe[i].status = FREE;
  }
}

//----------------creat pipe--------------------
PIPE *create_pipe()
{
  PIPE *temp;
  temp = &pipe[0];
  temp->head = 0; 
  temp->tail = 0;
  temp->data = 0;
  temp->room = PSIZE;
  temp->status = BUSY;
  return temp;
}

//---------- Algorithm of pipe_read-------------
int read_pipe(PIPE *p, char *buf, int n)
{
  printf("go in to read pipe\n");
  printf("reader print n: %d\n", n);
  int r = 0;
  if (n <= 0)
      return 0;
  //validate PIPE pointer p; 		// p->status must not be FREE
  if(p->status == FREE) 
      return -1;
  while(n)
  {
      while(p->data)
      {
          *buf++ = p->buf[p->tail++]; 	// read a byte to buf
           p->tail %= PSIZE;
           p->data--; p->room++; r++; n--;
           if (n == 0) break;
      }
      kwakeup(&p->room);	 	// wakeup writers(process 1)
      if (r) 				// if has read some data
          return r;			// pipe has no data
      ksleep(&p->data);			// sleep for data
  }
}

//---------- Algorithm of write_pipe -----------
int write_pipe(PIPE *p, char *buf, int n)
{
  int r = 0;
  if (n <= 0)
      return 0;
  //validate PIPE pointer p; 		// p->status must not be FREE
  if(p->status == FREE) 
      return -1;
  while(n)
  {
      printf("writer print n: %d\n", n);
      while(p->room)
      {
          p->buf[p->head++] = *buf++; 	// write a byte to pipe;
          p->head %= PSIZE;
          p->data++; p->room--; r++; n--;
          if (n == 0) 
              break;
       }
       kwakeup(&p->data);		// wakeup readers, if any.
       if (n == 0) 
       {
           tswitch();
           return r;
        }		// finished writing n bytes
       // still has data to write but pipe has no room
       ksleep(&p->room);		// sleep for room
  }
}
