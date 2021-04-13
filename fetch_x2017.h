#ifndef HEADER_FILE
#define HEADER_FILE

#include <stdio.h>
#include <stdlib.h>

int readbyte(FILE *bfile, u_int8_t *bit_ptr, u_int8_t *buffer);
int readbits(FILE *bfile, int numbers, u_int8_t *result);
int fetch_addr(FILE *bf, u_int8_t type, u_int8_t *result, int8_t *st, u_int8_t *sc);
int fetch_op(FILE *bf, u_int8_t *code, int8_t *st, u_int8_t op, u_int8_t *sc);

#endif