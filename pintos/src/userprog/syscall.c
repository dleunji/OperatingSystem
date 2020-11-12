#include "devices/shutdown.h"
#include "devices/input.h"
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "userprog/syscall.h"
#include "userprog/process.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/palloc.h"
#include "threads/vaddr.h"

struct lock filesys_lock;

void check_user(const uint8_t *addr);
static int get_user(const uint8_t *uaddr);
//static bool put_user(uint8_t *udst,uint8_t byte);
static int read_user(void *src, void *dst, size_t bytes);
static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  lock_init(&filesys_lock);
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f) 
{
  //hex_dump(0xbfffffe0,f->esp,100,1);
  int syscall_number;
  read_user(f->esp,&syscall_number,sizeof(syscall_number));
  ASSERT(sizeof(syscall_number == 4));
  //printf("system call number = %d\n",syscall_number);
  
  //Store the esp, which is needed in the page fault handler.
  thread_current()->current_esp = f->esp;
  
  switch(syscall_number){
    case SYS_HALT: //0
    {
      sys_halt();
      break;
    }

    case SYS_EXIT: //1
    {
      int exit_code;
      read_user(f->esp + 4,&exit_code,sizeof(exit_code));
      sys_exit(exit_code);
      break;
    }

    case SYS_EXEC: //2
    {
      void *cmd_line;
      read_user(f->esp+4,&cmd_line,sizeof(cmd_line));
      int return_code = sys_exec((const char*)cmd_line);
      f->eax = (uint32_t) return_code;
      break;
    }

    case SYS_WAIT: //3
    {
      pid_t pid;
      read_user(f->esp + 4,&pid,sizeof(pid));
      int ret = sys_wait(pid);
      f->eax = (uint32_t)ret;
      break;
    }
  
    case SYS_CREATE: //4
    {
      const char *file_name;
      unsigned initial_size;
      //printf("I'm here-1\n");
      read_user(f->esp + 4,&file_name,sizeof(file_name));
      read_user(f->esp + 8,&initial_size,sizeof(initial_size));
      //printf("I'm here0\n");
      bool return_code = sys_create(file_name,initial_size);
      f->eax = return_code;
      break;
    }

    case SYS_REMOVE: //5
    {
      const char *file_name;
      bool return_code; 
      read_user(f->esp + 4,&file_name,sizeof(file_name));
      return_code = sys_remove(file_name);
      f->eax = return_code;
      break;
    }

    case SYS_OPEN: //6
    {
      const char *file_name;
      int return_code;
      read_user(f->esp + 4,&file_name,sizeof(file_name));
      return_code = sys_open(file_name);
      f->eax = return_code;
      break;
    }
/*
    case SYS_FILESIZE: //7
    {
      int fd = (f->esp) + 4;
      int return_code = sys_filesize(fd);
      f->eax = return_code;
      break;
    }
*/
    case SYS_READ: //8
    {
      int fd;
      void *buffer;
      unsigned size;

      read_user(f->esp + 4, &fd, sizeof(fd));
      read_user(f->esp + 8, &buffer, sizeof(buffer));
      read_user(f->esp + 12, &size, sizeof(size));

      int return_code = sys_read(fd,buffer,size);
      f->eax = (uint32_t) return_code;
      break;
    }

    case SYS_WRITE: //9
    {
      int fd;
      void *buffer;
      unsigned size;

      read_user(f->esp + 4, &fd, sizeof(fd));
      read_user(f->esp + 8, &buffer, sizeof(buffer));
      read_user(f->esp + 12, &size, sizeof(size));

      int return_code = sys_write(fd,buffer,size);
      f->eax = (uint32_t) return_code;
      break;
    }
  }
  
  //printf ("system call!\n");
}

///////////////////////////////////////
////////////Implementation/////////////
///////////////////////////////////////

void sys_halt(void){
  shutdown_power_off();
}

void sys_exit(int status){
  printf("%s: exit(%d)\n",thread_current()->name,status);

  struct process_control_block *pcb = thread_current() -> pcb;
  if(pcb != NULL)
    pcb -> exitcode = status;
  thread_exit();
}

