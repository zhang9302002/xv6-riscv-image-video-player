// System call numbers
#define SYS_fork    1
#define SYS_exit    2
#define SYS_wait    3
#define SYS_pipe    4
#define SYS_read    5
#define SYS_kill    6
#define SYS_exec    7
#define SYS_fstat   8
#define SYS_chdir   9
#define SYS_dup    10
#define SYS_getpid 11
#define SYS_sbrk   12
#define SYS_sleep  13
#define SYS_uptime 14
#define SYS_open   15
#define SYS_write  16
#define SYS_mknod  17
#define SYS_unlink 18
#define SYS_link   19
#define SYS_mkdir  20
#define SYS_close  21

// System calls for semaphore

#define SYS_create_sem 24
#define SYS_free_sem 25
#define SYS_sem_p 26
#define SYS_sem_v 27

// System calls for vga display

#define SYS_show_window 33
#define SYS_close_window 34
#define SYS_reg_keycb 35
#define SYS_cb_return 36

// System calls for ac97 driver

#define SYS_memory 37
#define SYS_kwrite 38
#define SYS_setSampleRate 39
#define SYS_pause 40
#define SYS_wavdecode 41