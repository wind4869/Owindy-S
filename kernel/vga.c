#include "vga.h"

struct chr_cell {
    s8_t chr;
    u8_t fcolor : 4;
    u8_t bcolor : 4;
};

PRIVATE int frame_top, frame_left;
PRIVATE int frame_height, frame_width;
PRIVATE int frame_fcolor, frame_bcolor;
PRIVATE int caret_row, caret_col;
PRIVATE bool cursor;

PRIVATE inline volatile struct chr_cell *get_chr_cell(int row, int col) {
    return (struct chr_cell*)VIDEO_ADDR + row * VGA_COLS_NUMBER + col;
}

void get_vga_cell(int row, int col, char *chr, int *fore_color,
        int *back_color) {
    volatile struct chr_cell *cell = get_chr_cell(row, col);
    if (chr)
        *chr = cell->chr;
    if (fore_color)
        *fore_color = cell->fcolor;
    if (back_color)
        *back_color = cell->bcolor;
}

void set_vga_cell(int row, int col, char chr, int fore_color, int back_color) {
    *get_chr_cell(row, col) = (struct chr_cell) {
        chr, (u8_t)fore_color, (u8_t)back_color
    };
}

int get_vga_frame_top(void) {
    return frame_top;
}

int get_vga_frame_left(void) {
    return frame_left;
}

int get_vga_frame_height(void) {
    return frame_height;
}

int get_vga_frame_width(void) {
    return frame_width;
}

int get_vga_frame_fcolor(void) {
    return frame_fcolor;
}

int get_vga_frame_bcolor(void) {
    return frame_bcolor;
}

void set_vga_frame(int top, int left, int height, int width, int fcolor,
        int bcolor) {
    frame_top = top, frame_left = left, frame_height = height,
    frame_width = width, frame_fcolor = fcolor, frame_bcolor = bcolor;
}

void clear_vga_frame(void) {
    for (int row = 0; row < frame_height; row++)
        for (int col = 0; col < frame_width; col++)
            *get_chr_cell(frame_top + row, frame_left + col) =
                (struct chr_cell) { 0, frame_fcolor, frame_bcolor };
}

bool get_vga_cursor(void) {
    return cursor;
}

PRIVATE void put_cursor(int row, int col) {
    int off = row * VGA_COLS_NUMBER + col;
    outb(VGA_ADDR_REG, CURSOR_L);
    outb(VGA_DATA_REG, (unsigned char)(off & 0xFF));
    outb(VGA_ADDR_REG, CURSOR_H);
    outb(VGA_DATA_REG, (unsigned char)((off >> 8) & 0xFF));
}

PRIVATE void set_vga_cursor(bool visible) {
    cursor = visible;
    if (visible) {
        outw(VGA_ADDR_REG,0xE0A);
        outw(VGA_ADDR_REG,0xF0B);
        put_cursor(frame_top + caret_row, frame_left + caret_col);
    }
    else {
        outw(VGA_ADDR_REG,0x200A);
        outw(VGA_ADDR_REG,0xB);
        put_cursor(VGA_ROWS_NUMBER, 0);
    }
}

int get_vga_caret_row(void) {
    return caret_row;
}

int get_vga_caret_col(void) {
    return caret_col;
}

void set_vga_caret(int row, int col) {
    caret_row = row, caret_col = col;
    if (cursor)
        put_cursor(frame_top + row, frame_left + col);
}

PUBLIC void init_vga(void) {
    set_vga_frame(0, 0, VGA_ROWS_NUMBER, VGA_COLS_NUMBER, VGA_COLOR_LOW_GREEN,
            VGA_COLOR_BLACK);
    clear_vga_frame();
    set_vga_cursor(true);
}

PRIVATE void scroll_frame(void) {
    for (int row = 1; row < frame_height; row++)
        for (int col = 0; col < frame_width; col++)
            *get_chr_cell(frame_top + row - 1, frame_left + col) =
                *get_chr_cell(frame_top + row, frame_left + col);

    for (int col = 0; col < frame_width; col++)
        *get_chr_cell(frame_top + frame_height - 1, frame_left + col) =
            (struct chr_cell) { 0, frame_fcolor, frame_bcolor };
}

