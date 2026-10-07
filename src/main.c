/* Includes */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

/* Consts */
#define HEADER_SIZE 14
#define INFOHEADER_SIZE 40
#define COLORTABLE_SIZE NUM_COLORS*4
#define WIDTH 10000
#define HEIGHT 10000
#define PIXELS_PER_M 1
#define NUM_COLORS 16
#define BPP 4 /* NOTE: must correspond to NUM_COLORS */
#define PIXELDATA_SIZE (WIDTH*HEIGHT*BPP)/8

/* Colour table */
static uint8_t color_table[] = { /* r, g, b, 0 */
/*
  0x00, 0x00, 0x00, 0x00,
  0x11, 0x11, 0x11, 0x00,
  0x22, 0x22, 0x22, 0x00,
  0x33, 0x33, 0x33, 0x00,

  0x44, 0x44, 0x44, 0x00,
  0x55, 0x55, 0x55, 0x00,
  0x66, 0x66, 0x66, 0x00,
  0x77, 0x77, 0x77, 0x00,

  0x88, 0x88, 0x88, 0x00,
  0x99, 0x99, 0x99, 0x00,
  0xaa, 0xaa, 0xaa, 0x00,
  0xbb, 0xbb, 0xbb, 0x00,

  0xcc, 0xcc, 0xcc, 0x00,
  0xdd, 0xdd, 0xdd, 0x00,
  0xee, 0xee, 0xee, 0x00,
  0xff, 0xff, 0xff, 0x00,
*/
  0x29, 0x2f, 0x56, 0x00,
  0x29, 0x3b, 0x66, 0x00,
  0x26, 0x48, 0x74, 0x00,
  0x1f, 0x55, 0x82, 0x00,
  0x0b, 0x64, 0x8e, 0x00,
  0x00, 0x73, 0x98, 0x00,
  0x00, 0x82, 0xa0, 0x00,
  0x00, 0x91, 0xa5, 0x00,
  0x00, 0xa0, 0xa7, 0x00,
  0x00, 0xaf, 0xa8, 0x00,
  0x00, 0xbf, 0xa6, 0x00,
  0x00, 0xcd, 0xa1, 0x00,
  0x2c, 0xda, 0x99, 0x00,
  0x5e, 0xe6, 0x8e, 0x00,
  0x86, 0xf1, 0x80, 0x00,
  0xac, 0xfa, 0x70, 0x00,
};
/* Pixel data */
static uint8_t pixel_data[PIXELDATA_SIZE];

/* Fragment shader, so to speak */
uint8_t getpixel(size_t x, size_t y) {
  uint8_t res = 0; /* Can only use low 4 bits */

  const double c_real = -1;
  const double c_imm = 0;
  const double startx = 0.0f;
  const double starty = 0.0f;
  const double scale = 0.5f;

  double real = startx + ((double)x/WIDTH)*((double)WIDTH/HEIGHT) * scale;
  double imm = starty + (double)y/HEIGHT * scale;

  for (size_t i = 0; i < 16; i++) {
    double new_real = real*real - imm*imm + c_real;
    double new_imm = 2*imm*real + c_imm;
    real = new_real;
    imm = new_imm;
    if (real*real + imm*imm > 4) return i;
  }

  return 0x0;
}

/* Entry point */
int main(int argc, char *argv[]) {
  FILE *f = fopen("out/test.bmp", "w+");
  size_t size =
      HEADER_SIZE + INFOHEADER_SIZE + COLORTABLE_SIZE + PIXELDATA_SIZE;
  size_t cur = 0;
  uint8_t *bmpfile = malloc(size);

  /* -- DRAWING -- */
  for (size_t i = 0; i < PIXELDATA_SIZE; i++) {
    //pixel_data[i] = (uint8_t)(rand()&0xff);
    pixel_data[i] = getpixel((i*2)%WIDTH, (i*2)/WIDTH)
        | getpixel((i*2+1)%WIDTH, (i*2+1)/WIDTH);
  }
  
  /* -- HEADER -- */
  memcpy(bmpfile+cur, "BM", 2); cur+=2;
  memcpy(bmpfile+cur, &size, 4); cur+=4;
  memset(bmpfile+cur, 0, 4); cur+=4;
  uint32_t data_offset = HEADER_SIZE + INFOHEADER_SIZE + COLORTABLE_SIZE;
  memcpy(bmpfile+cur, &data_offset, 4); cur+=4;

  /* -- INFOHEADER -- */
  const uint32_t infoheader_size = INFOHEADER_SIZE;
  const uint32_t width = WIDTH;
  const uint32_t height = HEIGHT;
  const uint16_t planes = 1;
  const uint16_t bpp = BPP;
  const uint32_t compression = 0;
  const uint32_t pixels_per_m = PIXELS_PER_M;
  const uint32_t num_colors = NUM_COLORS;
  memcpy(bmpfile+cur, &infoheader_size, 4); cur+=4;
  memcpy(bmpfile+cur, &width, 4); cur+=4;
  memcpy(bmpfile+cur, &height, 4); cur+=4;
  memcpy(bmpfile+cur, &planes, 2); cur+=2;
  memcpy(bmpfile+cur, &bpp, 2); cur+=2;
  memcpy(bmpfile+cur, &compression, 4); cur+=4;
  memset(bmpfile+cur, 0, 4); cur+=4;
  memcpy(bmpfile+cur, &pixels_per_m, 4); cur+=4; /* Horizontal */
  memcpy(bmpfile+cur, &pixels_per_m, 4); cur+=4; /* Vertical */
  memcpy(bmpfile+cur, &num_colors, 4); cur+=4;
  memset(bmpfile+cur, 0, 4); cur+=4;

  /* -- COLORTABLE -- */
  memcpy(bmpfile+cur, color_table, COLORTABLE_SIZE); cur+=COLORTABLE_SIZE;

  /* -- PIXELDATA -- */
  memcpy(bmpfile+cur, pixel_data, PIXELDATA_SIZE); cur+= PIXELDATA_SIZE;

  fwrite(bmpfile, size, 1, f);
  free(bmpfile);
  fclose(f);
  return 0;
}
