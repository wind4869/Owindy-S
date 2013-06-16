#include "type.h"
#include "keyboard.h"
#include "keymap.h"
#include "kernel.h"
#include "tty.h"

PRIVATE KB_INPUT    kb_in;

PRIVATE bool code_with_E0;
PRIVATE bool shift_l;
PRIVATE bool shift_r;
PRIVATE bool alt_l;
PRIVATE bool alt_r;
PRIVATE bool ctrl_l;
PRIVATE bool ctrl_r;
PRIVATE bool caps_lock;
PRIVATE bool num_lock;
PRIVATE bool scroll_lock;
PRIVATE int column;

PRIVATE u8_t get_byte_from_kbuf();
PRIVATE void set_leds();
PRIVATE void kb_wait();
PRIVATE void kb_ack();

// 键盘中断处理
// 将扫描码放入键盘缓冲队列里，如果队列满直接丢弃
PUBLIC void keyboard_handler(int irq)
{
    u8_t scan_code = inb(0x60);

    if (kb_in.count < KB_IN_BYTES) {
        *(kb_in.p_head) = scan_code;
        kb_in.p_head++;
        if (kb_in.p_head == kb_in.buf + KB_IN_BYTES) {
            kb_in.p_head = kb_in.buf;
        }
        kb_in.count++;
    }
}

// 从键盘缓冲区中取出下一个字节
PUBLIC u8_t get_byte_from_kbuf()
{
    u8_t scan_code;
    
    while (kb_in.count <= 0) {
        ;
    }

    disable_int();
    scan_code = *(kb_in.p_tail);
    kb_in.p_tail++;
    if (kb_in.p_tail == kb_in.buf + KB_IN_BYTES) {
        kb_in.p_tail = kb_in.buf;
    }

    kb_in.count--;
    enable_int();

    return scan_code;
}

// 等待8042的输入缓冲区不忙
PRIVATE void kb_wait()
{
    u8_t kb_stat;

    do {
        kb_stat = inb(KB_CMD);
    } while (kb_stat & 0x02);
}

PRIVATE void kb_ack()
{
    u8_t kb_read;

    do {
        kb_read = inb(KB_DATA);
    } while (kb_read != KB_ACK);
}

PRIVATE void set_leds()
{
    u8_t leds = ((u8_t)caps_lock << 2) | ((u8_t)num_lock << 1) | (u8_t)scroll_lock;
    
    kb_wait();
    outb(KB_DATA, LED_CODE);
    kb_ack();

    kb_wait();
    outb(KB_DATA, leds);
    kb_ack();
}

