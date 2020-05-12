#include "ucode.c"

int main(int argc, char* argv[])
{
    int sh, pid, status;
    sh = ufork(); // (produce P2), ufork() function in ucode.c !!!
    uprintf("sh = %d\n", sh); // first time: sh = 2
                             // second time: sh = 0
    ugetc();

    if (sh)
    {
        // P1 runs in a while(1) loop
        while (1)
        {
            // wait for ANY child to die
            pid = wait(&status); // function wait() is in ucode.c !!!
            uprintf("pid = %d !!!!!", pid);
            if (pid == sh) 
            {
                // if sh died, fork another one
                sh = ufork();
                continue;
            }
            uprintf("P1: I just buried an orphan %d\n", pid);
        }
    }
    else
	{
		uprintf("666666\n");
		exec("sh"); // P2 runs sh
	}
    // child of P1 runs sh
}
