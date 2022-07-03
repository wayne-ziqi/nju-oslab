#include "types.h"
#include "const.h"
#include "mmemory.h"

void *mmemcpy(void *dst, const void *src, int size)
{
    if (dst == NULL || src == NULL || size <= 0)
        return NULL;

    uint8_t *pdst = (uint8_t *)dst;
    uint8_t *psrc = (uint8_t *)src;

    if (pdst > psrc && pdst < psrc + size) // self copy
    {
        pdst = pdst + size - 1;
        psrc = psrc + size - 1;
        while (size--)
            *pdst-- = *psrc--;
    }
    else
    {
        while (size--)
            *pdst++ = *psrc++;
    }
    return dst;
}

void *mmemset(void *dst, uint8_t set, int size)
{
    if (dst == NULL)
    {
        return NULL;
    }
    uint8_t *pdst = (uint8_t *)dst;
    while (size-- > 0)
    {
        *pdst++ = set;
    }
    return dst;
}
