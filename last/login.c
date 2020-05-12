// login.c : Upon entry, argv[0]=login, argv[1]=/dev/ttyX
#include "ucode.c"

int in, out, err;
char name1[128],password[128];

char path[64];
int nn;
char *name2[32];  // at most 32 component names


int tokenize(char *path)
{
  int i;
  char *cp;
  cp = path;
  nn = 0;
  
  while (*cp != 0)
  {
       while (*cp == ':') *cp++ = 0;        
       if (*cp != 0)
           name2[nn++] = cp;         
       while (*cp != ':' && *cp != 0) cp++;                  
       if (*cp != 0)   
           *cp = 0;                   
       else 
            break; 
       cp++;
  }
  
  printf("n = %d : ", nn);
  for (i=0; i<nn; i++)
       printf("  %s  ", name2[i]);
  mputc('\n');
}


int main(int argc, char *argv[])
{
    //close file descriptors 0,1 inherited from INIT.
    close(0);
    close(1);
    
    //open argv[1] 3 times as in(0), out(1), err(2).
    in = open(argv[1], 0);
    out = open(argv[1], 1);
    err = open(argv[1], 2);
    
    settty(argv[1]); // set tty name string in PROC.tty
    
    //open /etc/passwd file for READ;
    int fd = open("/etc/passwd", O_RDONLY);
    char buf[512];
    read(fd, buf, 512);
    
    //printf(" %s", buf);    
    //getc();
    
    char n1[128], n2[128], n3[128], n4[128];

    while(1)
    {
        printf("login:");
        gets(name1);
        printf("password:"); 
        gets(password);

        int i = 0, k = 0;
        while(buf[i] != '\n') // first line in buf
        {
            n1[k] = buf[i];
            i++;
            k++;
        }
        //printf(" %s \n\n", n1);
            
        i++; k = 0;
        while(buf[i] != '\n') // second line in buf
        {
            n2[k] = buf[i];
            i++;
            k++;
        }
        //printf(" %s \n\n", n2);

        i++; k = 0;
        while(buf[i] != '\n') // third line in buf
        {
            n3[k] = buf[i];
            i++;
            k++;
        }
        //printf(" %s \n\n", n3);

        i++; k = 0;
        while(buf[i] != 0) // fourth line in buf
        {
            n4[k] = buf[i];
            i++;
            k++;
        }
        //printf(" %s \n\n", n4);    
            
//------------------------------------------//
        
        tokenize(n1);
        if(strcmp(name2[0], name1) == 0  &&  strcmp(name2[1], password) == 0)
        {
            chuid(name2[3], name2[2]);
            chdir(name2[5]);
            close(fd);
            exec("sh");
        }
        else
        {
            printf("login failed, try again\n");
        }

        tokenize(n2);
        if(strcmp(name2[0], name1) == 0  &&  strcmp(name2[1], password) == 0)
        {
            chuid(name2[3], name2[2]);
            chdir(name2[5]);
            close(fd);
            exec("sh");
        }
        else
        {
            printf("login failed, try again\n");
        }

        tokenize(n3);
        if(strcmp(name2[0], name1) == 0  &&  strcmp(name2[1], password) == 0)
        {
            chuid(name2[3], name2[2]);
            chdir(name2[5]);
            close(fd);
            exec("sh");
        }
        else
        {
            printf("login failed, try again\n");
        }

        tokenize(n4);
        if(strcmp(name2[0], name1) == 0  &&  strcmp(name2[1], password) == 0)
        {
            chuid(name2[3], name2[2]);
            chdir(name2[5]);
            close(fd);
            exec("sh");
        }
        else
        {
            printf("login failed, try again\n");
        }
    }
}

