#include <stdio.h>
#include <stdlib.h>

typedef struct ConditionCodes {
    uint8_t     z:1;    // I assume these numbers after the colon are the number of bits
    uint8_t     s:1;
    uint8_t     p:1;
    uint8_t     cy:1;
    uint8_t     ac:1;
    uint8_t     pad:3;
} ConditionCodes;

typedef struct State8080 {
    uint8_t     a;
    uint8_t     b;
    uint8_t     c;
    uint8_t     d;
    uint8_t     e;
    uint8_t     h;
    uint8_t     l;
    uint16_t    sp; // stack pointer is a 16-bit register
    uint16_t    pc; // program counter is a 16-bit register
    uint8_t     *memory;
    struct      ConditionCodes      cc;
    uint8_t     int_enable;
} State8080;

void UnimplementedInstruction(State8080* state) {
    // pc will have advanced one, so undo that
    printf("Error: unimplemented instruction\n");
    exit(1);
}

void Emulate8080Op(State8080* state) {
    // pull the opcode.
    // state is a pointer to the state struct, so state->pc is the value of the pc
    // state->memory[state->pc] is the value of the byte where the pc is pointing
    // so we & it to get the address, and then assigne it to a char*
    // Could we drop the * and the &?
    unsigned char *opcode = &state->memory[state->pc];

    switch(*opcode) {
        case 0x00: // NOP
            break;
        case 0x01: // LXI B,D16
            state->c = opcode[1];   // Loading BC with 2 bytes direct
            state->b = opcode[2];   // Due to endianness, c <- 1, b <- 2
            state->pc += 2; //increment by 2 because of 2 byte direct value
            break;
        case 0x02:  UnimplementedInstruction(state); break;
        case 0x03:  UnimplementedInstruction(state); break;
        case 0x04:  UnimplementedInstruction(state); break;
        /* ... */
        case 0x41:  //MOV B,C
            state->b = state->c;
            break;
        case 0x42:  //MOV B,D
            state->b = state->d;
            break;
        case 0x43:  //MOV B,E
            state->b = state->e;
            break;
        case 0xfe:  UnimplementedInstruction(state); break;
        case 0xff:  UnimplementedInstruction(state); break;
    }
    state->pc += 1; // increment by one to account for the opcode byte
}

int main(int argc, char * argv[]) {
    return 0;
}
