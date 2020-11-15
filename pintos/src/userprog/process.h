#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H
#include <stdio.h>
#include "threads/thread.h"
#include "threads/synch.h"

/* Process identifier type.
   You can redefine this to whatever type you like. */
typedef int pid_t;
#define PID_ERROR ((pid_t) -1)          /* Error value for tid_t. */
#define PID_INIT ((pid_t) -2)

struct process_control_block {
    pid_t pid;

    const char *cmdline;

    struct list_elem elem;
    struct thread *parent_thread;

    bool waiting;   //indicates whether parent process is waiting.
    bool exited;    //indicates whether the process is done.
    bool orphan;    //indicates whether the parent process has terminated before.
    int32_t exitcode;   //the exit code passed from exit(), when exited = true

    //Synchronization
    struct semaphore sema_init; //the semaphore used between start_process() and process_execute()
    struct semaphore sema_wait; //the semaphore used for wait()
};

//File descriptor
struct file_desc {
    int id;
    struct list_elem elem;
    struct file* file;
    struct dir* dir;    //When directory is opening, dir!=NULL 
};

pid_t process_execute (const char *file_name);
int process_wait (tid_t);
void process_exit (void);
void process_activate (void);
//////////////////////////////////////
void parse_cmd(char *file_name, char **argv, int *argc); //parse the string and save the command to the cmd
void construct_stack(char **argv,int *argc, void ** esp);
struct process_control_block *process_find_child(pid_t pid);
#endif /* userprog/process.h */
