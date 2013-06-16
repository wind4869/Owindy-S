#ifndef _OWINDYS_UTIL_H_
#define _OWINDYS_UTIL_H_

// 线性地址->物理地址转换的宏
#define vir2phys(seg_base, vir)	(u32_t)(((u32_t)seg_base) + (u32_t)(vir))

static inline void disable_int() {
    __asm__ __volatile__ (
            "cli"
            );
}

static inline void enable_int() {
    __asm__ __volatile__ (
            "sti"
            );
}

static inline u8_t inb(u16_t port) {
	u8_t val;
	__asm__ __volatile__ (
			"inb %%dx, %%al" : "=a"(val) : "d"(port)
			);
	return val;
}

static inline u16_t inw(u16_t port) {
	u16_t val;
	__asm__ __volatile__ (
			"inw %%dx, %%al" : "=a"(val) : "d"(port)
			);
	return val;
}

static inline void outb(u16_t port, u8_t val) {
	__asm__ __volatile__ (
			"outb %%al, %%dx" : : "a"(val), "d"(port)
			);
}

static inline void outw(u16_t port, u16_t val) {
	__asm__ __volatile__ (
			"outw %%ax, %%dx" : : "a"(val), "d"(port)
			);
}

static inline u64_t rdmsr(u32_t msr) {
	u32_t low, high;
	__asm__ __volatile__ (
			"rdmsr" : "=a"(low), "=d"(high) : "c"(msr)
			);
	return (((u64_t)high << 32) | low);
}

static inline void wrmsr(u32_t msr, u64_t val) {
	u32_t low = (u32_t)val, high = (u32_t)(val >> 32);
	__asm__ __volatile__ (
			"wrmsr" : : "a"(low), "d"(high), "c"(msr)
		);
}

PUBLIC void disp_str(char *pstr);

PUBLIC char *strcat(char *dst, const char *src);
PUBLIC char *strcpy(char *dst, const char *src);
PUBLIC size_t strlen(const char *str);
PUBLIC char *strrev(char *str);

PUBLIC void *memset(void *ptr, int value, size_t size);
PUBLIC void *memcpy(void *dst, const void *src, size_t size);
PUBLIC int memcmp(const void *buf1, const void *buf2, size_t size);
PUBLIC void *memmem(const void *buf1, size_t size1, const void *buf2, size_t size2);

PUBLIC char *ultoa(unsigned long val, char *buf, int radix);
PUBLIC int kputchar (int chr);
PUBLIC int kputchar_color (int chr, int color);
PUBLIC int kprintf (const char *format, ...);

PUBLIC int vsprintf (char *buf, const char *fmt, va_list args);
PUBLIC int sprintf (char *buf, const char *fmt, ...);
PUBLIC int printf (const char *fmt, ...);
PUBLIC int puts (const char *buf);

#endif // UTIL_H
