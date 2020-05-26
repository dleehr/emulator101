#include <stdio.h>
#include <stdlib.h>

typedef struct ConditionCodes {
    // These use bit fields: https://www.tutorialspoint.com/cprogramming/c_bit_fields.htm
    uint8_t     z:1;    // Z (zero) set to 1 when the result is equal to zero
    uint8_t     s:1;    // S (sign) set to 1 when bit 7 (the most significant bit or MSB) of the math instruction is set
    uint8_t     p:1;    // P (parity) is set when the answer has even parity, clear when odd parity
    uint8_t     cy:1;   // CY (carry) set to 1 when the instruction resulted in a carry out or borrow into the high order bit
    uint8_t     ac:1;   // AC (auxillary carry) is used mostly for BCD (binary coded decimal) math. Read the data book for more details, Space Invaders doesn't use it.
    uint8_t     pad:3;  // empty to fill out the rest of the 8 bits of the struct
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

uint8_t Parity(uint8_t value) {
    // TODO: Implement this
    return 0;
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
        case 0x80:  { //ADD B
            // do the math with higher precision so we can capture the carry out
            uint16_t answer = (uint16_t) state->a + (uint16_t) state->b;
            // Zero flag: if the result is zero, set the flag
            // otherwise clear the flag
            if ((answer & 0xff) == 0) { // we AND with 0xff because we are emulating 8-bit math
                state->cc.z = 1;
            } else {
                state->cc.z = 0;
            }
            // Sign flag: if 7th bit is set, set the flag
            // otherwise clear the flag
            if (answer & 0x80) {
                state->cc.s = 1;
            } else {
                state->cc.s = 0;
            }
            // Carry flag: if answer is greater than 255, set the flag
            // otherwise, clear the flag
            if (answer > 0xff) {
                state->cc.cy = 1;
            } else {
                state->cc.cy = 0;
            }
            // Parity handled by subroutine
            state->cc.p = Parity( answer & 0xff);

            // Remember, we have to put the result in a single byte
            state->a = answer & 0xff;
            break;
        }
        case 0x81: {     //ADD C
            // More concise code for setting flags
            uint16_t answer = (uint16_t) state->a + (uint16_t) state->c;
            state->cc.z = ((answer & 0xff) == 0); // z if result was zero
            state->cc.s = ((answer & 0x80) != 0); // s if sign bit
            state->cc.cy = (answer > 0xff); // cy if carry
            state->cc.p = Parity(answer & 0xff); // p if Parity
            state->a = answer & 0xff; // store 8 bit result
            break;
        }
        case 0x86: {    //ADD M
            // Memory form
            // In the memory form, the addend is the byte pointed to by the address stored in the HL register pair.
            // Piece together the 16-bit address. lower part is in l, upper part is in h
            // shift up the h by 8 positions and then logically OR it with the l to get one 16-bit value
            uint16_t offset = (state->h<<8) | (state->l);
            uint16_t answer = (uint16_t) state->a + state->memory[offset];
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) != 0);
            state->cc.cy = (answer > 0xff);
            state->cc.p = Parity(answer & 0xff);
            state->a = answer & 0xff;
            break;
        }
        case 0xc2: {    // JNZ Address - jump to 16 bit address if not zero
            if (0 == state->cc.z) {
                // Remember, high byte is opcode[2] and low byte is opcode[1]
                state->pc = (opcode[2] << 8) | opcode[1];
                // But after the case we'll add 1 to pc. do we need to subtract that?
            } else {
                // branch not taken
                // Increment pc since we used opcode[2] and opcode[1]
                state->pc += 2;
            }
            break;
        }
        case 0xc3: {    //JMP address
            // Unconditional, so we just update the pc and don't need to +2 the pc
            state->pc = (opcode[2] << 8) | opcode[1];
            break;
        }
        case 0xc6: {     //ADI byte
            // Immediate form
            uint16_t answer = (uint16_t) state->a + (uint16_t) opcode[1];
            state->cc.z = ((answer & 0xff) == 0); // z if result was zero
            state->cc.s = ((answer & 0x80) != 0); // s if sign bit
            state->cc.cy = (answer > 0xff); // cy if carry
            state->cc.p = Parity(answer & 0xff); // p if Parity
            state->a = answer & 0xff; // store 8 bit result
            break;
        }
        case 0xc9: {    // RET
            // Get the 2 bytes off the stack and stuff them into PC
            state->pc = state->memory[state->sp] | (state->memory[state->sp+1] << 8);
            // bump the stack pointer
            state->sp += 2;
            break;
        }
        case 0xcd: {    // CALL address
            uint16_t ret = state->pc+2; // return address should be the next instruction after this one
            // push the return address onto the stack
            // The stack is just a section of memory where SP is pointing
            // take the high byte and put it in memory[sp-1]
            state->memory[state->sp-1] = (ret >> 8) & 0xff;
            // take the low byte and put it in memory[sp-2]
            state->memory[state->sp-2] = (ret & 0xff);
            // move the stack pointer down since we just pushed it
            state->sp = state->sp - 2;
            // now update pc to the address in the CALL
            state->pc = (opcode[2] << 8) | opcode[1];
            break;
        }
        case 0xfe:  UnimplementedInstruction(state); break;
        case 0xff:  UnimplementedInstruction(state); break;
    }
    state->pc += 1; // increment by one to account for the opcode byte
}

int main(int argc, char * argv[]) {
    return 0;
}
