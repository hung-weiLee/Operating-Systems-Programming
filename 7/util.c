/*********************************************************************
        You MAY use the util.o file to get a quick start.
 BUT you must write YOUR own util.c file in the final project
 The following are helps of how to wrtie functions in the util.c file
*********************************************************************/


/************** util.c file ****************/
//#include "type.h"

/**** globals defined in main.c file ****/

extern MINODE minode[NMINODE];
extern MINODE *root;
extern PROC   proc[NPROC], *running;
extern char gpath[256];
extern char *name[64];
extern int n;
extern int fd, dev;
extern int nblocks, ninodes, bmap, imap, inode_start,iblk;
extern char line[256], cmd[32], pathname[256];
char buf1[BLKSIZE], buf2[BLKSIZE], buf3[BLKSIZE];
int nn;
char *buf4;
char buf5[BLKSIZE];
extern u32 myino;
extern char gmyname[64];
//TEST OK
int get_block(int dev, int blk, char *buf)
{
  lseek(fd, (long)blk*BLKSIZE, 0);
  read(fd, buf, BLKSIZE);

}   
//TEST OK
int put_block(int dev, int blk, char *buf)
{
  lseek(fd, (long)blk*BLKSIZE, 0);
  write(fd, buf, BLKSIZE);

}   
//TEST OK
int tokenize(char *path)
{
  int i;
  char *cp;
  nn = 0;
  strcpy(gpath,path);
  cp = gpath;
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
  
  //printf("n = %d : ", nn);
  //for (i=0; i<nn; i++)
  //     printf("  %s  ", name[i]);
  //printf("\n");
}
//TEST OK
MINODE *iget(int dev, int ino)
{
  
  MINODE *mip, *tmip;
  INODE *ip;
  int blk,offset;
  int i;
  // return minode pointer to loaded INODE
  /*(1). Search minode[ ] for an existing entry with the needed (dev, ino):
       if found: inc its refCount by 1;
                 return pointer to this minode;*/
  for (i=0; i<NMINODE; i++) {
    tmip = &minode[i];
    if (tmip->ino == ino) {
       tmip->refCount++;
       blk = (ino-1) / 8 + inode_start;
       offset = (ino-1) % 8;
       //printf("iget:dev=%d ino=%d blk=%d offset=%d\n", dev, ino, blk, offset);
       get_block(dev, blk, buf1);
       ip = (INODE *)buf1 + offset;
       //printf("mip->refCount = %d\n",mip->refCount);
       tmip->INODE = ip;  // copy INODE to mp->INODE
      return tmip;
    }
  }

  /*(2). // needed entry not in memory:
       find a FREE minode (refCount = 0); Let mip-> to this minode;
       set its refCount = 1;
       set its dev, ino*/
  for (i=0; i<NMINODE; i++) {
    tmip = &minode[i];
    if (tmip->refCount == 0) {
      mip = tmip;
      mip->refCount = 1;
      mip->dev = dev;
      mip->ino = ino;
      mip->INODE = 0;
      break;
    }
  }

// (3). load INODE of (dev, ino) into mip->INOiDE:
       
    // get INODE of ino a char buf[BLKSIZE]    
       blk    = (ino-1) / 8 + inode_start;
       offset = (ino-1) % 8;

       printf("iget:dev=%d ino=%d blk=%d offset=%d\n", dev, ino, blk, offset);

       get_block(dev, blk, buf1);
       ip = (INODE *)buf1 + offset;
       //printf("mip->refCount = %d\n",mip->refCount);
       mip->INODE = ip;  // copy INODE to mp->INODE
	   //memcpy((char *)mip->INODE,(char *)ip,sizeof((char *)ip));

       return mip;
}

//TEST OK
int iput(MINODE *mip) // dispose a used minode by mip
{
	int ino,blk;
 mip->refCount--;
 
 if (mip->refCount > 0) return;
 if (!mip->dirty)       return;
 blk    = (ino-1) / 8 + inode_start;
 buf4 = (char*)(mip->INODE);
 
 put_block(dev,blk,buf4);
 // YOUR C CODE: write INODE back to disk
 return 0;
} 

