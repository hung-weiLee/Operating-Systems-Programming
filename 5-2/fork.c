/********************************************************************
Copyright 2010-2017 K.C. Wang, <kwang@eecs.wsu.edu>
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
********************************************************************/

int body(), goUmode();

char buf[BLKSIZE];
char buf1[BLKSIZE], buf2[BLKSIZE]; // (buffer)!!!

char *addr;
u16  iblk;
INODE *ip; /* INODE */
u16 inumber;

int k = 0;
int search(INODE *ip, char *name)
{
   int i; 
   char c, *cp;
   DIR  *dp; 
   for (i=0; i<12; i++)
   {
       if ( ip->i_block[i] ) // i_block[i] has store the data block number !!! 
       {
          printf("i_block[%d] = %d\n", i, ip->i_block[i]);
	        
          // put data block(data block number is store in i_block[i]) into buf2 !!!
	        getblk(ip->i_block[i], buf2); 
          cp = buf2;
          dp = (DIR *)buf2;

          while (cp < buf2 + BLKSIZE)
          {
              c = dp->name[dp->name_len];  // save last byte
              dp->name[dp->name_len] = 0;   
	            printf(" %s |", dp->name); 
              
              if ( strcmp(dp->name, name) == 0 )
              {
		             printf(" , (FOUND %s) !!!\n", name); 
                 return (dp->inode);
              }
              
              dp->name[dp->name_len] = c; // restore that last byte
              cp += dp->rec_len; // point to next dir struct
              dp = (DIR *)cp;
	        }
       }
   }
   return 0;
}

/**************************************************/

int loader(char *filename, PROC *p)
{
    inumber = search(ip, filename) - 1; // count from 1 , (1-1 = 0) , inumber = (u1's inode number)

    getblk(iblk + (inumber/8), buf1); // get block , put into buf1
    
    ip = (INODE *)buf1 + (inumber % 8);

    for(int j=0; j<12; j++)
    {
      getblk(ip->i_block[j], buf); // put u1 inode's i_block into buf
      memcpy(addr, buf, 1024);
      addr += 1024;
    }
}

/**************************************************/

PROC *kfork(char *filename)
{
  int i; 
  char *cp, *cq;
  char line[8];
  int usize1, usize;
  int *ustacktop, *usp;
  u32 BA, Btop, Busp;

  PROC *p = dequeue(&freeList);
  if (p==0){
    kprintf("kfork failed\n");
    return (PROC *)0;
  }
  p->ppid = running->pid;
  p->parent = running;
  p->parent = running;
  p->status = READY;
  p->priority = 1;
  
  // set kstack to resume to body
  for (i=1; i<29; i++)  // all 28 cells = 0
    p->kstack[SSIZE-i] = 0;

  p->kstack[SSIZE-15] = (int)goUmode;  // in dec reg=address ORDER !!!
  p->ksp = &(p->kstack[SSIZE-28]);

  // kstack must contain a resume frame FOLLOWed by a goUmode frame
  //  ksp  
  //  -|-----------------------------------------
  //  r0 r1 r2 r3 r4 r5 r6 r7 r8 r9 r10 fp ip pc|
  //  -------------------------------------------
  //  28 27 26 25 24 23 22 21 20 19 18  17 16 15
  //  
  //   usp      NOTE: r0 is NOT saved in svc_entry()
  // -|-----goUmode--------------------------------
  //  r0 r1 r2 r3 r4 r5 r6 r7 r8 r9 r10 ufp uip upc|
  //-------------------------------------------------
  //  14 13 12 11 10 9  8  7  6  5  4   3    2   1
  /********************

  // to go Umode, must set new PROC's Umode cpsr to Umode=10000
  // this was done in ts.s dring init the mode stacks ==> 
  // user mode's cspr was set to IF=00, mode=10000  

  ***********************/

  /* must load filename to Umode image area at 7MB+(pid-1)*1MB  !!! */
  addr = (char *)(0x800000 + (p->pid - 1)*0x100000);
  
  // loadelf(filename, p); // p->PROC containing pid, pgdir, etc
  
  //u16  iblk;
  u32   *up;
  GD    *gp;
  //INODE *ip; /* INODE */
  INODE *tp;
  DIR   *dp;
  //u16 inumber; 

  SUPER *sp;
  getblk(1, buf); // get block1 => super_block
  sp = (SUPER *)buf;
  printf("magic_number = %x \n",sp->s_magic); // magic_number


  getblk(2, buf); // put block2 into buf
  gp = (GD*)buf; // gp point to block2
  iblk =(u16*)gp->bg_inode_table; //bg_inode_table = 8
  printf("iblk = %d \n", iblk);
  
  getblk(iblk, buf1); //put block5 into buf1  
  ip = (INODE*)buf1; // ip point to block5 
  ip = ip + 1; /* get root Inode , (ip => first Inode => nothing) , (ip + 1 => second Inode => root Inode)  !!! */
  printf("ip = %d \n", ip);


  inumber = search(ip ,"bin") - 1; 
  getblk(iblk + (inumber/8),buf1); // get block , put into buf1
  ip = (INODE *)buf1 + (inumber % 8); // (mailman algorithn) , get (bin's inode) , ip point to bin's inode


  if (strcmp(filename, "u1") == 0)
  { 
    loader("u1", p);
  }
  
  if (strcmp(filename, "u2") == 0)
  {
    loader("u2", p);
  }
  
  if (strcmp(filename, "u3") == 0)
  {
    loader("u3", p);
  }
  
  if (strcmp(filename, "u4") == 0)
  {
    loader("u4", p);
  }

     
  cq = addr;
  for (i=0; i<usize; i++)
  {
      *cq++ = *cp++;
  }

  // must fix Umode ustack for it to goUmode: how did the PROC come to Kmode?
  // by swi # from VA=0 in Umode => at that time all CPU regs are 0
  // we are in Kmode, p's ustack is at its Uimage (8mb+(pid-1)*1mb) high end
  // from PROC's point of view, it's a VA at 1MB (from its VA=0)
  // but we in Kmode must access p's Ustack directly

  /***** this sets each proc's ustack differently, thinking each in 8MB+
  ustacktop = (int *)(0x800000+(p->pid)*0x100000 + 0x100000);
  TRY to set it to OFFSET 1MB in its section; regardless of pid
  **********************************************************************/
  //p->usp = (int *)(0x80100000);
  p->usp = (int *)VA(0x100000);

  //  p->kstack[SSIZE-1] = (int)0x80000000;
  p->kstack[SSIZE-1] = VA(0);
  // -|-----goUmode-------------------------------------------------
  //  r0 r1 r2 r3 r4 r5 r6 r7 r8 r9 r10 ufp uip upc|string       |
  //----------------------------------------------------------------
  //  14 13 12 11 10 9  8  7  6  5  4   3    2   1 |             |

  enqueue(&readyQueue, p);

  kprintf("proc %d kforked a child %d: ", running->pid, p->pid); 
  printQ(readyQueue);

  return p;
}
