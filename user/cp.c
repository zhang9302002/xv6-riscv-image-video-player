#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "kernel/fs.h"
#include "user/user.h"

#define BUF_SIZE 256

void ls(char *path);
char* fmtname(char *path);

int main(int argc, char *argv[])
{
	//判断参数
	if (argc !=3)
	{
		printf("please input the command as [cp src_file dest_file]\n");
		exit(0);
	}
	
	//打开源文件
	int fd_src = open(argv[1], O_RDONLY);
	if (fd_src == -1)
	{
		printf("open source file failed\n");
		exit(0);
	}
	
	//判断源文件状态是否为文件夹
	struct stat st;
	fstat(fd_src, &st);
	if (st.type == T_DIR)
	{
		printf("source file is a directory, the files in that directory is:\n");
		ls(argv[1]);
		printf("the program can't open the file in that directory after list them.\n");
		printf("So, I'm sorry that you have to copy them one by one.\n");
		exit(0);
	}

	//判断第二个参数是不是以"/"结尾，如果是，则补全路径
	char com[128] = {};
	strcpy(com, argv[2]);
	int len1 = strlen(argv[1]);
	int len2 = strlen(argv[2]);
	if (argv[2][len2-1] == '/')
	{
		//找到argv[1]中的文件名
		int i = len1 - 1;
		for (; i >= 0; i--)
			if (argv[1][i] == '/')
				break;
		i++;
		strcpy(&com[len2], &argv[1][i]);
	}
	
	//打开目标文件
	int fd_dest = open(com, O_WRONLY|O_CREATE);
	if (fd_dest == -1)
	{
		printf("create dest file failed\n");
		exit(0);
	}
	
	//复制文件
	char buf[BUF_SIZE] = {};
	int len = 0;
	while((len = read(fd_src, buf, BUF_SIZE)) > 0)
		write(fd_dest, buf, len);
	
	//关闭文件和程序
	close(fd_src);
	close(fd_dest);
	exit(0);
}

char* fmtname(char *path)
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
	memset(buf+strlen(p), ' ', DIRSIZ-strlen(p));
	return buf;
}

void ls(char *path)
{
	char buf[512], *p;
	int fd;
	struct dirent de;
	struct stat st;
	
	if((fd = open(path, 0)) < 0)
	{
		printf("cannot open %s\n", path);
		return;
	}
	
	if(fstat(fd, &st) < 0)
	{
		printf("cannot stat %s\n", path);
		close(fd);
		return;
	}
	
	switch(st.type)
	{
		case T_FILE:
		printf("name = %s, type = file, size = %d\n", fmtname(path), st.size);
		break;
		
		case T_DIR:
		if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf)
		{
			printf("path too long\n");
			break;
		}
		strcpy(buf, path);
		p = buf+strlen(buf);
		*p++ = '/';
		
		while(read(fd, &de, sizeof(de)) == sizeof(de))
		{
			if(de.inum == 0)
				continue;
			memmove(p, de.name, DIRSIZ);
			p[DIRSIZ] = 0;
			if(stat(buf, &st) < 0)
			{
				printf("cannot stat %s\n", buf);
				continue;
			}
			printf("name = %s, type = directory, size = %d\n", fmtname(buf), st.size);
		}
		break;
	}
	close(fd);
}
