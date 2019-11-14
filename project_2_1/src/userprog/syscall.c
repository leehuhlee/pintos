#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "devices/input.h"
#include "devices/shutdown.h"
#include "process.h"
#include "lib/string.h"
#include "threads/vaddr.h"


static void syscall_handler (struct intr_frame *);
int exec(const char *filename,struct intr_frame *f);
int pibo(int n);
int sumfour(int a, int b, int c, int d);
/*
void halt(void);
void exit1(int status);
int read(int fd, void* buffer, unsigned size);
int write(int fd, void* buffer, unsigned size);
pid_t exec(const char *filename);
int wait(pid_t pid);
헤더로 이동
*/
void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f) 
{
	int * temp;
	void* start;
	//int pid=0;
	struct thread *cur;
	cur=thread_current();
	if(!f->esp){
		f->eax=-1;
		thread_exit();
		return;
	}
	
	start = f->esp+4;
	temp = (int*)f->esp;
//	hex_dump((uintptr_t)(f->esp),(f->esp), PHYS_BASE-(f->esp), true);
	if(*temp == SYS_HALT){
		halt();
	}
	else if(*temp == SYS_EXIT){
		 if(!is_user_vaddr(start)){
			 
			 f->eax=-1;
			 exit1(-1);
		 }
		 else{
			 exit1(*(int*)start);
			 f->eax=*(int*)start;
		 }
	}
	else if(*temp == SYS_EXEC){
		exec(*(char**)(start),f);
	}
	else if(*temp == SYS_WAIT){
		f->eax=wait(*(char *)(start));
	}
	else if(*temp == SYS_READ){
		f->eax = read(*(int*)start,*(void**)(start+4),*(unsigned*)(start+8));
	}
	else if(*temp == SYS_WRITE){
		f->eax = write(*(int*)start,*(void**)(start+4),*(unsigned*)(start+8));
	}
	else if(*temp == SYS_PIBO){
		f->eax = pibo(*(int*)start);
	}
	else if(*temp == SYS_SUMFOUR){
		f->eax = sumfour(*(int*)start,*(int*)(start+4),*(int*)(start+8),*(int*)(start+12));
	}
	else{
		//printf("That System call isn't included in this project\n");
		exit1(-1);
	}
}

void halt(void){
	shutdown_power_off();
}

void exit1(int status){
	
	struct thread *cur;
	cur=thread_current();
	//if(!check_dying(cur))
	
	cur->dying_msg=status;
	list_remove(&cur->child_elem);
//	printf("%s가 공격받고 있습니다.\n",cur->name);
	thread_exit();

}

int exec(const char *filename,struct intr_frame *f){
	
	int pid;
	struct thread *cur;
	cur=thread_current();
//	printf("////%s/////\n",filename);
/*	if((!filename)){
	
		cur->success=false;
		f->eax=-1;
		exit1(-1);

	}*/
	pid=process_execute (filename);
//	printf("%d/////\n",pid);


	//if(!cur->success) pid= -1;
	
	f->eax=pid;
	
	return pid;
}

int wait(pid_t pid)
{
	return process_wait(pid);


}

int read(int fd, void* buffer, unsigned size){
	int i=0;
	char temp;
	if(fd == 0){
		for(i=0;i<(int)size;i++){
			temp = input_getc();
			if(temp == '\0')return i;
			else{
				*(char*)(buffer+i) = temp;
			}
		}
		return i;
	}
	else{
		printf("We only read Standard input\n");
		return -1;
	}
}

int write(int fd, void* buffer, unsigned size){
	if(fd == 1){
		char tmp_str[16];
		strlcpy(tmp_str,buffer,strlen(buffer));
		putbuf(buffer, size);
		return size;
	}
	return -1;
}

int pibo(int n){
	int previous=-1;
	int result=1;
	int now=0;
	int i=0;
	for(i=0;i<=n;++i){
		now = result+previous;
		previous = result;
		result = now;
	}
	return result;
}

int sumfour(int a, int b, int c, int d){
	int result = 0;
	result = a+b+c+d;
	return result;
}
