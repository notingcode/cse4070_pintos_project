#ifndef VM_PAGE_H
#define VM_PAGE_H

#include <hash.h>
#include "devices/block.h"
#include "filesys/off_t.h"
#include "threads/synch.h"

struct page 
  {
    void *addr;
    bool read_only;          
    struct thread *thread;     

    struct hash_elem hash_elem; 

    struct frame *frame;

    block_sector_t sector;     
  };

void page_exit (void);

struct page *page_allocate (void *vaddr, bool read_only);
void page_deallocate (void *vaddr);

bool page_in (void *fault_addr);
bool page_out (struct page *p);

#endif