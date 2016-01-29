#ifndef __FPU_H
#define __FPU_H
#include <stdint.h>
uint32_t fadd(uint32_t a, uint32_t b);
uint32_t fmul(uint32_t a, uint32_t b);
int finv_init();
uint32_t finv(uint32_t a);
#endif
