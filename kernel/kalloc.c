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

struct {
  struct spinlock lock;
  struct run *freelist;
  char *refPage;//
  int pageCount;//
  char* _end;
} kmem;

int
pageCount(void *pa_start, void *pa_end)
{
  char *p;
  int count=0;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    count++;
  return count;
}

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  kmem.pageCount=pageCount(end,(void*)PHYSTOP);//计算页表大小
  kmem.refPage=end;
  kmem._end=end+kmem.pageCount;//空出保留字段
  for(int no=0;no<kmem.pageCount;no++){//置0
    kmem.refPage[no]=0;
  }
  freerange(kmem._end, (void*)PHYSTOP);//好吗...
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
  if(kmem.refPage[page_index(pa)]>=1){
    delRef(pa);
    if(kmem.refPage[page_index(pa)]!=0)
      return;
  }

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < kmem._end || (uint64)pa >= PHYSTOP)//if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if(r){
    memset((char*)r, 5, PGSIZE); // fill with junk
    addRef((void*)r);//
  }
  return (void*)r;
}

int 
page_index(void* pa){//kmem...是啥...
  uint64 _pa=(uint64)pa;
  _pa=PGROUNDDOWN(_pa);
  int res=(_pa-(uint64)(kmem._end))/PGSIZE;//  int res=(pa-(uint64)end)/PGSIZE;
  if(res<0||res>kmem.pageCount){
    panic("page_index illegal");
  }
  return res;
}

void 
addRef(void* pa){
  int index=page_index(pa);
  acquire(&kmem.lock);
  kmem.refPage[index]++;
  release(&kmem.lock);
}

void 
delRef(void* pa){
  int index=page_index(pa);
  acquire(&kmem.lock);
  kmem.refPage[index]--;
  release(&kmem.lock);
}