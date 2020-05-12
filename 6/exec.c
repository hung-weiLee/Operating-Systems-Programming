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

int exec(char *cmdline) // cmdline=VA in Uspace
{
  int i, upa, usp; 
  char *cp, kline[128], file[32], filename[32]; 
  PROC *p = running;
 
  printf("EXEC: proc %d cmdline=%x\n", running->pid, cmdline);

  kstrcpy(kline, cmdline);
  printf("EXEC: proc %d kline = %s\n", running->pid, kline); 

  // first token of kline = filename
  cp = kline; i=0;
  while(*cp != ' '){
    filename[i] = *cp;
    i++; cp++;
    if (*cp == 0)
    {
		break;
	}
  } 
  filename[i] = 0;

  if (filename[0] != '/'){  // relative to /bin
     kstrcpy(file, "/bin/");
  }
  kstrcat(file, filename);

  upa = p->pgdir[2048] & 0xFFFF0000; // PA of Umode image
  kprintf("load file %s to %x\n", file, upa);

  // load filename to Umode image 
  load(file, p);
  printf("after loading ");

  // copy kline to high end of Ustack in Umode image
  
  usp = upa + 0x100000 - 128;
  printf("usp=%x ", usp);
  kstrcpy((char *)usp, kline);

  p->usp = (int *)VA(0x100000 - 128);
  printf("p->usp = %x ", p->usp);

  // set up syscall kstack frame to return to new image at VA=0
  //   usp             
  // --|-----goUmode--------------------------------
  //  r0 r1 r2 r3 r4 r5 r6 r7 r8 r9 r10 r11 r12 ULR|
  //-------------------------------------------------
  //  14 13 12 11 10 9  8  7  6  5  4   3    2   1
  for (i=2; i<14; i++)  // clear Umode regs r1-r12
      p->kstack[SSIZE - i ] = 0;
  p->kstack[SSIZE-14] = (int)VA(p->usp);
  p->kstack[SSIZE-1]  = (int)VA(0);

  kprintf("kexec exit\n");
  return (int)p->usp; // will replace saved r0 in kstack
}
