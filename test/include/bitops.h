#include <stdio.h>
#include <stdlib.h>
#include <stdtypes.h>


#define le64_to_cpu(x) ((UINT64)(                         \
          (((UINT64)(x) & (UINT64)0x00000000000000ffULL) << 56) |   \
         (((UINT64)(x) & (UINT64)0x000000000000ff00ULL) << 40) |   \
         (((UINT64)(x) & (UINT64)0x0000000000ff0000ULL) << 24) |   \
         (((UINT64)(x) & (UINT64)0x00000000ff000000ULL) <<  8) |   \
         (((UINT64)(x) & (UINT64)0x000000ff00000000ULL) >>  8) |   \
          (((UINT64)(x) & (UINT64)0x0000ff0000000000ULL) >> 24) |   \
          (((UINT64)(x) & (UINT64)0x00ff000000000000ULL) >> 40) |   \
          (((UINT64)(x) & (UINT64)0xff00000000000000ULL) >> 56)))
                                                     
#define SWAPENDIAN_32(x) ((UINT32)(                         \
           (((UINT32)(x) & (UINT32)0x000000ffUL) << 24) |            \
           (((UINT32)(x) & (UINT32)0x0000ff00UL) <<  8) |            \
           (((UINT32)(x) & (UINT32)0x00ff0000UL) >>  8) |            \
           (((UINT32)(x) & (UINT32)0xff000000UL) >> 24)))
                                                     
                                                     
          
#define dummy(x)    (x)                                                     
#define be32_to_cpu(x)                          SwapEndian_32(x)
#define cpu_to_le32(x)                           dummy(x)

#define MAX_READ_REQUEST_SZ_SHIFT   12
#define BITS_PER_LONG     32
#define BITS_PER_BYTE     8


#define BITOP_WORD(nr)          ((nr) / BITS_PER_LONG)

#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))
#define BITS_TO_LONGS(nr)       DIV_ROUND_UP(nr, BITS_PER_BYTE * sizeof(long))



static inline const unsigned long cpu_to_be32p(unsigned long val)
{
          asm volatile ("bswapl %0" : "=r" (val) : "0" (val));
          return val;
}

/**
 * __ffs - find first set bit in word
  * @word: The word to search
  *
  * Undefined if no bit exists, so code should check against 0 first.
  */
 static inline unsigned long __ffs(unsigned long word)
 {
         asm volatile("bsf %1,%0"
                 : "=r" (word)
                 : "rm" (word));
         return word;
 }
 
 /**
  * ffz - find first zero bit in word
  * @word: The word to search
  *
  * Undefined if no zero exists, so code should check against ~0UL first.
  */
 static inline unsigned long ffz(unsigned long word)
 {
         asm volatile ("bsf %1,%0"
                 : "=r" (word)
                 : "r" (~word));
         return word;
 }
 

static inline void set_bit(unsigned long nr, volatile void * addr)
  {
          int *m = ((int *) addr) + (nr >> 5);  
          *m |= 1 << (nr & 31);
  }


static inline unsigned long find_first_zero_bit(const unsigned long *addr, unsigned long size)
 {
         const unsigned long *p = addr;
         unsigned long result = 0;
         unsigned long tmp;
 
         while (size & ~(BITS_PER_LONG-1)) {
                 if (~(tmp = *(p++)))
                         goto found;
                 result += BITS_PER_LONG;
                 size -= BITS_PER_LONG;
         }
         if (!size)
                 return result;
 
         tmp = (*p) | (~0UL << size);
         if (tmp == ~0UL)        /* Are any bits zero? */
                 return result + size;   /* Nope. */
 found:
         return result + ffz(tmp);
 }

static inline void clear_bit(int nr, unsigned long *addr)
{
         addr[nr / BITS_PER_LONG] &= ~(1UL << (nr % BITS_PER_LONG));
}

static inline unsigned long find_first_bit(const unsigned long *addr, unsigned long size)
{
        const unsigned long *p = addr;
        unsigned long result = 0;
        unsigned long tmp;

       while (size & ~(BITS_PER_LONG-1)) 
       {
                if ((tmp = *(p++)))
                        goto found;
                result += BITS_PER_LONG;
                size -= BITS_PER_LONG;
       }
        
       if (!size)
               return result;

       tmp = (*p) & (~0UL >> (BITS_PER_LONG - size));
        
       if (tmp == 0UL)         /* Are any bits set? */
             return result + size;   /* Nope. */
                
  found:
        return result + __ffs(tmp);
}

