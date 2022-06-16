#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main() {
    printf("Hello, World!\n");
    int sid = create_sem(1);
    int pid;
    pid = fork();
    if(pid > 0){
        sem_p(sid);
        printf("parent: child=%d\n", pid);
        sem_v(sid);
        pid = wait(0);
        sem_p(sid);
        printf("child %d is done\n", pid);
        sem_v(sid);
        free_sem(sid);
    } else if(pid == 0){
        sem_p(sid);
        printf("child: exiting\n");
        sem_v(sid);
        exit(0);
    } else {
        printf("fork error\n");
    }
    exit(0);
}
