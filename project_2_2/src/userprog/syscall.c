#include<debug.h>
#include<stdio.h>
#include<syscall-nr.h>
#include"syscall.h"
#include"process.h"
#include"threads/interrupt.h"
#include"threads/thread.h"
#include"threads/vaddr.h"
#include"threads/malloc.h"
#include"lib/string.h"
#include"devices/input.h"
#include"devices/shutdown.h"
#include"filesys/inode.h"
#include"filesys/file.h"
#include"filesys/filesys.h"
#include"threads/synch.h"

static void syscall_handler (struct intr_frame *);
int exec(const char *filename,struct intr_frame *f);


typedef struct _file_node{
	struct file* file;
	char name[30];
	int fd;
	int tid;//open한 thread의 tid
	struct list_elem elem;
}file_node;

struct list realfilelist;
int fd_idx;

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
  list_init(&realfilelist);
  fd_idx=3;
  sema_init(&cham_sin,1);
}

static void
syscall_handler (struct intr_frame *f) 
{
	int * temp;
	void* start;
	struct thread *cur;

	cur=thread_current();
	if(!f->esp){
		f->eax=-1;
		thread_exit();
		return;
	}

	start = f->esp+4;
	temp = (int*)f->esp;
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
		f->eax = wait(*(char *)(start));
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
	else if(*temp == SYS_CREATE){
		f->eax = create1(*(char**)start,*(unsigned*)(start+4));
	}
	else if(*temp == SYS_CLOSE){
		close1(*(int*)start);
	}
	else if(*temp == SYS_REMOVE){
		f->eax = remove1(*(char**)start);
	}
	else if(*temp == SYS_OPEN){
		f->eax = open1(*(char**)start);
	}
	else if(*temp == SYS_FILESIZE){
		f->eax = filesize1(*(int*)start);
	}
	else if(*temp == SYS_TELL){
		f->eax = tell1(*(int*)start);
	}
	else if(*temp == SYS_SEEK){
		seek1(*(int*)start,*(unsigned*)(start+4));
	}
	else{
		exit1(-1);
	}
}

void halt(void){
	shutdown_power_off();
}

void exit1(int status){

	struct thread *cur;
	struct list_elem *e;
	cur=thread_current();
	sema_down(&cur->control);
	cur->dying_msg=status;

	for(e=list_begin(&realfilelist);e!=list_end(&realfilelist);e=list_next(e)){
		if(cur->tid==list_entry(e,file_node,elem)->tid){
			list_remove(e);
			//free(list_entry(e,file_node,elem));
			e=list_prev(e);
		}
	}
	thread_exit();
}

int exec(const char *filename,struct intr_frame *f){

	int pid;
	pid=process_execute (filename);
	f->eax=pid;
	return pid;
}

int wait(pid_t pid)
{
	int tid;
	tid= process_wait(pid);
	return tid;
}

int read(int fd, void* buffer, unsigned size){
	int i=0,getsu;;
	int result;
	struct list_elem *e;
	char temp;

	if(buffer+size > PHYS_BASE){
		exit1(-1);
		return -1;
	}
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
		getsu=list_size(&realfilelist);
		e=list_begin(&realfilelist);
		for(i=0;i<getsu;i++){
			if(fd==list_entry(e,file_node,elem)->fd){
				sema_down(&cham_sin);
				result = file_read(list_entry(e,file_node,elem)->file,buffer,size);
				sema_up(&cham_sin);
				return result;
			}
			e = list_next(e);
		}
		return -1;
	}
}

int write(int fd, void* buffer, unsigned size){
	int i,getsu;
	int result;
	struct list_elem *e;


	if(buffer+size > PHYS_BASE){
		exit1(-1);
		return -1;
	}
	if(fd == 1){
		putbuf(buffer, size);
		return size;
	}
	else{
		getsu=list_size(&realfilelist);
		e=list_begin(&realfilelist);
		for(i=0;i<getsu;i++){
			if(fd==list_entry(e,file_node,elem)->fd){
				sema_down(&cham_sin);
				result = file_write(list_entry(e,file_node,elem)->file,buffer,size);
				sema_up(&cham_sin);
				return result;
			}
			e = list_next(e);
		}
		return -1;
	}
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

int filesize1(int fd){
	int size=0;
	int getsu=0;
	int i=0;
	struct list_elem * e;

	getsu=list_size(&realfilelist);
	e=list_begin(&realfilelist);
	for(i=0;i<getsu;i++){
		if(fd==list_entry(e,file_node,elem)->fd){
			size = file_length(list_entry(e,file_node,elem)->file);
			return size;
		}
		e = list_next(e);
	}
	return -1;
}

void seek1(int fd, unsigned position){

	int getsu=0;int i=0;
	struct list_elem *e;

	getsu=list_size(&realfilelist);
	e=list_begin(&realfilelist);
	for(i=0;i<getsu;i++){
		if(fd==list_entry(e,file_node,elem)->fd){
			file_seek(list_entry(e,file_node,elem)->file,position);
			break;
		}
		e = list_next(e);
	}
}

bool remove1(const char *filename){
	bool result=false;
	if(filename == NULL){
		exit1(-1);
		return result;
	}
	else{
		sema_down(&cham_sin);
		result = filesys_remove(filename);
		sema_up(&cham_sin);
		return result;
	}
}

bool create1(const char *filename, unsigned initial_size){
	bool result=false;

	if(filename == NULL){
		exit1(-1);
		return result;
	}
	else{
		sema_down(&cham_sin);
		result = filesys_create(filename,initial_size);
		sema_up(&cham_sin);
		return result;
	}
}

int open1(const char *filename){
	struct thread *cur=thread_current();
	int result=0;
	char tmp[20];
	struct file* file;
	file_node *new_file;
	char* token,*save_ptr;
	
	new_file=(file_node *)malloc(sizeof(file_node));
	strlcpy(tmp,filename,strlen(filename)+1);
	token = strtok_r(tmp," \n",&save_ptr);

	if(!token){
		free(new_file);
		return -1;
	}
	sema_down(&cham_sin);
	file=filesys_open(token);
	sema_up(&cham_sin);
	if(!file){
		free(new_file);	
		return -1;
	}
	new_file->file=file;
	new_file->fd=fd_idx;
	new_file->tid=cur->tid;
	strlcpy(new_file->name,token,strlen(token)+1);

	list_push_back(&realfilelist,&new_file->elem);
	fd_idx++;
	result=fd_idx-1; 
	return result;
}

unsigned tell1(int fd){
	unsigned int result=0;
	int i,getsu=0;
	struct list_elem *e;

	getsu=list_size(&realfilelist);
	e=list_begin(&realfilelist);
	for(i=0;i<getsu;i++){
		if(fd==list_entry(e,file_node,elem)->fd){
			sema_down(&cham_sin);
			result = file_tell(list_entry(e,file_node,elem)->file);
			sema_up(&cham_sin);
			return result;
		}
		e = list_next(e);
	}
	return 0;
}

void close1(int fd){
	struct list_elem *e;
	struct thread *cur=thread_current();
	struct thread *tmp;
	int i,getsu;
	getsu=list_size(&realfilelist);

	if(fd==0 || fd==1 || fd==2) return;
	
	e=list_begin(&realfilelist);
	for(i=0;i<getsu;i++){
		if(fd==list_entry(e,file_node,elem)->fd){
			 tmp=cur->father;	
			 while(tmp){
				 if(tmp->tid==list_entry(e,file_node,elem)->tid) exit1(-1);//multi_child_fd
				 tmp=tmp->father;
			 }
			list_remove(e);
			break;
		}
		e = list_next(e);
	}
}
