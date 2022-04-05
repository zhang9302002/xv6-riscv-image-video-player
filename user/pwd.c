#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "user/user.h"


#define SIZE 128

int cnt = 0;

char*
fmtname(char *path)
{
    static char buf[DIRSIZ+1];
    char *p;

    // Find first character after last slash.
    for(p=path+strlen(path); p >= path && *p != '/'; p--)
        ;
    p++;

    // Return blank-padded name.
    if(strlen(p) >= DIRSIZ)
        return p;
    memmove(buf, p, strlen(p));
    return buf;
}

void inum_to_name(uint inode_to_find, char *namebuf, int buflen);
uint get_inode(char *filename)
{
    struct stat info;
    if (stat(filename, &info) == -1)
        exit(1);
    return info.ino;
}

void printfather(uint d) {
    uint my_inode;
    char its_name[SIZE];
    if (get_inode("..") == d)
        return;
    chdir("..");
    inum_to_name(d, its_name, SIZE);
    my_inode = get_inode(".");
    printfather(my_inode);
    printf("/%s", its_name);
    ++cnt;
}

void inum_to_name(uint inode_to_find, char *namebuf, int buflen)
{
    char path[10] = ".";
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;
    if((fd = open(path, 0)) < 0){
        fprintf(2, "ls: cannot open %s\n", path);
        return;
    }
    if(fstat(fd, &st) < 0){
        fprintf(2, "ls: cannot stat %s\n", path);
        close(fd);
        return;
    }
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf) {
        printf("ls: path too long\n");
        exit(1);
    }
    strcpy(buf, path);
    p = buf+strlen(buf);
    *p++ = '/';
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
        if(de.inum == 0)
            continue;
        memmove(p, de.name, DIRSIZ);
        p[DIRSIZ] = 0;
        if(stat(buf, &st) < 0){
            printf("ls: cannot stat %s\n", buf);
            continue;
        }
        if(st.ino == inode_to_find) {
            strcpy(namebuf, fmtname(buf));
            namebuf[buflen - 1] = '\0';
            return;
        }
    }

    close(fd);

    printf("error looking for inum %ld\n", inode_to_find);
    exit(1);
}

int
main(int argc, char *argv[])
{
    cnt = 0;
    printfather(get_inode("."));
    if(cnt)
        printf("\n");
    else
        printf("/\n");
    exit(0);
}
