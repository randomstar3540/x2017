#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void printBits(size_t const size, void const * const ptr)
{
    unsigned char *b = (unsigned char*) ptr;
    unsigned char byte;
    int i, j;

    for (i = size-1; i >= 0; i--) {
        for (j = 7; j >= 0; j--) {
            byte = (b[i] >> j) & 1;
            printf("%u", byte);
        }
        printf(" ");
    }
    puts("");
}

int readbyte(FILE* bfile, int16_t *filesize, int16_t readsize, u_int16_t *result){
    if (*filesize - readsize < 0){
        return -1;
    }
    fseek(bfile,-readsize,SEEK_CUR);
    fread(result,readsize,1,bfile); //处理
    fseek(bfile,-readsize,SEEK_CUR);
    *filesize -= readsize;
    return 0;
}

int relocate_buffer(FILE* bfile, int16_t *filesize, u_int16_t *buffer, u_int8_t *bptr){
    if(*bptr >= 8){
        u_int16_t byte = 0;
        if (readbyte(bfile,filesize,1,&byte) ==0){
            *buffer = *buffer >> 8;
            *buffer = (byte<<8) | *buffer;
            *bptr -= 8;
        }
    }
    return 0;
}

int read_bit_reverse(FILE* bf, int16_t *fsize,u_int16_t *buffer, u_int8_t *bptr, int bits, u_int8_t *result){
    u_int16_t buffer_cpy = *buffer;
    buffer_cpy = buffer_cpy << (16 - bits - *bptr);
    buffer_cpy = buffer_cpy >> (16 - bits - *bptr);
    buffer_cpy = buffer_cpy >> *bptr;
    *result = buffer_cpy;
    *bptr += bits;
    relocate_buffer(bf,fsize,buffer,bptr);
    return 0;
}

u_int8_t PC_readFunc(u_int8_t PC){
    return PC >> 5;
};
u_int8_t PC_readIns(u_int8_t PC){
    PC = PC <<3;
    return PC >> 3;
};
int PC_write(u_int8_t func, u_int8_t ins, u_int8_t *PC){
    (*PC) = (func << 5) | ins;
    return 0;
}

void debug(u_int8_t *reg, u_int8_t *RAM){
    printf("PC Value: F %d, I %d\n", PC_readFunc(reg[7]), PC_readIns(reg[7]));
    for(int i = 0; i < 8; i ++){
        printf("Reg%d: %x\n",i,reg[i]);
    }
    for(int i = 0; i < 32; i ++){
        for(int j = 0; j < 8; j++){
            printf("RAM %d: %x     ",i*8 + j, RAM[i*8 + j]);
        }
        printf("\n");
    }
}

int update_pc(u_int8_t *reg, u_int8_t (*code)[][32][6]){
    if (PC_readIns(reg[7]) != 0b11111){

        PC_write(PC_readFunc(reg[7]),PC_readIns(reg[7]) +1, &(reg[7]));

        if((*code)[PC_readFunc(reg[7])][PC_readIns(reg[7])][5] != 1 ){
            if(PC_readFunc(reg[7])==0){
                reg[4] = 2;
                return 0;
            }
        }
        return 0;
    }
    return -1;
}

u_int8_t read_addr(u_int8_t *reg, u_int8_t *RAM, u_int8_t type, u_int8_t addr){
    switch (type) {
        case 0b00:
            return addr;//Val
        case 0b01:
            //Reg
            return reg[addr];
        case 0b10:
            return RAM[reg[6]-addr]; // STK
        case 0b11:
            return RAM[RAM[reg[6]-addr]]; // Ptr
        default:
            return -1;
    }
}

int write_addr(u_int8_t *reg, u_int8_t *RAM, u_int8_t type, u_int8_t addr, u_int8_t val){
    switch (type) {
        case 0b00:
            return -1;//Val
        case 0b01:
            reg[addr] = val;//Reg
            return 0;
        case 0b10:
            RAM[reg[6]-addr] = val; // STK
            return 0;
        case 0b11://Ptr
            RAM[RAM[reg[6]-addr]] = val;
        default:
            return -1;
    }
}

int handle_op(u_int8_t *reg, u_int8_t *RAM, u_int8_t (*code)[][32][6], int16_t (*st)[][2]){
    u_int8_t opcode;
    u_int8_t first_v;
    u_int8_t first_t;
    u_int8_t second_v;
    u_int8_t second_t;

    opcode = (*code)[PC_readFunc(reg[7])][PC_readIns(reg[7])][0];
    first_t = (*code)[PC_readFunc(reg[7])][PC_readIns(reg[7])][1];
    first_v = (*code)[PC_readFunc(reg[7])][PC_readIns(reg[7])][2];
    second_t = (*code)[PC_readFunc(reg[7])][PC_readIns(reg[7])][3];
    second_v = (*code)[PC_readFunc(reg[7])][PC_readIns(reg[7])][4];

    update_pc(reg,code);

    switch(opcode){
        case 0b000: //MOV
            write_addr(reg,RAM,first_t,first_v,read_addr(reg,RAM,second_t,second_v));
            return 0;
        case 0b001: //CAL
            if(reg[6]<33){
                reg[4] = 3;
                return 0;
            }
            reg[6]-= 33;
            RAM[reg[6]+1] = reg[7];
            if ((*st)[first_v][0] != -1) {
                PC_write((*st)[first_v][0], 0, &(reg[7]));
                return 0;
            }
            return 3;
        case 0b010: //RET
            if(PC_readFunc(reg[7])==(*st)[0][0]){
                reg[4] = 1;
                return 0;
            }
            reg[4] = 0;
            reg[7] = RAM[reg[6]+1];
            reg[6] +=33;
            return 0;
        case 0b011: //REF
            write_addr(reg,RAM,first_t,first_v,reg[6] - second_v);
            return 0;
        case 0b100: //ADD
            write_addr(reg,RAM,first_t,first_v,reg[first_v]+reg[second_v]);
            return 0;
        case 0b101: //PRINT
            printf("%d\n",read_addr(reg,RAM,first_t,first_v));
            return 0;
        case 0b110: //NOT
            reg[first_v] = ~reg[first_v];
            return 0;
        case 0b111: //EQU
            if (reg[first_v] == 0){
                reg[first_v] = 1;
            }else{
                reg[first_v] = 0;
            }
            return 0;
        default:
            return -1;
    }
}

