#include "fetch_x2017.h"

int read_byte(FILE *bfile, u_int8_t *bit_ptr, u_int8_t *buffer) {
    if (ftell(bfile) < 1) {
        return -1;
    }
    fseek(bfile, -1, SEEK_CUR);
    fread(buffer, 1, 1, bfile);
    fseek(bfile, -1, SEEK_CUR);
    *bit_ptr = 0;
    return 0;
}

int read_bits(FILE *bfile, int numbers, u_int8_t *result) {
    //read specified number of bits
    static u_int8_t bit_ptr = 8;
    static u_int8_t buffer = 8;

    *result = 0;

    for (int i = 0; i < numbers; i++) {
        if (bit_ptr == 8) {
            //if this byte is finished, we apply for a new byte
            if (read_byte(bfile, &bit_ptr, &buffer) == -1) {
                return -1;
            }
        }
        //read a bit and add it to the result
        *result = *result | ((buffer >> bit_ptr) & 1) << i;
        bit_ptr++;
    }
    return 0;
}

int fetch_addr(FILE *bf, u_int8_t type, u_int8_t *result, int *st, u_int8_t *sc) {
    switch (type) {
        /*
         * Type: VAL
         * need 8 bit of information
         */
        case 0b00:
            if (read_bits(bf, 8, result) == -1) {
                return -1;
            }
            break;

        /*
         * Type: REG
         * need 3 bit of information
         */
        case 0b01:
            if (read_bits(bf, 3, result) == -1) {
                return -1;
            }
            break;

        /*
         * Type: STK and PTR
         * need 5 bit of information
         */
        case 0b10:
        case 0b11:
            if (read_bits(bf, 5, result) == -1) {
                return -1;
            }
            /*
             * Store the reverse order of appearance in the symbol table
             */
            if (st[*result] == -1) {
                //If not exist, initialize it
                st[*result] = *sc;
                *result = *sc;
                *sc += 1;
            } else {
                //If exist, read its order value
                *result = st[*result];
            }
            break;
        default:
            return -1;
    }
    return 0;
}

int fetch_op(FILE *bf, u_int8_t *code, int *st, u_int8_t op, u_int8_t *sc) {
    u_int8_t opcode = op;
    u_int8_t first_v;
    u_int8_t first_t;
    u_int8_t second_v;
    u_int8_t second_t;

    /*
     * operation : MOV, REF, ADD
     * Require two address
     */
    if (opcode == 0b000 || opcode == 0b011 || opcode == 0b100) {
        if (read_bits(bf, 2, &first_t) == -1) {
            return -1;
        }
        fetch_addr(bf, first_t, &first_v, st, sc);

        if (read_bits(bf, 2, &second_t) == -1) {
            return -1;
        }
        fetch_addr(bf, second_t, &second_v, st, sc);

        code[1] = first_t;
        code[2] = first_v;
        code[3] = second_t;
        code[4] = second_v;
        code[5] = 1;

    /*
     * operation : CAL, PRINT, NOT, EUQ
     * Require one address
     */
    } else if (opcode == 0b001 || opcode == 0b101 || opcode == 0b110 ||
               opcode == 0b111) {
        if (read_bits(bf, 2, &first_t) == -1) {
            return -1;
        }
        fetch_addr(bf, first_t, &first_v, st, sc);

        code[1] = first_t;
        code[2] = first_v;
        code[5] = 1;

    /*
     * operation : RET
     * Require no address
     */
    } else if (opcode == 0b010) {
        code[5] = 1;
    }
    return 0;
}
