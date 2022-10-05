#include "userprog/syscall.h"
#include <stdio.h>
#include <stdint.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "list.h"
#include "process.h"
#include "threads/synch.h"

static void syscall_handler(struct intr_frame *);

typedef int pid_t;

bool create(const char *file, unsigned initial_size);
bool remove(const char *file);
int open(const char *file);
void is_user_addr_valid(const void *);
void close(int fd);
int read(int fd, void *buffer, unsigned size);
int write(int fd, void *buffer, unsigned size);
void exit(int status);
int wait(pid_t pid);
pid_t exec(const char *cmd_line);
struct file *get_file_from_fd(int fd);
char *get_filename(const char *cmd_line);
void seek(int fd, unsigned position);
unsigned tell(int fd);
int fibonacci(int n);
int max_of_four_int(int a, int b, int c, int d);

static int fd = 2;

struct file_fd_map
{
    struct file *file_;
    int fd;
    struct list_elem elem;
};

void syscall_init(void)
{
    intr_register_int(0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler(struct intr_frame *f UNUSED)
{
    void *esp = f->esp;

    is_user_addr_valid(esp);

    switch (*(int *)(esp))
    {
    case SYS_EXIT:
        is_user_addr_valid(esp + 4);
        exit(*(int *)(esp + 4));
        break;

    case SYS_CREATE:
        is_user_addr_valid(esp + 16);
        is_user_addr_valid(esp + 20);
        is_user_addr_valid(*(int *)(esp + 16));
        f->eax = create(*(int *)(esp + 16), *(unsigned *)(esp + 20));
        break;

    case SYS_OPEN:
        is_user_addr_valid(*(int *)(esp + 4));
        f->eax = open(*(int *)(esp + 4));
        break;

    case SYS_CLOSE:
        is_user_addr_valid(esp + 4);
        close(*(int *)(esp + 4));
        break;

    case SYS_READ:
        is_user_addr_valid(esp + 16);
        is_user_addr_valid(esp + 20);
        is_user_addr_valid(esp + 24);
        is_user_addr_valid(*(int *)(esp + 24));
        f->eax = read(*(int *)(esp + 20), *(int *)(esp + 24), *(unsigned *)(esp + 28));
        break;

    case SYS_WRITE:
        is_user_addr_valid(esp + 16);
        is_user_addr_valid(esp + 20);
        is_user_addr_valid(esp + 24);
        is_user_addr_valid(*(int *)(esp + 24));
        f->eax = write(*(int *)(esp + 20), *(int *)(esp + 24), *(unsigned *)(esp + 28));
        break;

    case SYS_EXEC:
        is_user_addr_valid(*(int *)(esp + 4));
        f->eax = exec(*(int *)(esp + 4));
        break;

    case SYS_WAIT:
        is_user_addr_valid(esp + 4);
        f->eax = wait(*(pid_t *)(esp + 4));
        break;

    case SYS_HALT:
        halt();
        break;

    case SYS_SEEK:
        is_user_addr_valid(esp + 16);
        is_user_addr_valid(esp + 20);
        seek(*(int *)(esp + 16), *(int *)(esp + 20));
        break;

    case SYS_TELL:
        is_user_addr_valid(esp + 4);
        f->eax = tell(*(int *)(esp + 4));
        break;

    case SYS_REMOVE:
        is_user_addr_valid(esp + 4);
        f->eax = remove(*(int *)(esp + 4));
        break;

    case SYS_FIBO:
        is_user_addr_valid(esp + 4);
        f->eax = fibonacci(*(int *)(esp + 4));
        break;

    case SYS_MAXOFFOURINT:
        is_user_addr_valid(esp + 4);
        is_user_addr_valid(esp + 8);
        is_user_addr_valid(esp + 12);
        is_user_addr_valid(esp + 16);
        f->eax = max_of_four_int(*(int *)(esp + 4), *(int *)(esp + 8), *(int *)(esp + 12), *(int *)(esp + 16));
        break;
    }
}

bool create(const char *file, unsigned initial_size)
{
    return filesys_create(file, initial_size);
}

int open(const char *file)
{
    struct file *file_ = filesys_open(file);
    if (file_ != NULL)
    {
        struct file_fd_map *ffd_map = malloc(sizeof(struct file_fd_map));
        ffd_map->file_ = file_;
        ffd_map->fd = ++fd;
        list_push_back(&thread_current()->files, &ffd_map->elem);
        return fd;
    }
    else
        return -1;
}

void close(int fd)
{
    if (!list_empty(&thread_current()->files))
    {
        struct list_elem *list_elem_;
        for (
            list_elem_ = list_begin(&thread_current()->files);
            list_elem_ != list_end(&thread_current()->files);
            list_elem_ = list_next(list_elem_))
        {
            struct file_fd_map *ffd_map = list_entry(list_elem_, struct file_fd_map, elem);
            if (ffd_map->fd == fd)
            {
                file_close(ffd_map->file_);
                list_remove(list_elem_);
            }
        }
    }
}

int read(int fd, void *buffer, unsigned size)
{
    if (fd == 0)
    {
        uint8_t *buffer_ = (uint8_t *)buffer;
        for (int i = 0; i < size; i++)
            buffer_[i] = input_getc();
        return size;
    }
    else
    {
        if (get_file_from_fd(fd) != NULL)
        {
            struct file *file_ = get_file_from_fd(fd);
            return file_read(file_, buffer, size);
        }
    }
    return -1;
}

int write(int fd, void *buffer, unsigned size)
{
    if (fd == 1)
    {
        putbuf(buffer, size);
        return size;
    }
    else
    {
        if (get_file_from_fd(fd) != NULL)
        {
            struct file *file_ = get_file_from_fd(fd);
            return file_write(file_, buffer, size);
        }
    }
    return -1;
}

void is_user_addr_valid(const void *vaddr)
{
    if (!is_user_vaddr(vaddr) || !pagedir_get_page(thread_current()->pagedir, vaddr))
    {
        exit(-1);
    }
}

pid_t exec(const char *cmd_line)
{

    struct file *file_ = filesys_open(get_filename(cmd_line));

    if (file_ != NULL)
    {
        file_close(file_);
        return process_execute(cmd_line);
    }
    else
        return -1;
}

char *get_filename(const char *cmd_line)
{

    char *fn_copy = malloc(strlen(cmd_line) + 1);
    strlcpy(fn_copy, cmd_line, strlen(cmd_line) + 1);

    char *saveptr;
    return strtok_r(fn_copy, " ", &saveptr);
}

void halt(void)
{
    shutdown_power_off();
}

int wait(pid_t pid)
{
    return process_wait(pid);
}

void seek(int fd, unsigned position)
{
    if (get_file_from_fd(fd) != NULL)
    {
        struct file *file_ = get_file_from_fd(fd);
        file_seek(file_, position);
    }
    else
        return;
}

unsigned tell(int fd)
{
    if (get_file_from_fd(fd) != NULL)
    {
        struct file *file_ = get_file_from_fd(fd);
        return file_tell(file_);
    }
    else
        exit(-1);
}

bool remove(const char *file)
{
    return filesys_remove(file);
}

int filesize(int fd)
{
    if (get_file_from_fd(fd) != NULL)
    {
        struct file *file_ = get_file_from_fd(fd);
        return file_length(file_);
    }
    else
        exit(-1);
}

void exit(int status)
{

    struct list_elem *child_in_list = NULL;

    child_in_list = getChild(thread_current()->tid, &thread_current()->parent->children);

    if (child_in_list != NULL)
    {

        struct child *child_ = list_entry(child_in_list, struct child, elem);

        if (child_ == NULL)
        {
            thread_exit();
            return;
        }

        child_->alive = false;
        child_->exit_status = status;
        child_->has_lock = false;

        thread_current()->exit_status = status;

        thread_exit();
    }
    else
    {
        thread_exit();
        return;
    }
}

struct file *get_file_from_fd(int fd)
{

    if (!list_empty(&thread_current()->files))
    {
        struct list_elem *list_elem_;
        for (
            list_elem_ = list_begin(&thread_current()->files);
            list_elem_ != list_end(&thread_current()->files);
            list_elem_ = list_next(list_elem_))
        {
            struct file_fd_map *ffd_map = list_entry(list_elem_, struct file_fd_map, elem);
            if (ffd_map->fd == fd)
            {
                return ffd_map->file_;
            }
        }
    }

    return NULL;
}

int fibonacci(int n)
{
    if (n <= 1)
        return n;

    return n + fibonacci(n - 1);
}

int max_of_four_int(int a, int b, int c, int d)
{
    int max = a;

    max = max < b ? b : max;
    max = max < c ? c : max;
    max = max < d ? d : max;

    return max;
}