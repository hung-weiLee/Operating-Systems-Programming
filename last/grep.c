#include "ucode.c"

char buf[24800];
char buff[24800];
char cmp[24800];


int do_grep(int fd, char key[]) // input argv[2] into key !!!
{
    int n, k;
      
    printf("fd = %d\n", fd);

    if(fd < 0)
        exit(0);

    n = read(fd, buf, 24800); // read file into buf
    //printf("%s", buf); 
    
    int i = 0;
    
    while(buf[i] != 0)
    {
        k = 0;
        while(buf[i] != '\n') // read line
        {
            buff[k] = buf[i];
            i++;
            k++;
        }
        
        buff[k] = '\n'; k++;
        buff[k] = 0;          

    //--------------------//
        
        char *str1 = buff;
        char *str2 = key; // key => argv[2] => (printf) !!!
        
        if(strstr(str1, str2) != 0) // find substring !!!
        {
            printf("%s", buff);
        }

    //--------------------//
        i++;
    }
}


int do_grep2(int fd, char key[]) // input argv[2] into key !!!
{
    int n, k = 0;    

    if(fd < 0)
        exit(0);

    int i = 0;
    while(1)
    {
        k = 0;
        while(1)
        {    
            n = read(fd, buf, 24800); // read only one char into buf

            if(n <= 0)
              return 0;
            
            buff[k] = buf[0]; // put buf[0] which is only one char into buff[k]

            k++;

            if(buf[0] == '\n')  
              break;
        }

        buff[k] = 0;          
        //printf("%s", buff);

    //--------------------//

        char *str1 = buff;
        char *str2 = key; // key => argv[2] => (printf) !!!
        
        if(strstr(buff, key) != 0) // find string which have key => (printf) !!!
        {
            printf("%s", buff);
        }
    }
}

int main(int argc, char *argv[ ])
{
    int i;
    int fd, n, k = 0, j = 0;
    int pid = getpid();

    printf("argc = %d\n", argc);
    for (i=0; i<argc; i++)
        printf("argv[%d] = %s\n", i, argv[i]);
    
    
    if(argc == 2) // grep printf, (have pipe) !!!
    {
        do_grep2(0, argv[1]);
    }
    else // (argc == 3)  =>  grep f printf, (no pipe) !!!
    {
        fd = open(argv[1], O_RDONLY);
        do_grep(fd, argv[2]);
    }
}