#include "param.h"
#include "types.h"
#include "defs.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"

struct {
  struct spinlock lock;
  struct shm_page {
    uint id;
    char *frame;
    int refcnt;
  } shm_pages[64];
} shm_table;

void shminit() {
  int i;
  initlock(&(shm_table.lock), "SHM lock");
  acquire(&(shm_table.lock));
  for (i = 0; i< 64; i++) {
    shm_table.shm_pages[i].id =0;
    shm_table.shm_pages[i].frame =0;
    shm_table.shm_pages[i].refcnt =0;
  }
  release(&(shm_table.lock));
}

int shm_open(int id, char **pointer) {

//you write this
//So we id is given, this pointer is given because once we finish our job,
//we want to return this pointer, and this will be pointing to the id's virtual address
   //int va;
   int tablefound=0;
   acquire(&(shm_table.lock));
   uint i;
   for(i=0; i<64; i++)
   {
	if(shm_table.shm_pages[i].id==id)
	{
	   struct proc* curproc=myproc();
	   uint virtualaddress=PGROUNDUP(curproc->sz);
	   mappages(curproc->pgdir,(void*)virtualaddress,PGSIZE,V2P(shm_table.shm_pages[i].frame),PTE_W|PTE_U);
	   shm_table.shm_pages[i].refcnt++;
	   curproc->sz+=PGSIZE;
	   *pointer=(char*)virtualaddress;
	   tablefound=1;
	}
/*	else
	{
	   shm_table.shmpages[i].id=id;
	   shm_table.shm_pages[i].frame=kalloc();
	   shm_table.shm_pages[i].refcnt=1;
	   
	}
*/
   }
   if(tablefound==0)
   {
   	for(i=0;i<64;i++)
   	{
		if(shm_table.shm_pages[i].id==0)	//An empty page has been found! or unallocated
		{
	   	   shm_table.shm_pages[i].id=id;		//Set id of unallocated to our new one
           	   shm_table.shm_pages[i].frame=kalloc();	//kalloc is a function that allocates 4096 byte page, or one page
           	   shm_table.shm_pages[i].refcnt=1;	//This has been used once!
           	   memset(shm_table.shm_pages[i].frame,0,PGSIZE);
		   struct proc* curproc=myproc();
           	   uint virtualaddress=PGROUNDUP(curproc->sz);
           	   mappages(curproc->pgdir,(void*)virtualaddress,PGSIZE,V2P(shm_table.shm_pages[i].frame),PTE_W|PTE_U);
           	   shm_table.shm_pages[i].refcnt++;
           	   curproc->sz+=PGSIZE;
		   *pointer=(char*)virtualaddress;
		}
   	}
   }
   release(&(shm_table.lock));
   return 0; //added to remove compiler warning -- you should decide what to return
}


int shm_close(int id) {		//we call close under open, it tells us that their 
				//using that id, and it just decrements that id's refcnt
//you write this too!
   acquire(&(shm_table.lock));
   uint i;
   for(i=0;i<64;i++)
   {
	if(shm_table.shm_pages[i].id==id)
        {
	   shm_table.shm_pages[i].refcnt--;
        }
	if(shm_table.shm_pages[i].refcnt==0)
	{
	   shm_table.shm_pages[i].id=0;
	   shm_table.shm_pages[i].frame=0;
	   shm_table.shm_pages[i].refcnt=0;
	}
    }   
    release(&(shm_table.lock));
return 0; //added to remove compiler warning -- you should decide what to return
}