int fetch_addr(FILE* bf, int16_t *fsize, u_int16_t *buffer, u_int8_t type, u_int8_t *bptr, u_int8_t *result){
    switch (type) {
        case 0b00:
            read_bit_reverse(bf,fsize,buffer, bptr, 8, result); //Val
            break;
        case 0b01:
            read_bit_reverse(bf,fsize,buffer, bptr, 3, result); //Reg
            break;
        case 0b10:
        case 0b11:
            read_bit_reverse(bf,fsize,buffer, bptr, 5, result); //Stk,Ptr
            break;
        default:
            return -1;
    }
    relocate_buffer(bf, fsize, buffer, bptr);
    return 0;
}

int fetch_op(FILE* bf, int16_t *fsize,u_int16_t *buffer, u_int8_t *bptr, u_int8_t *code,u_int8_t op){
    u_int8_t opcode = op;
    u_int8_t first_v;
    u_int8_t first_t;
    u_int8_t second_v;
    u_int8_t second_t;

    if(opcode == 0b000 || opcode == 0b011 || opcode== 0b100){
        read_bit_reverse(bf,fsize,buffer,bptr, 2, &first_t);
        fetch_addr(bf,fsize,buffer,first_t,bptr,&first_v);

        read_bit_reverse(bf,fsize,buffer, bptr, 2, &second_t);
        fetch_addr(bf,fsize,buffer,second_t,bptr,&second_v);

        code[1] = first_t;
        code[2] = first_v;
        code[3] = second_t;
        code[4] = second_v;
        code[5] = 1;

    }else if(opcode == 0b001 || opcode == 0b101 || opcode == 0b110 || opcode == 0b111){
        read_bit_reverse(bf,fsize,buffer, bptr, 2, &first_t);
        fetch_addr(bf,fsize,buffer,first_t,bptr,&first_v);

        code[1] = first_t;
        code[2] = first_v;
        code[5] = 1;

    }else if(opcode == 0b010) {
        code[5] = 1;
    }
    return 0;
}



int fetch_next_func(FILE* bf, int16_t (*byte)[32], int16_t (*bit)[32], int16_t (*func)[2], u_int8_t (*code)[32][6]){
    fseek(bf,0,SEEK_END);
    int16_t fsize = ftell(bf);
    u_int16_t buffer;
    u_int8_t bit_ptr = 0;
    u_int8_t ins_num;
    u_int8_t opcode;

    u_int8_t func_label;

    readbyte(bf,&fsize,2,&buffer);
    buffer = ((buffer & 0xff) << 8) | ((buffer & 0xff00) >> 8);
    int counter = 0;
    while(fsize > 0){
        read_bit_reverse(bf,&fsize,&buffer,&bit_ptr,5, &ins_num);
        for(int i = ins_num -1; i >= 0; i--) {
            byte[counter][i] = fsize+1;
            bit[counter][i] = bit_ptr;
//            printf("%d,%d\n",fsize + 1, bit_ptr);
            read_bit_reverse(bf,&fsize,&buffer, &bit_ptr, 3, &opcode);
            code[counter][i][0] = opcode;
            fetch_op(bf,&fsize,&buffer,&bit_ptr, &code[counter][i][0],opcode);
        }
        read_bit_reverse(bf,&fsize,&buffer, &bit_ptr, 3, &func_label);
        func[func_label][0] =counter;
        func[func_label][1] =ins_num;
        counter ++;
    }
    return 0;
}

int main(int argc, char **argv){
    if (argc < 2){
        return 1;
    }

    FILE *bf = fopen(argv[1],"rb");

    fseek(bf,0,SEEK_END);

    int16_t ins_bit[8][32];
    int16_t ins_byte[8][32];
    int16_t func[8][2];
    u_int8_t code_space[8][32][6];

    memset(&ins_byte,-1, 8*32*sizeof(int16_t));
    memset(&ins_bit,-1, 8*32*sizeof(int16_t));
    memset(&func,-1, 8*2*sizeof(int16_t));
    memset(&code_space,0, 8*32*6*sizeof(u_int8_t));
    fetch_next_func(bf,ins_byte,ins_bit,func,code_space);

    u_int8_t reg[8];
    u_int8_t RAM[256];

    memset(&reg,0, 8*sizeof(u_int8_t));
    memset(&RAM,0, 256*sizeof(u_int8_t));

    if(func[0][0] != -1){
        PC_write(func[0][0],0,&reg[7]);
    }
    reg[6]=255;

    while(reg[4] == 0){
//        debug(reg,RAM);
        handle_op(reg,RAM,&code_space,&func);
    }
    return 0;
}




