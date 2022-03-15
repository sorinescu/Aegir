#ifndef __STRING_UTILS_HPP__
#define __STRING_UTILS_HPP__

#define _strlcpy(dst, src)                  \
    do                                      \
    {                                       \
        strncpy(dst, src, sizeof(dst) - 1); \
        (dst)[sizeof(dst) - 1] = 0;         \
    } while (0)

#endif // __STRING_UTILS_HPP__
