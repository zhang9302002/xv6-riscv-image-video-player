#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/sound.h"
#include "kernel/fcntl.h"
#include "user/user.h"

#define SINGLE_BUF_SIZE 4096
#define BUF_SIZE  (SINGLE_BUF_SIZE * 8)

char buf[BUF_SIZE];

int
main(int argc, char *argv[])
{
    int i;
    int fd;
    struct wav info;
    fd = open(argv[1], O_RDWR);
    if (fd < 0) {
        printf("open wav file fail\n");
        exit(0);
    }

    read(fd, &info, sizeof(struct wav));
    if ((info.riff_id != 0x46464952)||(info.wave_id != 0x45564157)) {
        printf("invalid file format\n");
        close(fd);
        exit(0);
    }

    if ((info.info.id != 0x20746d66)||
        (info.info.channel != 0x0002)||
        (info.info.bytes_per_sample != 0x0004)||
        (info.info.bits_per_sample != 0x0010)) {
        printf("data encoded in an unaccepted way\n");
        printf("id=%x\n", info.info.id);
        printf("channel=%x\n", info.info.channel);
        printf("bytes_per_sample=%x\n", info.info.bytes_per_sample);
        printf("bits_per_sample=%x\n", info.info.bits_per_sample);
        close(fd);
        exit(0);
    }

    if(info.data_id == 0x5453494C) {  // LIST chunk
        read(fd, buf, info.dlen);
        read(fd, &info.data_id, 4);
        read(fd, &info.dlen, 4);
    }
    setSampleRate(info.info.sample_rate);
    uint rd = 0;
    int mp3pid = fork();
    if (mp3pid == 0) {
        exec("decode", argv);
        exit(0);
    }
    while (rd < info.dlen) {
        int len = (info.dlen - rd < BUF_SIZE ? info.dlen - rd : BUF_SIZE);
        i = 0;
        read(fd, buf, len);
        rd += len;
        while(len > SINGLE_BUF_SIZE) {
            kwrite(buf+(i++)*SINGLE_BUF_SIZE, SINGLE_BUF_SIZE);
            len -= SINGLE_BUF_SIZE;
        }
        if(len > 0) {
            kwrite(buf+i*SINGLE_BUF_SIZE, len);
        }
    }

    close(fd);
    kill(mp3pid);
    wait(0);
    exit(0);
}

