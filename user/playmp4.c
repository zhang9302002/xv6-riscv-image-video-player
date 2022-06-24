#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "user/font.h"

struct RGB_Header {
    uint16 type;
    uint8 compression;      // 0 = uncompressed, 1 = RLE compressed
    uint8 bytesPerPixel;    // 1 = 8 bit, 2 = 16 bit
    uint16 dimension;
    uint16 width;
    uint16 height;
    uint16 channels;
    uint32 minPixelValue;
    uint32 maxPixelValue;
    uint32 reserved;
    char name[80];
    uint32 colorMapID;
    char dummy[404];
}header;

#define WINDOW_WIDTH 320
#define WINDOW_HEIGHT 200
#define max(x, y) (((x) > (y)) ? (x) : (y))
#define min(x, y) (((x) < (y)) ? (x) : (y))

int update;
int fd;
int n;
uint16 buf[WINDOW_HEIGHT * WINDOW_WIDTH];
char fbuf[WINDOW_HEIGHT][WINDOW_WIDTH];

void loadVideo(char name[]) {
    fd = open(name, O_RDONLY);
    if(fd < 0) {
        printf("viewer: cannot open %s\n", name);
        exit(1);
    }
}

void draw() {
    if((n = read(fd, buf, sizeof(buf))) > 0) {
        for(int i = 0; i < WINDOW_HEIGHT; ++i)
            for(int j = 0; j < WINDOW_WIDTH; ++j) {
                int pos = i * WINDOW_WIDTH + j;
                uint16 val = buf[pos];
                // format rrrr rggg gggb bbbb
                int r = (int)((val & 0xF800) >> (6 + 5 + 3));
                int g = (int)((val & 0x07E0) >> (5 + 3));
                int b = (int)((val & 0x001E) >> (2));
                fbuf[i][j] = (char)((r << 6) | (g << 3) | b);
            }
    } else {
        exit(0);
    }
}

void key_handle(uint64 key0, uint64 key1) {
    int step = 10;

    update = 1;
    cb_return();
}

int main(int argc, char *argv[]) {
    if(argc < 2){
        printf("Usage: playmp4 *.rgb\n");
        exit(1);
    }
    loadVideo(argv[1]);

    int len = strlen(argv[1]);
    argv[1][len - 3] = 'w';
    argv[1][len - 2] = 'a';
    argv[1][len - 1] = 'v';

    int pid = fork();
    if(pid == 0) {
        sleep(8);
        exec("playwav", argv);
        exit(0);
    }

    update = 1;
    reg_keycb(key_handle);
    int frame = 0;
    while(1) {
        if(update) {
            draw();
            show_window((char *) fbuf);
            update = 0;
        }
        sleep(1);
        update = 1;
    }
    close_window();
    exit(0);
}