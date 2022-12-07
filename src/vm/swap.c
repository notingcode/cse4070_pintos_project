#include "vm/swap.h"
#include <bitmap.h>
#include <debug.h>
#include <stdio.h>
#include "vm/frame.h"
#include "vm/page.h"
#include "threads/synch.h"
#include "threads/vaddr.h"

static struct block *swap_device;

static struct bitmap *swap_bitmap;

static struct lock swap_lock;

#define PAGE_SECTORS (PGSIZE / BLOCK_SECTOR_SIZE)

void
swap_init (void) 
{

}

void
swap_in (struct page *p) 
{

}

bool
swap_out (struct page *p) 
{

}
