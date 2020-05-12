typedef unsigned char   u8;
typedef unsigned short  u16;
typedef unsigned int    u32;

#include "string.c"
#include "uio.c"
int usleep()
{
  int pid = getpid();
  uprintf("proc %d go to sleep in kernel\n", pid);
  return syscall(11,pid,0,0);
}

int getpid()
{
	return syscall(0,0,0,0);
}

int uwakeup()
{
  int pid;
  uprintf("enter a pid to wakeup: ");
  pid = geti();
  printf("\npid=%d\n", pid);
  return syscall(12,pid,0,0);
}

int getppid()
{ 
  return syscall(1,0,0,0);
}

int umenu()
{
  uprintf("------------------------------------------\n");
  uprintf("ps chname kfork switch wait exit fork exec sleep wakeup\n");
  uprintf("------------------------------------------\n");
}

int ups()
{
  return syscall(2,0,0,0);
}

int uchname()
{
  char s[32];
  uprintf("input a name string : ");
  ugetline(s);
  printf("\n");
  return syscall(3,s,0,0);
}

//--------------------------------------------------------------------------//
int ukfork(char *filename)
{
  int pid;
  pid = syscall(13,filename,0,0);
  uprintf("kforked child = %d\n", pid);
}
//--------------------------------------------------------------------------//

int uswitch()
{
  syscall(5,0,0,0);
}

//--------------------------------------------------------------------------//
int wait(int *status)
{
  return syscall(6,status,0,0); // kkwait((int *)b)
}
//--------------------------------------------------------------------------//

int uwait()
{
  int pid, status;
  uprintf("%d syscall6 to wait for ZOMBIE child\n", getpid());
  pid = syscall(6,&status,0,0);
  uprintf("%d waited for a ZOMBIE child=%d ", getpid(), pid);
  if (pid>0)
    uprintf("status=%x %d", status, status);
  uprintf("\n");

  return pid;
}

int myexit(int value)
{
  syscall(7, value, 0,0);
}

//--------------------------------------------------------------------------//
int uexit()
{
  int value;
  uprintf("input an exit value : ");
  printf("\n");
  value = geti();
  syscall(7,value,0,0);
}
//--------------------------------------------------------------------------//

int ugetusp()
{
  return syscall(8,0,0,0);
}

int fork()
{
  return syscall(9,0,0,0);
}

int exec(char *line)
{
  return syscall(10,line,0,0);
}

//--------------------------------------------------------------------------//
int ufork()
{
  int pid, mypid;
  //mypid = getpid();
  // can we find out the return PC here?
  pid = syscall(9,0,0,0); // fork()
  
  if (pid>0)
  {
    uprintf("parent %d forked a child %d\n", getpid(), pid);
  }
  if (pid==0)
  {
    uprintf("child %d return from fork(), pid=%d\n", getpid(), pid);
  }  
  if (pid < 0)
  {
    uprintf("%d fork failed\n", getpid());
  }

  return pid;
}
//--------------------------------------------------------------------------//

int uexec()
{
  int r, mypid; char line[64];
  mypid = getpid();
  uprintf("enter a command string : ");
  ugetline(line);
  uprintf("\nline=%s\n", line);
  r = syscall(10,line,0,0);
  if (r<0)
     uprintf("%d exec failed\n", mypid);
}

int ugetc()
{
  return syscall(90,0,0,0);
}

int uputc(char c)
{
  return syscall(91,c,0,0);
}

int argc;
char *argv[32];
 
int token(char *line)
{
  int i;
  char *cp;
  cp = line;
  argc = 0;
  
  while (*cp != 0){
       while (*cp == ' ') *cp++ = 0;        
       if (*cp != 0)
           argv[argc++] = cp;         
       while (*cp != ' ' && *cp != 0) cp++;                  
       if (*cp != 0)   
           *cp = 0;                   
       else 
            break; 
       cp++;
  }
  argv[argc] = 0;
}


main0(char *s)
{
  uprintf("main0: s = %s\n", s);
  token(s);
  main(argc, argv);
  uexit();
}

int getPA()
{
  return syscall(92,0,0,0);
}
