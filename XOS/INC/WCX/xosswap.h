#define SWAPWORD(val) (((val & 0xFF00) >> 8) | ((val & 0xFF) << 8))
#define SW(val) (((val & 0xFF00) >> 8) | ((val & 0xFF) << 8))

#define SWAPLONG(val) (SWAPWORD(val >> 16) | (SWAPWORD(val) << 16))
#define SL(val) (SWAPWORD(val >> 16) | (SWAPWORD(val) << 16))

#define sw(xx) swapword(xx)
extern ulong swapword (ulong value);
#pragma aux swapword =	\
   "xchg al, ah"	\
    parm [EAX] value [EAX];

#define sl(xx) swaplong(xx)
extern ulong swaplong (ulong value);
#pragma aux swaplong =	\
   "xchg al, ah"	\
   "ror  eax, 16"	\
   "xchg al, ah"	\
    parm [EAX] value [EAX];

