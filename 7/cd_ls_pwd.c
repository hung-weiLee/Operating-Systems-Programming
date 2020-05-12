char b1[BLKSIZE], b2[BLKSIZE], b3[BLKSIZE];
extern char gmyname[64];
extern int nn;
extern char *name[64];
int ls(char *pathname)
{
	
	u16 mode;
    int mask, k;
	int blk,offset;
    INODE *ip;
    u32 ino = getino(pathname);
	//printf("ino = %d\n",ino);
    blk = (ino-1) / 8 + inode_start;
    offset = (ino-1) % 8;
    get_block(dev, blk, b1);
    ip = (INODE *)b1 + offset;
	mode = ip->i_mode;
	if ((mode & 0xF000) != 0x4000)
    {
		printf("NOT DIR!\n");
		return 0;
	}
	//printf("ip->i_size=%d\n",ip->i_size);
    ls_all(ip);

}
int ls_all(INODE *ip)
{
   
   int i; INODE *tp;
   char c, *cp;
   DIR  *dp; 
   for (i=0; i<12; i++){
       if ( ip->i_block[i] ){
         //printf("i_block[%d] = %d\n", i, ip->i_block[i]);
	      get_block(dev,ip->i_block[i], b2);
          cp = b2;
          dp = (DIR *)b2;

          while (cp < b2 + BLKSIZE){
              c = dp->name[dp->name_len];  // save last byte
              dp->name[dp->name_len] = 0;
              get_block(fd,iblk+((dp->inode -1) / 8), b3);      // read block inode of me 
      		  tp = (INODE *)b3 + ((dp->inode -1) % 8);
              ls_ip(tp,dp->name);
              dp->name[dp->name_len] = c; // restore that last byte
              cp += dp->rec_len;
			  
              dp = (DIR *)cp;
	}
     }
   }
   return 0;
}
int ls_ip(INODE *ip, char *name)
  {
    u16 mode;
    int mask, k;
    
    mode = ip->i_mode;
    if ((mode & 0xF000) == 0x4000)
        putchar('d');

    if ((mode & 0xF000) == 0xA000)
        putchar('l');
    else if ((mode & 0xF000) == 0x8000)
        putchar('-');

    mask = 000400;
    for (k=0; k<3; k++){
      if (mode & mask)
         putchar('r');
      else
         putchar('-');
      mask = mask >> 1;
      if (mode & mask)
         putchar('w');
      else
         putchar('-');
      mask = mask >> 1;
      if (mode & mask)
         putchar('x');
      else
         putchar('-');
      mask = mask >> 1;
    }    
    printf(" %d ", ip->i_links_count);
    printf(" %d  %d ", ip->i_uid, ip->i_gid);
    printf("%d ", ip->i_size);
    printf("%s ", ctime((long *)&ip->i_mtime));
    printf("%s ", name);
    printf("\n");     
}

int pwd(MINODE *wd){
	//printf("%s\n","pwd");
	
	int blk,offset;
    INODE *ip;
    u32 ino = wd->ino;
	if(ino == 2)
	{
		printf("/\n");
		return 0;
	}
	else
	{
		wd = iget(dev,ino);
		rpwd(wd);
	}
	
    //if (wd == root) print "/"
	
	
     // else
     //    rpwd(wd);
}

int rpwd(MINODE *wd)
{
	char rpwdname[64];
	int blk,offset;
    INODE *ip;
    u32 ino = wd->ino;
	MINODE *pip;
	if(ino == 2)
	{
		return 0;
	}
	else
	{
		
		pip = iget(dev, findino(wd));
		findmyname(pip, rpwdname);
		rpwd(pip);
		printf("/%s", rpwdname);
	}
     //if (wd==root) return;
     //from i_block[0] of wd->INODE: get my_ino of . parent_ino of ..
     //pip = iget(dev, parent_ino);
     //from pip->INODE.i_block[0]: get my_name string as LOCAL

     //rpwd(pip);  // recursive call rpwd() with pip

     //print "/%s", my_name;
}
/*

int ls_dir(dirname){
      
      ino = getino(dirname);
      mip = iget(dev, ino); ===> mip points at dirname's minode
                                                         INODE  
                                                         other fields
      Get INODE.i_block[0] into a buf[ ];
      Step through each dir_entry (skip . and .. if you want)
      For each dir_entry: you have its ino.
      call ls_file(ino)
}
int ls_file(ino)
{

      mip = iget(dev, ino); ==> mip points at minode
                                              INODE
      Use INODE contents to ls it 

}
*/

int chdir(char *pathname)
{
	MINODE *mip;
	u16 mode;
    int mask, k;
	int blk,offset;
    INODE *ip;
	u32 ino;
	int myino;
	char myname[32];
      //if (no pathname)
      //   cd to root;
      //else
      //   cd to pathname by
      //(1).  ino = getino(pathname);
      //(2).  mip = iget(dev, ino);
      //(3).  verify mip->INODE is a DIR
      //(4).  iput(running->cwd);
      //(5).  running->cwd = mip;
	//printf("%s\n","chdir");
	if(pathname[0] == 0)
	{

	mip = iget(dev,2);
	running->cwd = mip;
	pwd(running->cwd);
	return 0;
	}
	tokenize(pathname);
	if(nn == 1)
	{
		//strcpy(myname,name[0]);
		//printf("xxx %d  \n",strcmp("..", myname));
		ino = search(running->cwd,name[0]);
		if(ino == 0)
		{
			printf("cannot find %s \n", name[0]);
			return 0;
		}
		blk = (ino-1) / 8 + inode_start;
    	offset = (ino-1) % 8;
    	get_block(dev, blk, b1);
    	ip = (INODE *)b1 + offset;
		mode = ip->i_mode;
		if ((mode & 0xF000) != 0x4000)
    	{
		printf("NOT DIR!\n");
		return 0;
		}

		mip = iget(dev,ino);
		iput(running->cwd);
		running->cwd = mip;
		pwd(running->cwd);
		printf("\n");
		return 0;
	}
	
    ino = getino(pathname);
	//printf("ino = %d\n",ino);
	if(ino == 0)
	{
		printf("cannot find %s \n", pathname);
		return 0;
	}
    blk = (ino-1) / 8 + inode_start;
    offset = (ino-1) % 8;
    get_block(dev, blk, b1);
    ip = (INODE *)b1 + offset;
	mode = ip->i_mode;
	if ((mode & 0xF000) != 0x4000)
    {
		printf("NOT DIR!\n");
		return 0;
	}

	mip = iget(dev,ino);
	iput(running->cwd);
	running->cwd = mip;
	pwd(running->cwd);
	printf("\n");
}





