/*******************************************************
*                  @t.c file                          *
*******************************************************/

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned long  u32;

#define TRK 18
#define CYL 36
#define BLK 1024

#include "ext2.h"
typedef struct ext2_group_desc  GD;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR;

int prints(char *s)
{
    while(*s != 0)
    {
      putc(*s++); //print cahr , point to next char
    }
}

int gets(char *s)
{
  while((*s=getc()) != 0x0D) // 0x0D ==> '\r' ==> press Enter
  {
    putc(*s++); //print cahr , point to next char
  }

  *s = 0;
}


u16 NSEC = 2;
char buf1[BLK], buf2[BLK]; // (buffer)!!!


u16 getblk(u16 blk, char *buf)
{
  readfd( (2*blk)/CYL, ( (2*blk)%CYL)/TRK, ((2*blk)%CYL)%TRK, buf);

  // readfd( blk/18, ((blk)%18)/9, ( ((blk)%18)%9)<<1, buf);
}


u16 search(INODE *ip, char *name)
{
  // search for name in the data block of INODE; 
  // return its inumber if found
  // else error();
	char c; int i; DIR *dp;

    for(i=0 ; i<12 ;i++)
    { 
      if((u16)ip->i_block[i]) // i_block[i] have data
      { 
        /* put data block(which is point by i_block[i]) into buf2  !!! */
        getblk((u16)(ip->i_block[i]), buf2);
            
        dp = (DIR *)buf2;
        
        while((char *)dp < &buf2[BLK]) // dp won't exceed buf2 memory space
        { 
          c = dp->name[dp->name_len]; // for example: root => dp->name_len = 4
                
          dp->name[dp->name_len] = 0; // name[4] = 0
                
          prints(dp->name);
          putc(' ');  
                   
          if(strcmp(dp->name, name) == 0) // find "boot"
          {
            if(i == 0)
            {
              /** (boot) and (mtx) are all in the (data block) which is point by i_block[0]  !!! **/
              prints(" iblock[0]   "); 
            }

            return ((u16)dp->inode); // return (boot's inode number) 
          }
                
          dp->name[dp->name_len] = c; //restore byte
                
          dp = (char *)dp + dp->rec_len; // point to next dir struct
        } 
      } 
    }
    error();
}


main()
{ 
    u16 i=0,iblk;
    u16 inumber;

    u32   *up;
    GD    *gp;
    INODE *ip; /* INODE */
    INODE *tp;
    DIR   *dp;
//    char inode;
//1. Write YOUR C code to get the INODE of /boot/mtx
//   INODE *ip --> INOD

    getblk(2, buf1); // put block2 into buf1
    
    gp = (GD*)buf1; // gp point to block2
    
    iblk =(u16*)gp->bg_inode_table; //bg_inode_table = 5
    
    getblk(iblk, buf1); //put block5 into buf1 , (iblk = 5) !!!
    
    ip = (INODE*)buf1; // ip point to block5
    
    /* get root Inode , (ip => first Inode => nothing) , (ip + 1 => second Inode => (/)root Inode)  !!! */
    ip = ip + 1;
    
    inumber = search(ip ,"boot") - 1; // count from 1 , (1-1 = 0) , inumber = (boot's inode number)

    getblk(iblk + (inumber/8),buf1); // get block , put into buf1
    
    ip = (INODE *)buf1 + (inumber % 8); // (mailman algorithn) , get (boot's inode) , ip point to boot's inode

 /*--------------------*/

    inumber = search(ip ,"mtx") - 1; // count from 1 , (1-1 = 0) , inumber = (mtx's inode number)
   
    getblk(iblk + (inumber/8),buf1); // get block , put into buf1
    
    ip = (INODE *)buf1 + (inumber % 8); // (mailman algorithn) , get mtx's inode , ip point to mtx's inode



//2. if INODE has indirect blocks: get i_block[12] int buf2
    if((u16)ip->i_block[12])
    {
      getblk((u16)ip->i_block[12],buf2); // put i_block[12] into buf2
    }
   

    setes(0x1000);  // MTX loading segment = 0x1000


//3. load mtx's 12 DIRECT blocks of INODE into memory
    for (i=0; i<12; i++) // load mtx's 12 i_bolck into 0x1000
    {
      getblk((u16)ip->i_block[i], 0); // ES = 0
      putc('*');
      inces();
    }
    

//4. load INDIRECT blocks, if any, into memory
   if (tp->i_block[12])
   {
     up = (u32 *)buf2;      
     while(*up)
     {
       getblk((u16)*up, 0); 
       putc('.');
       inces();
       up++;
     }
   }
  
  prints("go?"); getc();
}  
