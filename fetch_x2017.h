#ifndef HEADER_FILE
#define HEADER_FILE

#include <stdio.h>

#define u_int8_t unsigned char

int read_byte(FILE *bfile, u_int8_t *bit_ptr, u_int8_t *buffer);
int read_bits(FILE *bfile, int numbers, u_int8_t *result);
int fetch_addr(FILE *bf, u_int8_t type, u_int8_t *result, int *st, u_int8_t *sc);
int fetch_op(FILE *bf, u_int8_t *code, int *st, u_int8_t op, u_int8_t *sc);

#endif