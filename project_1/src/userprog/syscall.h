#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

void syscall_init (void);
void halt(void);
void exit1(int status);
int read(int fd, void* buffer, unsigned size);
int write(int fd, void* buffer, unsigned size);
int pibo(int n);
int sumfour(int a, int b, int c, int d);

//project2
int filesize1(int fd);
int open1(const char *filename);
unsigned tell1(int fd);
void close1(int fd);
bool remove1(const char *filename);
bool create1(const char *filename, unsigned initial_size);
void seek1(int fd, unsigned position);

typedef int pid_t;
#define PID_ERROR ((pid_t) -1)

typedef struct _list_elem{
	struct list_elem *prev;
	struct list_elem *next;
}list_elem;

int wait(pid_t pid);
#endif /* userprog/syscall.h */
