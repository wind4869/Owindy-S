#ifndef VGA_H
#define VGA_H

#include "type.h"
#include "util.h"

#define VGA_COLOR_BLACK 0
#define VGA_COLOR_LOW_BLUE 1
#define VGA_COLOR_LOW_GREEN 2
#define VGA_COLOR_LOW_CYAN 3
#define VGA_COLOR_LOW_RED 4
#define VGA_COLOR_LOW_MAGENTA 5
#define VGA_COLOR_BROWN 6
#define VGA_COLOR_LIGHT_GRAY 7
#define VGA_COLOR_DARK_GRAY 8
#define VGA_COLOR_HIGH_BLUE 9
#define VGA_COLOR_HIGH_GREEN 10
#define VGA_COLOR_HIGH_CYAN 11
#define VGA_COLOR_HIGH_RED 12
#define VGA_COLOR_HIGH_MAGENTA 13
#define VGA_COLOR_HIGH_YELLOW 14
#define VGA_COLOR_WHITE 15

#define VGA_ROWS_NUMBER 25
#define VGA_COLS_NUMBER 80

#define VGA_ADDR_REG    0x3D4
#define VGA_DATA_REG    0x3D5
#define START_ADDR_H    0xC
#define START_ADDR_L    0xD
#define CURSOR_H        0xE
#define CURSOR_L        0xF

#define VIDEO_ADDR      0xB8000
#define VIDEO_SIZE      0x8000

// low level API (no concern of frames)
PRIVATE void get_vga_cell(int row, int col, char *chr, int *fore_color,
                  int *back_color);
PRIVATE void set_vga_cell(int row, int col, char chr, int fore_color, int back_color);

int get_vga_frame_top(void);
int get_vga_frame_left(void);
int get_vga_frame_height(void);
int get_vga_frame_width(void);
int get_vga_frame_fcolor(void);
int get_vga_frame_bcolor(void);
void set_vga_frame(int top, int left, int height, int width, int fcolor,
                   int bcolor);
void clear_vga_frame(void);

PRIVATE bool get_vga_cursor(void);
PRIVATE void set_vga_cursor(bool visible);

int get_vga_caret_row(void);
int get_vga_caret_col(void);
void set_vga_caret(int row, int col);

PUBLIC void init_vga(void);

#endif // VGA_H
