#include <string.h>
#include "fetch_x2017.h"

int fetch_code(FILE *bf, int (*st)[32], int (*ft)[2], u_int8_t (*code)[32][6]) {
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
        if (readbits(bf, 5, &ins_num) == -1) {
            //read the instruction count, raise error if any
            return -1;
        }
        stack_counter = 0;

        for (int i = ins_num - 1; i >= 0; i--) {
            if (readbits(bf, 3, &opcode) == -1) {
                // read operation code, raise error if any
                return -1;
            }
            code[counter][i][0] = opcode;
            fetch_op(bf, &code[counter][i][0], &st[func_label][0], opcode, &stack_counter);
        }
        if (readbits(bf, 3, &func_label) == -1) {
            // read the function label, raise error if any
            return -1;
        }
        if (ft[func_label][0] != -1) {
            // if the function we read already exist, raise error
            return -1;
        }
        ft[func_label][0] = counter; //store the function order and instruction count in the function table
        ft[func_label][1] = ins_num;
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
 * Program counter (reg[7])
 * | 000 | 00000 |
 *   FUNC   INS
 */
u_int8_t PC_readFunc(u_int8_t PC) { return PC >> 5; };
u_int8_t PC_readIns(u_int8_t PC) {
    PC = PC << 3;
    return PC >> 3;
};
int PC_write(u_int8_t func, u_int8_t ins, u_int8_t *PC) {
    (*PC) = (func << 5) | ins;
    return 0;
}

int update_pc(u_int8_t *reg, u_int8_t (*code)[][32][6]) {
    if (PC_readIns(reg[7]) != 0b11111) {
        //if the instruction count of current PC points neq to 31,update by add 1 to ins count
        PC_write(PC_readFunc(reg[7]), PC_readIns(reg[7]) + 1, &(reg[7]));

        if ((*code)[PC_readFunc(reg[7])][PC_readIns(reg[7])][5] != 1) {
            //no instruction next, prepare to return
            reg[4] = 2;
            return 0;
        }
        return 0;
    }
    return -1;
}

u_int8_t read_addr(u_int8_t *reg, u_int8_t *RAM, u_int8_t type, u_int8_t addr) {
    switch (type) {
        case 0b00:
            return addr;  // Val
        case 0b01:
            // Reg
            return reg[addr];
        case 0b10:
            if (reg[6] < addr) {
                reg[4] = 3;
                return 1;
            }
            return RAM[reg[6] - addr];  // STK, add the offset to the stack base pointer
        case 0b11:
            if (reg[6] < addr) {
                reg[4] = 3;
                return 1;
            }
            return RAM[RAM[reg[6] - addr]];  // PTR, first get the value in stack and use it as an address
        default:
            return 1;
    }
}

int write_addr(u_int8_t *reg, u_int8_t *RAM, u_int8_t type, u_int8_t addr,
               u_int8_t val) {
    switch (type) {
        case 0b00:
            /*
             * Will raise an error, since you can't write to a value
             */
            reg[4] = 5;
            return 1;  // Val
        case 0b01:
            if (addr > 3 && addr != 7) {
                reg[4] = 4;
                return 1;
            }
            reg[addr] = val;  // Reg
            return 0;
        case 0b10:
            if (reg[6] < addr) {
                //detect stack overflow
                reg[4] = 3;
                return 1;
            }
            if (reg[6] - addr < reg[5]) {
                //update stack top pointer if needed
                reg[5] = reg[6] - addr;
            }
            RAM[reg[6] - addr] = val;  // STK
            return 0;
        case 0b11:
            if (reg[6] < addr) {
                //detect stack overflow
                reg[4] = 3;
                return 1;
            }
            if (reg[6] - addr < reg[5]) {
                //update stack top pointer if needed
                reg[5] = reg[6] - addr;
            }
            RAM[RAM[reg[6] - addr]] = val;  // Ptr
        default:
            return 1;
    }
}

int handle_op(u_int8_t *reg, u_int8_t *RAM, u_int8_t (*code)[][32][6], int (*ft)[][2]) {
    u_int8_t opcode;
    u_int8_t first_t;
    u_int8_t first_v;
    u_int8_t second_t;
    u_int8_t second_v;

    opcode = (*code)[PC_readFunc(reg[7])][PC_readIns(reg[7])][0];
    first_t = (*code)[PC_readFunc(reg[7])][PC_readIns(reg[7])][1];
    first_v = (*code)[PC_readFunc(reg[7])][PC_readIns(reg[7])][2];
    second_t = (*code)[PC_readFunc(reg[7])][PC_readIns(reg[7])][3];
    second_v = (*code)[PC_readFunc(reg[7])][PC_readIns(reg[7])][4];

    if(update_pc(reg, code)==-1){
        //prepare to return
        reg[4]=2;
    }

    switch (opcode) {

        /* ===========
         *     MOV
         * ===========
         */
        case 0b000:
            if (write_addr(reg, RAM, first_t, first_v,
                           read_addr(reg, RAM, second_t, second_v)) == 1) {
                return 1;
            }
            return 0;

        /* ===========
         *     CAL
         * ===========
         */
        case 0b001:
            if (reg[5] <= 4) {
                //detect stack overflow
                reg[4] = 3;
                return 0;
            }
            /*
             * RAM     | A | PC | Old_SBP | SC | ? |
             *           |                       |
             *      New_SBP&STP               STP-now
             */
            RAM[reg[5] - 1] = reg[4]; //push status code into stack
            RAM[reg[5] - 2] = reg[6]; //push stack base pointer into stack
            RAM[reg[5] - 3] = reg[7]; //push PC into stack
            reg[6] = reg[5] - 4; //update SBP
            reg[5] = reg[6]; //update STP
            reg[4] = 0; //update Status Code
            if ((*ft)[first_v][0] != -1) {
                //function exist, change PC
                PC_write((*ft)[first_v][0], 0, &(reg[7]));
                return 0;
            } else {
                //function doesn't exist, report error
                reg[4] = 7;
                return 1;
            }

        /* ===========
         *     RET
         * ===========
         */
        case 0b010:
            if (PC_readFunc(reg[7]) == (*ft)[0][0] || reg[6] == 255) {
                //if function is main or stack at top of RAM, terminate the program
                reg[4] = 1;
                return 0;
            }
            reg[4] = RAM[reg[6] + 3]; //revert status code back
            reg[7] = RAM[reg[6] + 1]; //revert PC back
            reg[5] = reg[6] + 4; //revert STP back
            reg[6] = RAM[reg[6] + 2]; //revert SBP back
            return 0;

        /* ===========
         *     REF
         * ===========
         */
        case 0b011:
            if (second_t == 0b10) {
                // if REF STK
                if (write_addr(reg, RAM, first_t, first_v, reg[6] - second_v) == 1) {
                    return 1;
                }
            } else {
                // else, force REF STK
                if (write_addr(reg, RAM, first_t, first_v, read_addr(reg, RAM, 0b10, second_v)) == 1) {
                    return 1;
                }
            }
            return 0;

        /* ===========
         *     ADD
         * ===========
         */
        case 0b100:
            if (first_t != 0b01 || second_t != 0b01){
                //Check type
                reg[4]=6;
                return 1;
            }
            if (first_v > 3 && first_v != 7 || second_v > 3 && second_v != 7 ) {
                //Check if access valid
                reg[4] = 4;
                return 1;
            }
            if (write_addr(reg, RAM, first_t, first_v, reg[first_v] + reg[second_v]) == 1) {
                return 1;
            }
            return 0;

        /* ===========
         *     PRINT
         * ===========
         */
        case 0b101:
            printf("%d\n", read_addr(reg, RAM, first_t, first_v));
            return 0;

        /* ===========
         *     NOT
         * ===========
         */
        case 0b110:
            if (first_t != 0b01) {
                //Check type
                reg[4] = 6;
                return 1;
            }
            if (first_v > 3 && first_v != 7) {
                //Check if access valid
                reg[4] = 4;
                return 1;
            }
            reg[first_v] = ~reg[first_v];
            return 0;

        /* ===========
         *     EQU
         * ===========
         */
        case 0b111:
            if (first_t != 0b01) {
                //Check type
                reg[4] = 6;
                return 1;
            }
            if (first_v > 3 && first_v != 7) {
                //Check if access valid
                reg[4] = 4;
                return 1;
            }
            if (reg[first_v] == 0) {
                reg[first_v] = 1;
            } else {
                reg[first_v] = 0;
            }
            return 0;
        default:
            return 1;
    }
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

    int symbol_table[8][32];
    int function_table[8][2];
    u_int8_t code_space[8][32][6];

    /*
     * Initialize function table, symbol table, code space
     */
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
     * Initialize VM
     */
    u_int8_t reg[8];
    u_int8_t RAM[256];

    memset(&reg, 0, 8 * sizeof(u_int8_t));
    memset(&RAM, 0, 256 * sizeof(u_int8_t));

    if (function_table[0][0] != -1) {
        PC_write(function_table[0][0], 0, &reg[7]);
    } else {
        printf("ERROR: No main function found\n");
        return 1;
    }
    reg[6] = 255;
    reg[5] = 255;

    /*
     * Execute instructions
     */
    while (reg[4] == 0) {
        //        debug(reg,RAM);
        handle_op(reg, RAM, &code_space, &function_table);
    }

    /*
     * Error Handling
     */
    if (reg[4] < 2) {
        return 0;
    } else if (reg[4] == 2) {
        printf("ERROR: Function terminated without return\n");
        return 1;
    } else if (reg[4] == 3) {
        printf("ERROR: Stack Overflow detected!\n");
        return 1;
    } else if (reg[4] == 4) {
        printf("ERROR: Invalid access on register!\n");
        return 1;
    } else if (reg[4] == 5) {
        printf("ERROR: Writing on value type!\n");
        return 1;
    } else if (reg[4] == 6) {
        printf("ERROR: Memory type Error!\n");
        return 1;
    } else if (reg[4] == 7) {
        printf("ERROR: Function called do not exist!\n");
        return 1;
    } else {
        return 1;
    }
}
