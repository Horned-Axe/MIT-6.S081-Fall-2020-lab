// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

/*
struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;
*/
struct {
  struct spinlock lock;
  struct run *freelist;
} kmem[NCPU];//为每个CPU分配单独的freelist，使用独立的锁

void
kinit()
{
  char lockname[16];//一般都不会超出的...
  for(int i=0;i<NCPU;i++) { // 初始化所有锁
    snprintf(lockname, sizeof(lockname), "kmem_%d", i);
    initlock(&kmem[i].lock, lockname);
  }
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  push_off();//要获取cpuid，先关中断
  int cid = cpuid();
  acquire(&kmem[cid].lock);
  r->next = kmem[cid].freelist;
  kmem[cid].freelist = r;
  release(&kmem[cid].lock);
  pop_off();//开中断
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;
  push_off();//使用前关中断
  int cid = cpuid();
  acquire(&kmem[cid].lock);
  r = kmem[cid].freelist;
  if(r)//还有空闲页
    kmem[cid].freelist = r->next;
  else {
    int tmp_cid;
    //从其他CPU的空闲链表中搜索可能的结果
    for(tmp_cid = 0; tmp_cid < NCPU; ++tmp_cid) {
      if(tmp_cid == cid)
        continue;
      else{
        acquire(&kmem[tmp_cid].lock);
        r = kmem[tmp_cid].freelist;
        if(r) {
          kmem[tmp_cid].freelist = r->next;
          release(&kmem[tmp_cid].lock);
          break;
        }
        release(&kmem[tmp_cid].lock);
      }      
    }
  }
  release(&kmem[cid].lock);
  pop_off();//重新开中断

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}