PRIVATE void put_char_color(char chr, int fcol, int bcol) {
    switch (chr) {
    case '\r':
        caret_col = 0;
        break;
    case '\b':
        if (caret_col > 0) {
            caret_col--;
            *get_chr_cell(frame_top + caret_row, frame_left + caret_col) = (struct chr_cell) {' ', frame_bcolor, frame_bcolor};
        }
        break;
    case '\n':
new_line:
        caret_col = 0, caret_row++;
        if (caret_row == frame_height) {
            scroll_frame();
            caret_row--;
        }
        break;
    default:
        *get_chr_cell(frame_top + caret_row, frame_left + caret_col) = 
            (struct chr_cell) {chr, fcol, bcol};
        if (++caret_col == frame_width)
            goto new_line;
        break;
    }
}

PRIVATE void put_char(char chr) {
/*
 *    switch (chr) {
 *        case '\r':
 *            caret_col = 0;
 *            break;
 *        case '\b':
 *            if (caret_col > 0) {
 *                caret_col--;
 *                *get_chr_cell(frame_top + caret_row, frame_left + caret_col) = (struct chr_cell) {' ', frame_bcolor, frame_bcolor};
 *            }
 *            break;
 *        case '\n':
 *new_line:
 *            caret_col = 0, caret_row++;
 *            if (caret_row == frame_height) {
 *                scroll_frame();
 *                caret_row--;
 *            }
 *            break;
 *            // TODO: implement other escapes
 *        default:
 *            *get_chr_cell(frame_top + caret_row, frame_left + caret_col) =
 *                (struct chr_cell) { chr, frame_fcolor, frame_bcolor };
 *            if (++caret_col == frame_width)
 *                goto new_line;
 *            break;
 *    }
 */
    put_char_color(chr, frame_fcolor, frame_bcolor);
}

PUBLIC int kputchar(int chr) {
    put_char(chr);
    if (cursor)
        put_cursor(frame_top + caret_row, frame_left + caret_col);
    return chr;
}

PUBLIC int kputchar_color(int chr, int color) {
    put_char_color(chr, color, frame_bcolor);
    if (cursor)
        put_cursor(frame_top + caret_row, frame_left + caret_col);
    return chr;
}

PUBLIC int kprintf(const char *format, ...) {

    va_list vargs;
    va_start(vargs, format);

    int num = 0;
    char chr, *str, buf[20];
    long lint;

    struct attributes {
        u32_t size_long : 1;
    } attrs;

    while ((chr = *format++))
        if (chr == '%') {
            memset(&attrs, 0, sizeof(attrs));

next_attr_type:
            switch ((chr = *format++)) {
                case 'l':
                    attrs.size_long = true;
                    goto next_attr_type;

                case '%':
                    put_char('%');
                    num++;
                    break;

                case 's':
                    str = va_arg(vargs, char*);
puts:
                    while (*str) {
                        put_char(*str++);
                        num++;
                    }
                    break;

                case 'd':
                    lint = va_arg(vargs, long);
                    if (!attrs.size_long)
                        lint = (int)lint;
                    if (lint < 0) {
                        put_char('-');
                        lint = -lint;
                        num++;
                    }
                    str = ultoa(lint, buf, 10);
                    goto puts;

                case 'x':
                case 'X':
                    lint = va_arg(vargs, long);
                    if (!attrs.size_long)
                        lint = (unsigned int)lint;
                    str = ultoa(lint, buf, chr == 'x' ? 16 : -16);
                    goto puts;

                case 'c':
                    buf[0] = (char)va_arg(vargs, int), buf[1] = 0, str = buf;
                    goto puts;

                    // TODO: implement other types and attributes
            }
        }
        else {
            put_char(chr);
            num++;
        }
    va_end(vargs);

    if (cursor)
        put_cursor(frame_top + caret_row, frame_left + caret_col);

    return num;
}
