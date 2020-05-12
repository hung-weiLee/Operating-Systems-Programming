#include "ucode.c"

int pid, pd[2];
char line[64];


int main(int argc, char* argv[])
{
    int status;

    while (1)
    {
        printf(" ( Hung-Wei's  program !!! )\n *( sh )* : ");
        gets(line);

        pid = fork();


        if(strcmp(line, "logout") == 0)
        {
          exit(0); // syscall to die
        }

        if (pid)
        {
			      // parent sh waits for child to die
            pid = wait(&status);
		    }   
        else
        {
            char f1[64], f2[64];
            
            int i = 0, j = 0, k = 0;
            int flag = 0;

            /* cut (char array line) into (left char array f1) and (right char array f2), which is cut by (pipe) !!!  */
            while(line[i] != 0)
            {
              if( line[i] == '|') // '|' ==> pipe
              {
                  if(flag == 0) flag = 1;
                  else flag = 0;
                  
                  i = i + 2; // skip f2's first char, which is a blank !!!
              }
              else
              {
                  if(flag == 0)
                  {
                    f1[j] = line[i];
                    j++;
                    i++;
                  }
                  else // flag == 1
                  { 
                    f2[k] = line[i];
                    k++;
                    i++;
                  }
              }
            }
            
            f1[j] = '\0';
            f2[k] = '\0';
           
            printf("j = %d \n", j);
            printf("k = %d \n", k);
            printf("flag = %d \n", flag);
            printf("%s ", f1);
            printf("%s ", f2);
            //getc();


            if(flag == 1) // have pipe
            {
                /***************** do_pipe Algorithm **************/

                pipe(pd); // create a pipe: pd[0]=READ, pd[1]=WRITE
                pid = fork(); // fork a child to share the pipe

                if (pid) // parent: as pipe READER
                {
                  close(pd[1]); // close pipe WRITE end
                  dup2(pd[0], 0); 
                  exec(f2);
                }
                else // child : as pipe WRITER
                {
                  close(pd[0]); // close pipe READ end
                  dup2(pd[1], 1); // redirect stdout to pipe WRITE end
                  exec(f1);
                }
            }
            else // no pipe
            {
                exec(f1);
            }
	      }     
    }
}

