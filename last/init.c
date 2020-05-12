#include "ucode.c"
int console2, console3, console4;


int parent3() // P1's code
{
    int pid, status;
    while(1)
    {
        printf("INIT : wait for ZOMBIE child\n");
        pid = wait(&status); // if no ZOMBIE child => P1 sleep
        
        if (pid == console2) // P2
        {
            // if console login process died
            printf("INIT: forks a new console login\n");
            console2 = fork(); // fork another one
            if (console2)
                continue;
            else
                exec("login /dev/tty0"); // new console login process

            printf("INIT: I just buried an orphan child proc %d\n", pid);
        }
        if (pid == console3) // P3
        {
            // if console login process died
            printf("INIT: forks a new console login\n");
            console3 = fork(); // fork another one
            if (console3)
                continue;
            else
                exec("login /dev/ttyS0"); // new console login process

            printf("INIT: I just buried an orphan child proc %d\n", pid);
        }
        if (pid == console4) // P4
        {
            // if console login process died
            printf("INIT: forks a new console login\n");
            console4 = fork(); // fork another one
            if (console4)
                continue;
            else
                exec("login /dev/ttyS1"); // new console login process

            printf("INIT: I just buried an orphan child proc %d\n", pid);
        }
    }
}

int main()
{
    int in1, in2, in3, out1, out2, out3; // file descriptors for terminal I/O
    
    //in = open("/dev/tty0", O_RDONLY); // file descriptor 0
    //out = open("/dev/tty0", O_WRONLY); // for display to console
    
    printf("INIT : fork a login proc on console\n");


    console2 = fork(); // P1 fork P2
    if (console2) 
        tswitch(); // tswitch back to P1 to fork P3
    else 
        exec("login /dev/tty0"); // exec to login on tty0


    console3 = fork(); // P1 fork P3
    if (console3) 
        tswitch(); // tswitch back to P1 to fork P4
    else 
        exec("login /dev/ttyS0"); // exec to login on ttyS0

    
    console4 = fork(); // P1 fork P4
    if (console4) 
        parent3(); // execute parent3 function
    else 
        exec("login /dev/ttyS1"); // exec to login on ttyS1
}