#include <string.h>
#include "fetch_x2017.h"

int fetch_code(FILE *bf,int (*st)[32],int (*ft)[2], u_int8_t (*code)[32][6]) {
    /*
     * Function: fetch_code
     * --------------------
     * Fetch all codes in the binary file and store it in the array provided
     *
     * @Param:
     * bf: file pointer
     * st: symbol table
     * ft: function table
     * code: code space
     *
     * returns:
     * -1 when error happened
     *  0 for normal cases
     */
    u_int8_t ins_num;
    u_int8_t opcode;
    u_int8_t func_label;
    u_int8_t stack_counter;
    int counter = 0;
    while (ftell(bf) > 0 && counter < 8) {
        /*
         * If their are any unread byte, continue the loop
         * unless we already read 8 functions
         */
        if (read_bits(bf, 5, &ins_num) == -1) {
            return -1;
        }

        stack_counter = 0;

        for (int i = ins_num - 1; i >= 0; i--) {
            if (read_bits(bf, 3, &opcode) == -1) {
                return -1;
            }
            // Store codes by their appearance order
            code[counter][i][0] = opcode;
            fetch_op(bf, &code[counter][i][0], &st[counter][0], opcode, &stack_counter);
        }
        if (read_bits(bf, 3, &func_label) == -1) {
            return -1;
        }
        // Update Function Table by their appearance order
        ft[counter][0] = func_label;
        ft[counter][1] = ins_num;
        counter++;
    }
    if (ftell(bf) > 1) {
        /*
         * After we finish reading the file,
         * if the remaining file size still greater than 1,
         * raise error.
         */
        return -1;
    }
    return 0;
}

/*
 * PC[0]: Current Function Address
 * PC[1]: Current Instruction Address
 */
int update_pc(u_int8_t *PC, u_int8_t (*code)[][32][6]) {
    if (PC[1] + 1 <= 0b11111 && (*code)[PC[0]][PC[1] + 1][5] == 1) {
        //update pc by add 1 to instruction, if ins + 1 <= 31 and the instruction exist
        PC[1] += 1;
        return 0;
    } else if (PC[0] >= 1) {
        //Go to next function, if exist
        //As we store code by appearance order, function address will start from 0
        PC[0] -= 1;
        PC[1] = 0;
        return 0;
    }
    return -1;
}

u_int8_t print_addr(u_int8_t type, u_int8_t addr) {
    /*
     * Function: print_addr
     * --------------------
     * Print out the address with specified type
     *
     * @Param:
     * type: address type
     * address: address value
     *
     * returns:
     * -1 when error happened
     *  0 for normal cases
     */
    switch (type) {
        case 0b00:  // Val
            printf(" VAL %d", addr);
            break;
        case 0b01:  // Reg
            printf(" REG %d", addr);
            break;
        case 0b10:  // STK
            if (addr > 25) {
                addr -= 26;
                printf(" STK %c", addr + 'a');
            } else {
                printf(" STK %c", addr + 'A');
            }
            break;
        case 0b11:  // Ptr
            if (addr > 25) {
                addr -= 26;
                printf(" PTR %c", addr + 'a');
            } else {
                printf(" PTR %c", addr + 'A');
            }
            break;
        default:
            return -1;
    }
    return 0;
}

int decode(u_int8_t type, u_int8_t *val,int *st,int *sc) {
    /*
     * Function: decode
     * --------------------
     * Take a stack symbol and decode it by order of appearance
     *
     * @Param:
     * type: address type
     * val: address value
     * st: symbol table for this function
     * sc: stack table for this function
     *
     * returns:
     * -1 when error happened
     *  0 for normal cases
     */
    if (type == 0b10 || type == 0b11) {
        if (st[*val] == -1) {
            st[*val] = *sc;
            *val = *sc;
            *sc += 1;
        } else {
            *val = st[*val];
        }
    }
    return 0;
}

int print_op(u_int8_t *PC, u_int8_t (*code)[][32][6],int *st,int *sc) {
    /*
     * Function: print_op
     * --------------------
     * Print out the instruction
     *
     * @Param:
     * PC: points to instruction address
     * code: code space
     * st: symbol table for this function
     * sc: stack table for this function
     *
     * returns:
     * -1 when error happened
     *  0 for normal cases
     */
    u_int8_t opcode = (*code)[PC[0]][PC[1]][0];
    u_int8_t first_t = (*code)[PC[0]][PC[1]][1];
    u_int8_t first_v = (*code)[PC[0]][PC[1]][2];
    u_int8_t second_t = (*code)[PC[0]][PC[1]][3];
    u_int8_t second_v = (*code)[PC[0]][PC[1]][4];

   int status = update_pc(PC, code);

    decode(first_t, &first_v, st, sc);
    decode(second_t, &second_v, st, sc);

    printf("    ");
    switch (opcode) {
        case 0b000:  // MOV
            printf("MOV");
            print_addr(first_t, first_v);
            print_addr(second_t, second_v);
            break;

        case 0b001:  // CAL
            printf("CAL");
            print_addr(first_t, first_v);
            break;

        case 0b010:  // RET
            printf("RET");
            break;
        case 0b011:  // REF
            printf("REF");
            print_addr(first_t, first_v);
            print_addr(second_t, second_v);
            break;

        case 0b100:  // ADD
            printf("ADD");
            print_addr(first_t, first_v);
            print_addr(second_t, second_v);
            break;

        case 0b101:  // PRINT
            printf("PRINT");
            print_addr(first_t, first_v);
            break;

        case 0b110:  // NOT
            printf("NOT");
            print_addr(first_t, first_v);
            break;

        case 0b111:  // EQU
            printf("EQU");
            print_addr(first_t, first_v);
            break;

        default:
            return -1;
    }
    printf("\n");
    if (status == -1) {
        return -1;
    }
    return 0;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Please Provide a <filename> as command line arguments");
        return 1;
    }

    FILE *bf = fopen(argv[1], "rb");
    if (bf == NULL) {
        printf("File Not Found!\n");
        return 1;
    }

    fseek(bf, 0, SEEK_END);

    /*
     * Initialize function table, symbol table, code space
     */
    int symbol_table[8][32];
    int function_table[8][2];
    u_int8_t code_space[8][32][6];
    int status = 0;

    memset(&symbol_table, -1, 8 * 32 * sizeof(int));
    memset(&function_table, -1, 8 * 2 * sizeof(int));
    memset(&code_space, 0, 8 * 32 * 6 * sizeof(u_int8_t));

    /*
     * Fetch Codes
     */
    if (fetch_code(bf, symbol_table, function_table, code_space) == -1) {
        printf("Not an x2017 formatted file\n");
        return 1;
    }

    /*
     * Find out the top function in the file
     */
    u_int8_t PC[2] = {0, 0};
    for (int i = 0; i < 8; i++) {
        if (function_table[i][0] != -1) {
            PC[0] = i;
        } else {
            break;
        }
    }

    /*
     * Print!
     */
   int stack_table[32];
   int stack_counter;
    while (status != -1) {
        printf("FUNC LABEL %d\n", function_table[PC[0]][0]);
       int ins_num = function_table[PC[0]][1];
        memset(&stack_table, -1, 32 * sizeof(int));
        stack_counter = 0;
        if (ins_num == 0) {
            status = -1;
        }
        for (int i = 0; i < ins_num; i++) {
            status = print_op(PC, &code_space, stack_table, &stack_counter);
            if (status == -1) {
                break;
            }
        }
    }
    return 0;
}