PUBLIC void keyboard_read(TTY *p_tty)
{
    u8_t scan_code;
    bool make;

    u32_t key = 0;
    u32_t *keyrow;

    if (kb_in.count > 0) {
        code_with_E0 = false;
        scan_code = get_byte_from_kbuf();

        if (scan_code == 0xE1) {
            // For PauseBreak
            int i;
            u8_t pausebrk_scode[] = {0xE1, 0x1D, 0x45, 0xE1, 0x9D, 0xC5};
            bool is_pausebreak = true;
            for (i = 1; i < 6; ++i) {
                if (get_byte_from_kbuf() != pausebrk_scode[i]) {
                    is_pausebreak = 0;
                    break;
                }
            }
            if (is_pausebreak) {
                key = PAUSEBREAK;
            }
        } else if (scan_code == 0xE0) {
            scan_code = get_byte_from_kbuf();

            // For PrintScreen
            if (scan_code == 0x2A) {
                if (get_byte_from_kbuf() == 0xE0) {
                    if (get_byte_from_kbuf() == 0x37) {
                        key = PRINTSCREEN;
                        make = true;
                    }
                }
            }

            if (scan_code == 0xB7) {
                if (get_byte_from_kbuf() == 0xE0) {
                    if (get_byte_from_kbuf() == 0xAA) {
                        key = PRINTSCREEN;
                        make = false;
                    }
                }
            }

            // Not PrintScreen
            if (key == 0) {
                code_with_E0 = true;
            }
        }

        if ((key != PAUSEBREAK) && (key != PRINTSCREEN)) {
            make = (scan_code & FLAG_BREAK ? false : true);

            keyrow = &keymap[(scan_code & 0x7F) * MAP_COLS];

            column = 0;
            
            bool caps = shift_l || shift_r;
            if (caps_lock) {
                if ((keyrow[0] >= 'a') && (keyrow[0] <= 'z')) {
                    caps = !caps;
                }
            }
            if (caps) {
                column = 1;
            }
            if (code_with_E0) {
                column = 2;
            }

            key = keyrow[column];

            switch (key) {
            case SHIFT_L:
                shift_l = make;
                break;
            case SHIFT_R:
                shift_r = make;
                break;
            case CTRL_L:
                ctrl_l = make;
                break;
            case CTRL_R:
                ctrl_r = make;
                break;
            case ALT_L:
                alt_l = make;
                break;
            case ALT_R:
                alt_l = make;
                break;
            case CAPS_LOCK:
                if (make) {
                    caps_lock = !caps_lock;
                    set_leds();
                }
                break;
            case NUM_LOCK:
                if (make) {
                    num_lock = !num_lock;
                    set_leds();
                }
                break;
            case SCROLL_LOCK:
                if (make) {
                    scroll_lock = !scroll_lock;
                    set_leds();
                }
                break;
            default:
                break;
            }

            if (make) {         // 只需处理make-code
                bool pad = false;

                // 小键盘
                if ((key >= PAD_SLASH) && (key <= PAD_9)) {
                    pad = true;
                    switch (key) {
                    case PAD_SLASH:
                        key = '/';
                        break;
                    case PAD_STAR:
                        key = '*';
                        break;
                    case PAD_MINUS:
                        key = '-';
                        break;
                    case PAD_PLUS:
                        key = '+';
                        break;
                    case PAD_ENTER:
                        key = ENTER;
                        break;
                    default:
                        if (num_lock && (key >= PAD_0) && (key <= PAD_9)) {
                            key = key - PAD_0 + '0';
                        } else if (num_lock && (key == PAD_DOT)) {
                            key = '.';
                        } else {
                            switch (key) {
                            case PAD_HOME:
                                key = HOME;
                                break;
                            case PAD_END:
                                key = END;
                                break;
                            case PAD_PAGEUP:
                                key = PAGEUP;
                                break;
                            case PAD_PAGEDOWN:
                                key = PAGEDOWN;
                                break;
                            case PAD_INS:
                                key = INSERT;
                                break;
                            case PAD_UP:
                                key = UP;
                                break;
                            case PAD_DOWN:
                                key = DOWN;
                                break;
                            case PAD_LEFT:
                                key = LEFT;
                                break;
                            case PAD_RIGHT:
                                key = RIGHT;
                                break;
                            case PAD_DOT:
                                key = DELETE;
                                break;
                            default:
                                break;
                            }
                        }
                        break;
                    }
                }
                key |= shift_l ? FLAG_SHIFT_L : 0;
                key |= shift_r ? FLAG_SHIFT_R : 0;
                key |= ctrl_l ? FLAG_CTRL_L : 0;
                key |= ctrl_r ? FLAG_CTRL_R : 0;
                key |= alt_l ? FLAG_ALT_L : 0;
                key |= alt_r ? FLAG_ALT_R : 0;
                key |= pad ? FLAG_PAD : 0;

                in_process(p_tty, key);
            }
        }
    }
}

PUBLIC void init_keyboard()
{
    kb_in.count = 0;
    kb_in.p_head = kb_in.p_tail = kb_in.buf;

    caps_lock = false;
    num_lock = true;
    scroll_lock = false;

    set_leds();

    put_irq_handler(KEYBOARD_IRQ, keyboard_handler);
    enable_irq(KEYBOARD_IRQ);
}