pid_t sys_exec(const char *cmd_line){
  //process_execute()함수를 호출하여 자식 프로세스 생성
  //생성된 자식 프로세스의 프로세스 디스크립터 검색
  //자식 프로세스의 프로그램이 적재될 때까지 대기
  //프로그램 적재 실패 시 -1 리턴
  //프로그램 적재 성공 시 자식 프로세스 pid 리턴

  //1. Check the validity
  check_user((const uint8_t *)cmd_line);
  //printf("Exec : %s\n",cmd_line);
  //2. Create a new process
  pid_t pid = process_execute(cmd_line);
  if(pid == PID_ERROR)
    return pid;

  //Obtain the new process
  struct process_control_block *child = process_find_child(pid);
  if(child == NULL)
    return PID_ERROR;

  //Wait until the new process is successfully loaded
  while(child->waiting)
    thread_yield();

  if(child->orphan)
    return PID_ERROR;

  return  pid;
}
int sys_wait(pid_t pid){
  //printf("Wait : %d\n",pid);
  return process_wait(pid);
}


bool sys_create(const char *file_name, unsigned initial_size){
  if(file_name == NULL)
    sys_exit(-1);
  check_user((const uint8_t*)file_name);
  //printf("I'm here1\n");
  lock_acquire(&filesys_lock);
  //printf("I'm here2\n");
  bool success = filesys_create(file_name,initial_size);
  //lock_release(&filesys_lock);
  //printf("I'm here3\n");
  return success;
}

bool sys_remove(const char *file_name){
  if(file_name == NULL)
    sys_exit(-1);
  check_user((const uint8_t*)file_name);
  lock_acquire(&filesys_lock);
  bool success = filesys_remove(file_name);
  lock_release(&filesys_lock);
  return success;
}

int sys_open(const char *file_name){
  if(file_name)
    sys_exit(-1);
  check_user((const uint8_t*)file_name);
  struct file_desc* fd = palloc_get_page(0);
  if(!fd) 
    return -1;

  lock_acquire(&filesys_lock);
  struct file *file = filesys_open(file_name);
  if(!file){
    palloc_free_page(fd);
    lock_release(&filesys_lock);
    return -1;
  }
  //save the file to the file descriptor
  fd->file = file;
  
  //no directory
  //fd_list
  struct list* fd_list = &thread_current()->file_descriptors;
  if(!list_empty(fd_list)){
    fd->id = 3;
  }
  else{
    fd->id = (list_entry(list_back(fd_list),struct file_desc,elem)->id) + 1;
  }
  list_push_back(fd_list,&(fd->elem));
  return fd->id;
}

//int sys_filesize(int fd);

int sys_read(int fd,void *buffer, unsigned size){
  check_user((const uint8_t *)buffer);
  check_user((const uint8_t *)buffer + size -1);

  if(fd == 0){
    unsigned i;
    for(i = 0;i<size;i++){//STDIN
      if(!input_getc()){
        sys_exit(-1);
      }
    }
  }
  return size;
}

int sys_write(int fd, const void *buffer,unsigned size){
  //printf("%d\n",size);
  //int ret;
  check_user((const uint8_t*)buffer);
  check_user((const uint8_t*)buffer + size -1);

  if(fd == 1){
    putbuf(buffer,size);
  }
  return size;
}

///////////////////////////////////////
/////////////Memory Access/////////////
///////////////////////////////////////

void check_user(const uint8_t *addr){
  if(!is_user_vaddr(addr)){
    sys_exit(-1);
  }
}

static int get_user(const uint8_t *uaddr){
  if(!((void*)uaddr < PHYS_BASE)){
    return -1;
  }

  int result;
  asm("movl $1f, %0; movzbl %1, %0; 1:"
      : "=&a"(result): "m"(*uaddr));
  return result;
}
/*
static bool put_user(uint8_t *udst,uint8_t byte){
  int error_code;
  if(!((void*)udst < PHYS_BASE)){
    return false;
  }

  asm("movl $lf, %0; movb %b2, %1; 1:"
      : "=&a" (error_code), "=m" (*udst) : "q"(byte));
  return error_code != -1;
}
*/

static int read_user(void *src, void *dst, size_t bytes){
  int32_t value;
  size_t i;
  for(i=0;i < bytes;i++){
    value = get_user(src + i);
    if(value == -1)//invalide memory access
      sys_exit(-1);
    
    *(char*)(dst + i) = value & 0xff;
  }
  return (int)bytes;
}

