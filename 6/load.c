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
//extern char *disk;  // disk base pointer defined and set in t.c file

#define InodePerBlock BLKSIZE/sizeof(INODE)

char buf[BLKSIZE], b1[BLKSIZE], b2[BLKSIZE];

char path[64];
int nn;
char *name[32];  // at most 32 component names

int bmap, imap, iblk;

int tokenize(char *path)
{
  int i;
  char *cp;
  cp = path;
  nn = 0;
  
  while (*cp != 0){
       while (*cp == '/') *cp++ = 0;        
       if (*cp != 0)
           name[nn++] = cp;         
       while (*cp != '/' && *cp != 0) cp++;                  
       if (*cp != 0)   
           *cp = 0;                   
       else 
            break; 
       cp++;
  }
  
  printf("n = %d : ", nn);
  for (i=0; i<nn; i++)
       printf("  %s  ", name[i]);
  printf("\n");
}

int search(INODE *ip, char *name)
{
   int i; 
   char c, *cp;
   DIR  *dp; 
   for (i=0; i<12; i++)
   {
       if ( ip->i_block[i] )
       {
          //printf("i_block[%d] = %d\n", i, ip->i_block[i]);
	      getblk(ip->i_block[i], b2);
          cp = b2;
          dp = (DIR *)b2;

          while (cp < b2 + BLKSIZE)
          {
              c = dp->name[dp->name_len];  // save last byte
              dp->name[dp->name_len] = 0;   
	            //printf("%s ", dp->name); 
              if ( strcmp(dp->name, name) == 0 )
              {
		             //printf("FOUND %s\n", name); 
                 return(dp->inode);
              }
              dp->name[dp->name_len] = c; // restore that last byte
              cp += dp->rec_len;
              dp = (DIR *)cp;
          }
       }
   }

   return 0;
}

int load(char *filename, PROC *p)
{ 
  int i;

  char *addr;
  int me;

  SUPER *sp;
  GD    *gp;
  INODE *ip;
  DIR   *dp;
  char  *cp, c;
   
  //printf("proc %x loading %s disk at %x\n", p->pid, filename, disk);
  color = CYAN;

  //printf("proc%d load for proc%d ", running->pid, p->pid);
  strcpy(path,filename);

  //printf("loader : tokenlize pathname\n");
  tokenize(path);     // break up filename into nn string pointers
                     // in name[i]
  //printf("read INODE table block\n");
  getblk(iblk, buf);
  ip = (INODE *)buf + 1;

  /* serach for system name */
  for (i=0; i<nn; i++)
  {
      me = search(ip, name[i]);
      if (me == 0)
      {
          printf("can't find %s\n", name[i]); 
          return(0);
      }
      me--;
      getblk(iblk+(me / 8), b1);      /* read block inode of me */
      ip = (INODE *)b1 + (me % 8);
   }

  addr = (char *)(p->pgdir[2048] & 0xFFFF0000);
  //addr = (char *)(0x800000 + (p->pid - 1)*0x200000);
  printf("loading address = %x\n", addr);
  
  for (i=0; i<12; i++)
  {
    if (ip->i_block[i] == 0)
      break;
    //printf("loading block %d ", ip->i_block[i]);
    getblk(ip->i_block[i], addr);
    addr += BLKSIZE;
  }
  printf("loader done\n");
  return 1;
}

