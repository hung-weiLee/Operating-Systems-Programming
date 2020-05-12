#include "ucode.c"

char buf[24800];
char buff[24800];


int main(int argc, char *argv[ ])
{
  int i;
  int fd, n, k = 0;
  int pid = getpid();

  printf("argc = %d\n", argc);
  for (i=0; i<argc; i++)
    printf("argv[%d] = %s\n", i, argv[i]);

  if(argc == 1) // cat
  {
      while(1)
      {
          read(0, buf, 24800);
          mputc(buf[0]);

          buff[k] = buf[0];

          if (buff[k] == ' ')
          {
              printf("\n%s ", buff);
              
              for(int j=0; j<=1000; j++)
              {
                buff[j] = 0; // clear buffer
              }
              
              k = 0;
          }
          else
          {
              k++;
          }
      }
  }
  else // (argc == 2)  =>  cat f
  {
      fd = open(argv[1], O_RDONLY);
      printf("fd = %d\n", fd);

      if(fd < 0)
        exit(0);

      n = read(fd, buf, 24800);
      
      int i = 0;
      while(buf[i] != 0)
      {
          k = 0;
          while(buf[i] != '\n')
          {
              buff[k] = buf[i];
              i++;
              k++;
          }
          buff[k] = '\n'; k++;
          buff[k] = 0;
          
          printf("%s", buff);
          i++;
      }
  }  
}