static inline unsigned long find_next_bit(const unsigned long *addr, unsigned long size, unsigned long offset)
 {
         const unsigned long *p = addr + BITOP_WORD(offset);
        unsigned long result = offset & ~(BITS_PER_LONG-1);
        unsigned long tmp;
         if (offset >= size)
                return size;
         size -= result;
         offset %= BITS_PER_LONG;
         if (offset) {
                 tmp = *(p++);
                 tmp &= (~0UL << offset);
                 if (size < BITS_PER_LONG)
                         goto found_first;
                 if (tmp)
                         goto found_middle;
                 size -= BITS_PER_LONG;
                 result += BITS_PER_LONG;
         }
         while (size & ~(BITS_PER_LONG-1)) {
                 if ((tmp = *(p++)))
                         goto found_middle;
                 result += BITS_PER_LONG;
                 size -= BITS_PER_LONG;
         }
         if (!size)
                 return result;
         tmp = *p;
 
 found_first:
         tmp &= (~0UL >> (BITS_PER_LONG - size));
         if (tmp == 0UL)         /* Are any bits set? */
                 return result + size;   /* Nope. */
 found_middle:
         return result + __ffs(tmp);
 }
 
 
 
 #define BITOP_ADDR(x) "+m" (*(volatile long *) (x))
 #define ADDR_BITOP        BITOP_ADDR(addr)

 
 static inline int test_and_set_bit(int nr, volatile unsigned long *addr)
 {
         int oldbit=0;
 
         asm volatile(" " "bts %2,%1\n\t"
                      "sbb %0,%0" : "=r" (oldbit), ADDR_BITOP : "Ir" (nr) : "memory");
                      
         return oldbit;
 }




static inline unsigned char is_power_of_2(unsigned long n)
{
        return (n != 0 && ((n & (n - 1)) == 0));
}


#define mb()    asm volatile("mfence" : : : "memory")

static inline UINT32 SwapEndian_32(const UINT32 DWord)
{

        return SWAPENDIAN_32(DWord);  
}


static inline int fls(int x)
{
	int r=0;
    asm volatile("bsrl %1,%0\n\t"
        "jnz 1f\n\t"
        "movl $-1,%0\n"
        "1:" : "=r" (r) : "rm" (x));
            
         return r + 1;
 }


static inline unsigned fls_long(unsigned long l)
{
        if (sizeof(l) == 4)
               return fls(l);
}

static inline __attribute__((const)) unsigned long roundup_pow_of_two(unsigned long n)
{
       return 1UL << (fls_long(n) - 1);
}


static inline unsigned long __fls(unsigned long word)
{
    int num = BITS_PER_LONG - 1;

#if BITS_PER_LONG == 64
    if (!(word & (~0ul << 32))) {
        num -= 32;
        word <<= 32;
    }
#endif
    if (!(word & (~0ul << (BITS_PER_LONG-16)))) {
        num -= 16;
        word <<= 16;
    }
    if (!(word & (~0ul << (BITS_PER_LONG-8)))) {
        num -= 8;
        word <<= 8;
    }
    if (!(word & (~0ul << (BITS_PER_LONG-4)))) {
        num -= 4;
        word <<= 4;
    }
    if (!(word & (~0ul << (BITS_PER_LONG-2)))) {
        num -= 2;
        word <<= 2;
    }
    if (!(word & (~0ul << (BITS_PER_LONG-1))))
        num -= 1;
    return num;
}


static inline void cache_off(void)
{
        asm(
        "push %eax\n\t"
        "movl %cr0,%eax\n\t"
                "orl $0x40000000,%eax\n\t"  /* Set CD */
                "movl %eax,%cr0\n\t"
        "wbinvd\n\t"
        "pop  %eax\n\t");
}
static inline void cache_on(void)
{
        asm(
        "push %eax\n\t"
        "movl %cr0,%eax\n\t"
        "andl $0x9fffffff,%eax\n\t" /* Clear CD and NW */
        "movl %eax,%cr0\n\t"
        "pop  %eax\n\t");
}


