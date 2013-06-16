
//-------------------------------------------------------
//						type.h
//-------------------------------------------------------

#ifndef	_OWINDYS_TYPE_H_
#define	_OWINDYS_TYPE_H_

#define PUBLIC
#define PRIVATE static
#define IN
#define OUT
#define IN_OUT

#define STR(x) #x
#define STR_EXPAND(x) STR(x)

#define ALIGNED(n) __attribute__((aligned(n)))
#define NOINLINE __attribute__((noinline))
#define PACKED __attribute__((packed))
#define SECTION(name) __attribute__((section(name)))
#define UNUSED __attribute__((unused))

typedef __builtin_va_list va_list;
#define va_start(vargs,last_param) __builtin_va_start(vargs,last_param)
#define va_end(vargs) __builtin_va_end(vargs)
#define va_arg(vargs,arg_type) __builtin_va_arg(vargs,arg_type)

typedef _Bool bool;
#define true (bool)1
#define false (bool)0
static inline const char *bool_str(bool value) {
	return value ? "true" : "false";
}

typedef char				s8_t;
typedef unsigned char		u8_t;
typedef short				s16_t;
typedef	unsigned short		u16_t;
typedef	int					s32_t;
typedef unsigned int		u32_t;
typedef long long			s64_t;
typedef unsigned long long	u64_t;
typedef unsigned int		size_t;

#define NULL (void*)0

typedef void (*task_f) ();
typedef	void (*int_handler)	();
typedef void (*irq_handler) (int irq);

typedef void* system_call;

#endif 
