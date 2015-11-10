#include <string.h>

/* some systems do not have newest memcpy@@GLIBC_2.14 - stay with old good one */
void *__memcpy_glibc_2_2_5(void* ,const void* ,size_t);

asm (".symver __memcpy_glibc_2_2_5, memcpy@GLIBC_2.2.5");
void *__wrap_memcpy(void *dest, const void *src, size_t n)
{
    return __memcpy_glibc_2_2_5(dest, src, n);
}

//__asm__(".symver __fdelt_chk,__fdelt_chk@GLIBC_2.2.5");
//void *__wrap_fdelt_chk(void *dest, const void *src, size_t n)
//{
//    return fdelt_chk(dest, src, n);
//}
