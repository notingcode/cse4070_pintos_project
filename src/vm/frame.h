#ifndef VM_FRAME_H
#define VM_FRAME_H

#include <stdbool.h>
#include "threads/synch.h"

struct frame 
  {
    void *base;
    struct page *page;
  };

void frame_init (void);

void frame_free (struct frame *);

#endif
