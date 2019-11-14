#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

void syscall_init (void);
void halt(void);
void exit1(int status);
int read(int fd, void* buffer, unsigned size);
int write(int fd, void* buffer, unsigned size);
int pibo(int n);
int sumfour(int a, int b, int c, int d);

typedef int pid_t;
#define PID_ERROR ((pid_t) -1)




 int wait(pid_t pid);
#endif /* userprog/syscall.h */
