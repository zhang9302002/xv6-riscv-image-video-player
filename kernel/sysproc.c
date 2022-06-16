#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"


/**
 * my code start
 */
int user_int = 1;
int sys_read_user(void) {
    return user_int;
}
int sys_write_user(void) {
    user_int++;
    return user_int;
}

#define MAX_SEM 32
int sem_num = 0;
struct Sem{
    struct spinlock lock;
    int resource_num;
    int queue_length;
    int allocated;
    int procs[];
};
struct Sem sems[MAX_SEM];
struct spinlock sems_lock;
int sys_create_sem(void) {
    initlock(&sems_lock, "sems");
    int n;
    if(argint(0, &n) < 0)
        return -1;
    for(int i = 0; i < MAX_SEM; ++i)
        if(!sems[i].allocated) {
            sems[i].allocated = 1;
            sems[i].resource_num = n;
            sems[i].queue_length = 0;
            return i;
        }
    return 0;
}
int sys_free_sem(void) {
    int idx;
    if(argint(1, &idx) < 0)
        return -1;
    if(idx < 0 || idx >= MAX_SEM)
        return -1;
    acquire(&sems[idx].lock);
    if(sems[idx].allocated == 1 && sems[idx].resource_num > 0) {
        sems[idx].allocated = 0;
    }
    release(&sems[idx].lock);
    return 0;
}
int sys_sem_p(void) {
    int idx;
    if(argint(0, &idx) < 0)
        return -1;
    acquire(&sems[idx].lock);
    sems[idx].resource_num--;
    if(sems[idx].resource_num < 0) {
        sleep(&sems[idx], &sems[idx].lock);
    }
    release(&sems[idx].lock);
    return 0;
}
int sys_sem_v(void) {
    int idx;
    if(argint(0, &idx) < 0)
        return -1;
    acquire(&sems[idx].lock);
    sems[idx].resource_num++;
    if(sems[idx].resource_num <= 0) {
        wakeup(&sems[idx]);
    }
    release(&sems[idx].lock);
    return 0;
}

/**
 * my code end
 */

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

// return the available memory of xv6
uint64
sys_memory(void)
{
    return bd_memory();
}