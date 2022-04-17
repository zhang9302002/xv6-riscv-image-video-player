#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "user/font.h"

#define BACKGROUND_COLOR 0xf0
#define PAD_COLOR 0xff
#define WINDOW_WIDTH 100
#define WINDOW_HEIGHT 95
#define PADDING_SIZE 1

char fbuf[WINDOW_HEIGHT][WINDOW_WIDTH];
char __attribute((unused)) discard;
char number[7] = {'0', '0', '0', '0', '0', '0', 0};

static void draw_background() {
  for (int i = 0; i < WINDOW_HEIGHT; i++) {
    for (int j = 0; j < WINDOW_WIDTH; j++) {
        int x = i/6;
        int y = j/6;
        int z = x << 4 | y;
      fbuf[i][j] = z;
    }
  }
  return;
}

#ifdef ZZZ
static void draw_padding() {
  for (int i = 0; i < WINDOW_HEIGHT; i++) {
    for (int j = 0; j < WINDOW_WIDTH; j++) {
      if (i < PADDING_SIZE || i + PADDING_SIZE >= WINDOW_HEIGHT ||
          j < PADDING_SIZE || j + PADDING_SIZE >= WINDOW_WIDTH) {
          fbuf[i][j] = PAD_COLOR;
      }
    }
  }
  return;
}
void draw_char(char c, int x, int y) {
  for (int row = 0; row < 8; row++) {
    for (int col = 0; col < 5; col++) {
      if (VGA_FONT[c * 8 + row] & (1 << (7 - col))) {
        fbuf[(row + y)][(col + x)] = 0x00;
      }
    }
  }
}

void draw_text(char * text) {
  int n = 7;
  int char_width = 5;
  int char_height = 8;
  int pad = 1;
  int x0 = WINDOW_WIDTH / 2 - n / 2 * (char_width + pad);
  int y0 = WINDOW_HEIGHT / 2 - char_height;
  int pos = x0;

  for (char *c = text; *c != 0; c++) {
    draw_char(*c, pos, y0 + 5);
    pos += 5;
  }
}

static void update_number() {
  int x = 5;
  while (x >= 0 && number[x] == '9') {
    number[x] = '0';
    x--;
  }
  if (x >= 0) {
    number[x]++;
  }
}
#endif
int main(void) {

    draw_background();
    show_window((char*) fbuf);

    read(0, &discard, 1);
    close_window();
    exit(0);
}
