#ifndef COMMON_H
#define COMMON_H

typedef enum 
{
    UTIL_OK = 0,

    UTIL_ERR_ALLOC,
    UTIL_ERR_NOT_FOUND,
    UTIL_ERR_OUT_OF_RANGE,

    UTIL_ITER_END
} util_err_t;

#endif // COMMON_H