//TEST OK
// serach a DIRectory INODE for entry with given name
int search(MINODE *mip, char *name)
{
   int i;
   char c, *cp;
   DIR  *dp;
   int ino = mip->ino;
   //INODE *ip = mip->INODE;
   int blk,offset;
   INODE *ip;
   //printf("before search ino=%d name=%s\n",ino,name);
   get_block(dev,iblk + ((ino-1) / 8),buf5);
   ip = (INODE*)buf5 + ((ino-1)%8);

   for (i=0; i<12; i++){
       if ( ip->i_block[i] ){
         //printf("i_block[%d] = %d\n", i, ip->i_block[i]);
	      get_block(fd,ip->i_block[i], buf2);
          cp = buf2;
          dp = (DIR *)buf2;

          while (cp < buf2 + BLKSIZE){
              c = dp->name[dp->name_len];  // save last byte
              dp->name[dp->name_len] = 0;
	          //printf("%s \n", dp->name); 
              if ( strcmp(dp->name, name) == 0 ){
					//printf("FOUND %s \n", dp->name); 
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

int search2(INODE *ip, char *name)
{
   int i; INODE *tp;
   char c, *cp;
   DIR  *dp; 

   for (i=0; i<12; i++){
       if ( ip->i_block[i] ){
         //printf("i_block[%d] = %d\n", i, ip->i_block[i]);
	      get_block(fd,ip->i_block[i], buf2);
          cp = buf2;
          dp = (DIR *)buf2;

          while (cp < buf2 + BLKSIZE){
              c = dp->name[dp->name_len];  // save last byte
              dp->name[dp->name_len] = 0;
	          //printf("%s \n", dp->name); 
              if ( strcmp(dp->name, name) == 0 ){
					//printf("FOUND %s \n", dp->name); 
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
// retrun inode number of pathname
//TEST OK
int getino(char *pathname)
{ 
	int i =0;int ino=0;
   int blk,offset;
    INODE *ip;
  //u16 iblk, ino;
   // SAME as LAB6 program: just return the inode's ino;
  strcpy(gpath,pathname);
  tokenize(gpath);     // break up filename
  //get_block(dev, 2 ,buf1);
  //gp = (GD*)buf1;
  //iblk = gp->bg_inode_table;
  //get_block(dev, iblk, buf2);
  //ip = (INODE*)buf2+1;
  ino = 2;
  blk = (ino-1) / 8 + inode_start;
  offset = (ino-1) % 8;
  get_block(dev, blk, buf1);
  ip = (INODE *)buf1 + offset;
  for (i=0;i<nn;i++)
	{
			//printf("before search ino=%d name=%s\n",ino,name[i]);
		    ino = search2(ip,name[i]);
			//ino = search(iget(dev,ino),name[i]);
 			//printf("search ino=%d\n",ino);
			if(ino == 0)
			{
				printf("cannot find %s \n", name[i]);
				return 0;
			}
			if(i == nn-1)
			{
				return ino;
			}
			ino--;
            get_block(dev,iblk + (ino / 8),buf2);
			ip = (INODE*)buf2 + (ino%8);
	}
    ino = running->cwd->ino;
	return ino;
}

//TEST OK
int findmyname(MINODE *parent, char *myname) 
{
	
   
    
   int i;
   char c, *cp;
   DIR  *dp;
   INODE *ip = parent->INODE;

   for (i=0; i<12; i++){
       if ( ip->i_block[i] ){
         //printf("i_block[%d] = %d\n", i, ip->i_block[i]);

          get_block(dev,ip->i_block[i], buf2);
          cp = buf2;
          dp = (DIR *)buf2;

          while (cp < buf2 + BLKSIZE){
              c = dp->name[dp->name_len];  // save last byte
              dp->name[dp->name_len] = 0;
              //printf("%s \n", dp->name);

              if ( myino == dp->inode ){
                 //printf("FOUND %s\n", dp->name);
                 strcpy(myname,dp->name);
				 //printf("/%s\n", dp->name);
				 return 0;
                 //return(dp->inode);
              }

              dp->name[dp->name_len] = c; // restore that last byte
              cp += dp->rec_len;
              dp = (DIR *)cp;
        }
     }
   }
   return 0;

   // return name string of myino: SAME as search except by myino;
   // copy entry name (string) into myname[ ];
}

//TEST OK
int findino(MINODE *mip) 
{
	myino = search(mip,".");
	return search(mip,"..");
  // fill myino with ino of . 
  // retrun ino of ..
}

