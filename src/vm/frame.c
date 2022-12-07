#include "vm/frame.h"
#include <stdio.h>
#include "vm/page.h"
#include "devices/timer.h"
#include "threads/init.h"
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "threads/synch.h"
#include "threads/vaddr.h"

static struct frame *frames;
static size_t frame_cnt;

void
frame_init (void) 
{
  void *base;
  
  frames = malloc (sizeof *frames * init_ram_pages);
  if (frames == NULL)
    PANIC ("out of memory allocating page frames");

  while ((base = palloc_get_page (PAL_USER)) != NULL) 
    {
      struct frame *f = &frames[frame_cnt++];
      f->base = base;
      f->page = NULL;
    }
}

void
frame_lock (struct page *p) 
{
  struct frame *f = p->frame;
  if (f != NULL) 
    {
      if (f != p->frame)
        {
          ASSERT (p->frame == NULL); 
        } 
    }
}

void
frame_free (struct frame *f)
{   
  f->page = NULL;
}
