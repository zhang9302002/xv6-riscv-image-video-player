#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
	if (argc <= 1) {
		printf("Please input this command as [touch file_name1 file_name2 ...]\n");
	}

	int i, count = 0;
	for (i = 1; i < argc; i++) {		
		int fd;
		// 测试文件是否存在
		if ((fd = open(argv[i], O_RDONLY)) < 0) {
			// 文件不存在就创建它
			fd = open(argv[i], O_CREATE|O_RDONLY);
			count++;
		}
		close(fd);
	}

	printf("%d file(s) created, %d file(s) skiped.\n", count, argc - 1 - count);

	exit(0);
}