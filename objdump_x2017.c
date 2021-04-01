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

int read_addr(FILE* bf, int16_t *fsize,u_int8_t type, u_int8_t *result, u_int16_t *buffer, u_int8_t *bptr, char * adrstr){
    char addr[32] = "";
    switch (type) {
        case 0b00:
            read_bit_reverse(bf,fsize,buffer, bptr, 8, result); //Val
            sprintf(addr," VAL %d",*result);
            break;
        case 0b01:
            read_bit_reverse(bf,fsize,buffer, bptr, 3, result); //Reg
            sprintf(addr," REG %d",*result);
            break;
        case 0b10:
            read_bit_reverse(bf,fsize,buffer, bptr, 5, result); //Stk
            sprintf(addr," STK %c",*result + 'A');
            break;
        case 0b11:
            read_bit_reverse(bf,fsize,buffer, bptr, 5, result); //Ptr
            sprintf(addr," PTR %d",*result);
            break;
        default:
            return -1;
    }
    strcat(adrstr,addr);
    return 0;
}

int read_op(FILE* bf, int16_t byte, u_int8_t bit, char *result){
    int16_t fsize = byte+1;
    fseek(bf,fsize,SEEK_SET);
    u_int16_t buffer = 0;
    u_int8_t opcode;
    readbyte(bf,&fsize,2,&buffer);
    buffer = ((buffer & 0xff) << 8) | ((buffer & 0xff00) >> 8);

    read_bit_reverse(bf,&fsize,&buffer,&bit,3, &opcode);

    u_int8_t first_v;
    u_int8_t first_t;
    u_int8_t second_v;
    u_int8_t second_t;
    char address[32] = "";

    if(opcode == 0b000 || opcode == 0b011 || opcode == 0b100){
        read_bit_reverse(bf,&fsize,&buffer, &bit, 2, &first_t);
        read_addr(bf,&fsize,first_t,&first_v,&buffer, &bit, address);

        read_bit_reverse(bf,&fsize,&buffer, &bit, 2, &second_t);
        read_addr(bf,&fsize,second_t,&second_v,&buffer, &bit, address);

    }else if(opcode == 0b001 || opcode == 0b101 || opcode == 0b110 || opcode == 0b111){
        read_bit_reverse(bf,&fsize,&buffer, &bit, 2, &first_t);
        read_addr(bf,&fsize,first_t,&first_v,&buffer, &bit, address);

    }else if(opcode == 0b010) {
        relocate_buffer(bf, &fsize, &buffer, &bit);
    }
    switch(opcode){
        case 0b000: //MOV
            sprintf(result,"MOV%s\n",address);
            return 0;
        case 0b001: //CAL
            sprintf(result,"CAL%s\n",address);
            return 0;
        case 0b010: //RET
            sprintf(result,"RET\n");
            return 0;
        case 0b011: //REF
            sprintf(result,"REF%s\n",address);
            return 0;
        case 0b100: //ADD
            sprintf(result,"ADD%s\n",address);
            return 0;
        case 0b101: //PRINT
            sprintf(result,"PRT%s\n",address);
            return 0;
        case 0b110: //NOT
            sprintf(result,"NOT%s\n",address);
            return 0;
        case 0b111: //EQU
            sprintf(result,"EQU%s\n",address);
            return 0;
        default:
            return -1;
    }
}

int fetch_addr(FILE* bf, int16_t *fsize, u_int16_t *buffer, u_int8_t type, u_int8_t *bptr){
    switch (type) {
        case 0b00:
            *bptr += 8; //Val
            break;
        case 0b01:
            *bptr += 3; //Reg
            break;
        case 0b10:
        case 0b11:
            *bptr += 5; //Stk,Ptr
            break;
        default:
            return -1;
    }
    relocate_buffer(bf, fsize, buffer, bptr);
    return 0;
}

int fetch_op(u_int8_t opcode, FILE* bf, int16_t *fsize,u_int16_t *buffer, u_int8_t *bptr){
    u_int8_t first_t;
    u_int8_t second_t;

    if(opcode == 0b000 || opcode == 0b011 || opcode == 0b100){
        read_bit_reverse(bf,fsize,buffer,bptr, 2, &first_t);
        fetch_addr(bf,fsize,buffer,first_t,bptr);
        relocate_buffer(bf, fsize, buffer, bptr);

        read_bit_reverse(bf,fsize,buffer, bptr, 2, &second_t);
        fetch_addr(bf,fsize,buffer,second_t,bptr);
        relocate_buffer(bf, fsize, buffer, bptr);

    }else if(opcode == 0b001 || opcode == 0b101 || opcode == 0b110 || opcode == 0b111){
        read_bit_reverse(bf,fsize,buffer, bptr, 2, &first_t);
        fetch_addr(bf,fsize,buffer,first_t,bptr);
        relocate_buffer(bf, fsize, buffer, bptr);

    }else if(opcode == 0b010) {
        relocate_buffer(bf, fsize, buffer, bptr);
    }
    return 0;
}



int fetch_next_func(FILE* bf, int16_t (*byte)[8][256], int16_t (*bit)[8][256], int16_t (*func)[8][2]){
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
            (*byte)[counter][i] = fsize+1;
            (*bit)[counter][i] = bit_ptr;
            read_bit_reverse(bf,&fsize,&buffer, &bit_ptr, 3, &opcode);
            fetch_op(opcode,bf,&fsize,&buffer,&bit_ptr);
        }
        read_bit_reverse(bf,&fsize,&buffer, &bit_ptr, 3, &func_label);
        (*func)[counter][0] =func_label;
        (*func)[counter][1] =ins_num;
        counter ++;
    }
    return 0;
}

int update_pc(int16_t(*PC)[2],int16_t (*byte)[8][256], int16_t (*bit)[8][256], int16_t (*func)[8][2]){
    if((*PC)[0]+1 < 256 && (*byte)[(*PC)[0]][(*PC)[0]+1] != -1 && (*byte)[(*PC)[0]][(*PC)[0]+1] != -1){
        (*PC)[0] += 1;
        return 0;
    }else if ((*PC)[1] +1 < 8 && (*func)[(*PC)[1] +1][0] !=-1){
        (*PC)[1] += 1;
        (*PC)[0] = 0;
        return 1;
    }else{
        return -1;
    }
}

int main(int argc, char **argv){
    if (argc < 2){
        return 1;
    }

    FILE *bf = fopen(argv[1],"rb");

    fseek(bf,0,SEEK_END);

    int16_t PC[2] = {0,0};
    int16_t CUR_PC[2] = {0,0};

    char asmcode[32] = "";

    int16_t ins_bit[8][256];
    int16_t ins_byte[8][256];
    int16_t func[8][2];

    memset(&ins_byte,-1, 8*256*sizeof(int16_t));
    memset(&ins_bit,-1, 8*256*sizeof(int16_t));
    memset(&func,-1, 8*2*sizeof(int16_t));
    fetch_next_func(bf,&ins_byte,&ins_bit,&func);
    int status = 0;
    while(status != -1){
        printf("FUNC LABEL %d\n",func[PC[1]][0]);
        for(int i = 0; i < func[PC[1]][1]; i++) {
            memccpy(&CUR_PC,&PC,3,sizeof(int16_t));
            status = update_pc(&PC,&ins_byte,&ins_bit,&func);
            read_op(bf, ins_byte[CUR_PC[1]][CUR_PC[0]], ins_bit[CUR_PC[1]][CUR_PC[0]], asmcode);
            printf("%s", asmcode);
        }
    }
    return 0;
}




