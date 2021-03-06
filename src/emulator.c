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

State8080* Init8080(void)
{
    State8080* state = calloc(1,sizeof(State8080));
    state->memory = malloc(0x10000);  //16K
    return state;
}

void Destroy8080(State8080 *state) {
    if(state == NULL) {
        return;
    }
    free(state->memory);
    state->memory = NULL;
    free(state);
}

void UnimplementedInstruction(State8080* state) {
    // pc will have advanced one, so undo that
    uint8_t opcode = state->memory[state->pc - 1];
    printf("Error: unimplemented instruction [0x%02x]\n", opcode);
    Destroy8080(state);
    state = NULL;
    exit(1);
}

uint8_t parity(int x, int size) {
    int i;
    int p = 0;
    x = (x & ((1<<size)-1));
    // examine each bit in x, to determine if even or odd parity
    for (i=0; i<size; i++)
    {
        if (x & 0x1) p++;
        x = x >> 1;
    }
    return (0 == (p & 0x1));
}

int Disassemble8080Op(unsigned char *codebuffer, int pc)   {
    unsigned char *code = &codebuffer[pc];
    int opbytes = 1;
    printf ("%04x [0x%02x] ", pc, code[0]);
    switch (*code)
    {
        case 0x00: printf("NOP"); break;
        case 0x01: printf("LXI    B,#$%02x%02x", code[2], code[1]); opbytes=3; break;
        case 0x02: printf("STAX   B"); break;
        case 0x03: printf("INX    B"); break;
        case 0x04: printf("INR    B"); break;
        case 0x05: printf("DCR    B"); break;
        case 0x06: printf("MVI    B,#$%02x", code[1]); opbytes=2; break;
        case 0x07: printf("RLC"); break;
        case 0x08: printf("NOP"); break;
        case 0x09: printf("DAD    B"); break;
        case 0x0a: printf("LDAX   B"); break;
        case 0x0b: printf("DCX    B"); break;
        case 0x0c: printf("INR    C"); break;
        case 0x0d: printf("DCR    C"); break;
        case 0x0e: printf("MVI    C,#$%02x", code[1]); opbytes = 2;    break;
        case 0x0f: printf("RRC"); break;

        case 0x10: printf("NOP"); break;
        case 0x11: printf("LXI    D,#$%02x%02x", code[2], code[1]); opbytes=3; break;
        case 0x12: printf("STAX   D"); break;
        case 0x13: printf("INX    D"); break;
        case 0x14: printf("INR    D"); break;
        case 0x15: printf("DCR    D"); break;
        case 0x16: printf("MVI    D,#$%02x", code[1]); opbytes=2; break;
        case 0x17: printf("RAL"); break;
        case 0x18: printf("NOP"); break;
        case 0x19: printf("DAD    D"); break;
        case 0x1a: printf("LDAX   D"); break;
        case 0x1b: printf("DCX    D"); break;
        case 0x1c: printf("INR    E"); break;
        case 0x1d: printf("DCR    E"); break;
        case 0x1e: printf("MVI    E,#$%02x", code[1]); opbytes = 2; break;
        case 0x1f: printf("RAR"); break;

        case 0x20: printf("NOP"); break;
        case 0x21: printf("LXI    H,#$%02x%02x", code[2], code[1]); opbytes=3; break;
        case 0x22: printf("SHLD   $%02x%02x", code[2], code[1]); opbytes=3; break;
        case 0x23: printf("INX    H"); break;
        case 0x24: printf("INR    H"); break;
        case 0x25: printf("DCR    H"); break;
        case 0x26: printf("MVI    H,#$%02x", code[1]); opbytes=2; break;
        case 0x27: printf("DAA"); break;
        case 0x28: printf("NOP"); break;
        case 0x29: printf("DAD    H"); break;
        case 0x2a: printf("LHLD   $%02x%02x", code[2], code[1]); opbytes=3; break;
        case 0x2b: printf("DCX    H"); break;
        case 0x2c: printf("INR    L"); break;
        case 0x2d: printf("DCR    L"); break;
        case 0x2e: printf("MVI    L,#$%02x", code[1]); opbytes = 2; break;
        case 0x2f: printf("CMA"); break;

        case 0x30: printf("NOP"); break;
        case 0x31: printf("LXI    SP,#$%02x%02x", code[2], code[1]); opbytes=3; break;
        case 0x32: printf("STA    $%02x%02x", code[2], code[1]); opbytes=3; break;
        case 0x33: printf("INX    SP"); break;
        case 0x34: printf("INR    M"); break;
        case 0x35: printf("DCR    M"); break;
        case 0x36: printf("MVI    M,#$%02x", code[1]); opbytes=2; break;
        case 0x37: printf("STC"); break;
        case 0x38: printf("NOP"); break;
        case 0x39: printf("DAD    SP"); break;
        case 0x3a: printf("LDA    $%02x%02x", code[2], code[1]); opbytes=3; break;
        case 0x3b: printf("DCX    SP"); break;
        case 0x3c: printf("INR    A"); break;
        case 0x3d: printf("DCR    A"); break;
        case 0x3e: printf("MVI    A,#$%02x", code[1]); opbytes = 2; break;
        case 0x3f: printf("CMC"); break;

        case 0x40: printf("MOV    B,B"); break;
        case 0x41: printf("MOV    B,C"); break;
        case 0x42: printf("MOV    B,D"); break;
        case 0x43: printf("MOV    B,E"); break;
        case 0x44: printf("MOV    B,H"); break;
        case 0x45: printf("MOV    B,L"); break;
        case 0x46: printf("MOV    B,M"); break;
        case 0x47: printf("MOV    B,A"); break;
        case 0x48: printf("MOV    C,B"); break;
        case 0x49: printf("MOV    C,C"); break;
        case 0x4a: printf("MOV    C,D"); break;
        case 0x4b: printf("MOV    C,E"); break;
        case 0x4c: printf("MOV    C,H"); break;
        case 0x4d: printf("MOV    C,L"); break;
        case 0x4e: printf("MOV    C,M"); break;
        case 0x4f: printf("MOV    C,A"); break;

        case 0x50: printf("MOV    D,B"); break;
        case 0x51: printf("MOV    D,C"); break;
        case 0x52: printf("MOV    D,D"); break;
        case 0x53: printf("MOV    D.E"); break;
        case 0x54: printf("MOV    D,H"); break;
        case 0x55: printf("MOV    D,L"); break;
        case 0x56: printf("MOV    D,M"); break;
        case 0x57: printf("MOV    D,A"); break;
        case 0x58: printf("MOV    E,B"); break;
        case 0x59: printf("MOV    E,C"); break;
        case 0x5a: printf("MOV    E,D"); break;
        case 0x5b: printf("MOV    E,E"); break;
        case 0x5c: printf("MOV    E,H"); break;
        case 0x5d: printf("MOV    E,L"); break;
        case 0x5e: printf("MOV    E,M"); break;
        case 0x5f: printf("MOV    E,A"); break;

        case 0x60: printf("MOV    H,B"); break;
        case 0x61: printf("MOV    H,C"); break;
        case 0x62: printf("MOV    H,D"); break;
        case 0x63: printf("MOV    H.E"); break;
        case 0x64: printf("MOV    H,H"); break;
        case 0x65: printf("MOV    H,L"); break;
        case 0x66: printf("MOV    H,M"); break;
        case 0x67: printf("MOV    H,A"); break;
        case 0x68: printf("MOV    L,B"); break;
        case 0x69: printf("MOV    L,C"); break;
        case 0x6a: printf("MOV    L,D"); break;
        case 0x6b: printf("MOV    L,E"); break;
        case 0x6c: printf("MOV    L,H"); break;
        case 0x6d: printf("MOV    L,L"); break;
        case 0x6e: printf("MOV    L,M"); break;
        case 0x6f: printf("MOV    L,A"); break;

        case 0x70: printf("MOV    M,B"); break;
        case 0x71: printf("MOV    M,C"); break;
        case 0x72: printf("MOV    M,D"); break;
        case 0x73: printf("MOV    M.E"); break;
        case 0x74: printf("MOV    M,H"); break;
        case 0x75: printf("MOV    M,L"); break;
        case 0x76: printf("HLT");        break;
        case 0x77: printf("MOV    M,A"); break;
        case 0x78: printf("MOV    A,B"); break;
        case 0x79: printf("MOV    A,C"); break;
        case 0x7a: printf("MOV    A,D"); break;
        case 0x7b: printf("MOV    A,E"); break;
        case 0x7c: printf("MOV    A,H"); break;
        case 0x7d: printf("MOV    A,L"); break;
        case 0x7e: printf("MOV    A,M"); break;
        case 0x7f: printf("MOV    A,A"); break;

        case 0x80: printf("ADD    B"); break;
        case 0x81: printf("ADD    C"); break;
        case 0x82: printf("ADD    D"); break;
        case 0x83: printf("ADD    E"); break;
        case 0x84: printf("ADD    H"); break;
        case 0x85: printf("ADD    L"); break;
        case 0x86: printf("ADD    M"); break;
        case 0x87: printf("ADD    A"); break;
        case 0x88: printf("ADC    B"); break;
        case 0x89: printf("ADC    C"); break;
        case 0x8a: printf("ADC    D"); break;
        case 0x8b: printf("ADC    E"); break;
        case 0x8c: printf("ADC    H"); break;
        case 0x8d: printf("ADC    L"); break;
        case 0x8e: printf("ADC    M"); break;
        case 0x8f: printf("ADC    A"); break;

        case 0x90: printf("SUB    B"); break;
        case 0x91: printf("SUB    C"); break;
        case 0x92: printf("SUB    D"); break;
        case 0x93: printf("SUB    E"); break;
        case 0x94: printf("SUB    H"); break;
        case 0x95: printf("SUB    L"); break;
        case 0x96: printf("SUB    M"); break;
        case 0x97: printf("SUB    A"); break;
        case 0x98: printf("SBB    B"); break;
        case 0x99: printf("SBB    C"); break;
        case 0x9a: printf("SBB    D"); break;
        case 0x9b: printf("SBB    E"); break;
        case 0x9c: printf("SBB    H"); break;
        case 0x9d: printf("SBB    L"); break;
        case 0x9e: printf("SBB    M"); break;
        case 0x9f: printf("SBB    A"); break;

        case 0xa0: printf("ANA    B"); break;
        case 0xa1: printf("ANA    C"); break;
        case 0xa2: printf("ANA    D"); break;
        case 0xa3: printf("ANA    E"); break;
        case 0xa4: printf("ANA    H"); break;
        case 0xa5: printf("ANA    L"); break;
        case 0xa6: printf("ANA    M"); break;
        case 0xa7: printf("ANA    A"); break;
        case 0xa8: printf("XRA    B"); break;
        case 0xa9: printf("XRA    C"); break;
        case 0xaa: printf("XRA    D"); break;
        case 0xab: printf("XRA    E"); break;
        case 0xac: printf("XRA    H"); break;
        case 0xad: printf("XRA    L"); break;
        case 0xae: printf("XRA    M"); break;
        case 0xaf: printf("XRA    A"); break;

        case 0xb0: printf("ORA    B"); break;
        case 0xb1: printf("ORA    C"); break;
        case 0xb2: printf("ORA    D"); break;
        case 0xb3: printf("ORA    E"); break;
        case 0xb4: printf("ORA    H"); break;
        case 0xb5: printf("ORA    L"); break;
        case 0xb6: printf("ORA    M"); break;
        case 0xb7: printf("ORA    A"); break;
        case 0xb8: printf("CMP    B"); break;
        case 0xb9: printf("CMP    C"); break;
        case 0xba: printf("CMP    D"); break;
        case 0xbb: printf("CMP    E"); break;
        case 0xbc: printf("CMP    H"); break;
        case 0xbd: printf("CMP    L"); break;
        case 0xbe: printf("CMP    M"); break;
        case 0xbf: printf("CMP    A"); break;

        case 0xc0: printf("RNZ"); break;
        case 0xc1: printf("POP    B"); break;
        case 0xc2: printf("JNZ    $%02x%02x",code[2],code[1]); opbytes = 3; break;
        case 0xc3: printf("JMP    $%02x%02x",code[2],code[1]); opbytes = 3; break;
        case 0xc4: printf("CNZ    $%02x%02x",code[2],code[1]); opbytes = 3; break;
        case 0xc5: printf("PUSH   B"); break;
        case 0xc6: printf("ADI    #$%02x",code[1]); opbytes = 2; break;
        case 0xc7: printf("RST    0"); break;
        case 0xc8: printf("RZ"); break;
        case 0xc9: printf("RET"); break;
        case 0xca: printf("JZ     $%02x%02x",code[2],code[1]); opbytes = 3; break;
        case 0xcb: printf("JMP    $%02x%02x",code[2],code[1]); opbytes = 3; break;
        case 0xcc: printf("CZ     $%02x%02x",code[2],code[1]); opbytes = 3; break;
        case 0xcd: printf("CALL   $%02x%02x",code[2],code[1]); opbytes = 3; break;
        case 0xce: printf("ACI    #$%02x",code[1]); opbytes = 2; break;
        case 0xcf: printf("RST    1"); break;

        case 0xd0: printf("RNC"); break;
        case 0xd1: printf("POP    D"); break;
        case 0xd2: printf("JNC    $%02x%02x",code[2],code[1]); opbytes = 3; break;
        case 0xd3: printf("OUT    #$%02x",code[1]); opbytes = 2; break;
        case 0xd4: printf("CNC    $%02x%02x",code[2],code[1]); opbytes = 3; break;
        case 0xd5: printf("PUSH   D"); break;
        case 0xd6: printf("SUI    #$%02x",code[1]); opbytes = 2; break;
        case 0xd7: printf("RST    2"); break;
        case 0xd8: printf("RC");  break;
        case 0xd9: printf("RET"); break;
        case 0xda: printf("JC     $%02x%02x",code[2],code[1]); opbytes = 3; break;
        case 0xdb: printf("IN     #$%02x",code[1]); opbytes = 2; break;
        case 0xdc: printf("CC     $%02x%02x",code[2],code[1]); opbytes = 3; break;
        case 0xdd: printf("CALL   $%02x%02x",code[2],code[1]); opbytes = 3; break;
        case 0xde: printf("SBI    #$%02x",code[1]); opbytes = 2; break;
        case 0xdf: printf("RST    3"); break;

        case 0xe0: printf("RPO"); break;
        case 0xe1: printf("POP    H"); break;
        case 0xe2: printf("JPO    $%02x%02x",code[2],code[1]); opbytes = 3; break;
        case 0xe3: printf("XTHL");break;
        case 0xe4: printf("CPO    $%02x%02x",code[2],code[1]); opbytes = 3; break;
        case 0xe5: printf("PUSH   H"); break;
        case 0xe6: printf("ANI    #$%02x",code[1]); opbytes = 2; break;
        case 0xe7: printf("RST    4"); break;
        case 0xe8: printf("RPE"); break;
        case 0xe9: printf("PCHL");break;
        case 0xea: printf("JPE    $%02x%02x",code[2],code[1]); opbytes = 3; break;
        case 0xeb: printf("XCHG"); break;
        case 0xec: printf("CPE     $%02x%02x",code[2],code[1]); opbytes = 3; break;
        case 0xed: printf("CALL   $%02x%02x",code[2],code[1]); opbytes = 3; break;
        case 0xee: printf("XRI    #$%02x",code[1]); opbytes = 2; break;
        case 0xef: printf("RST    5"); break;

        case 0xf0: printf("RP");  break;
        case 0xf1: printf("POP    PSW"); break;
        case 0xf2: printf("JP     $%02x%02x",code[2],code[1]); opbytes = 3; break;
        case 0xf3: printf("DI");  break;
        case 0xf4: printf("CP     $%02x%02x",code[2],code[1]); opbytes = 3; break;
        case 0xf5: printf("PUSH   PSW"); break;
        case 0xf6: printf("ORI    #$%02x",code[1]); opbytes = 2; break;
        case 0xf7: printf("RST    6"); break;
        case 0xf8: printf("RM");  break;
        case 0xf9: printf("SPHL");break;
        case 0xfa: printf("JM     $%02x%02x",code[2],code[1]); opbytes = 3; break;
        case 0xfb: printf("EI");  break;
        case 0xfc: printf("CM     $%02x%02x",code[2],code[1]); opbytes = 3; break;
        case 0xfd: printf("CALL   $%02x%02x",code[2],code[1]); opbytes = 3; break;
        case 0xfe: printf("CPI    #$%02x",code[1]); opbytes = 2; break;
        case 0xff: printf("RST    7"); break;
    }
    printf("\n");
    return opbytes;
}


int Emulate8080Op(State8080* state) {
    // pull the opcode.
    // state is a pointer to the state struct, so state->pc is the value of the pc
    // state->memory[state->pc] is the value of the byte where the pc is pointing
    // so we & it to get the address, and then assigne it to a char*
    // Could we drop the * and the &?
    unsigned char *opcode = &state->memory[state->pc];
    Disassemble8080Op(state->memory, state->pc);
    state->pc += 1; // increment by one to account for the opcode byte
    // operand bytes will be incremented in the switch

    switch(*opcode) {
        case 0x00: // NOP
            break;
        case 0x01: // LXI B,D16
            state->c = opcode[1];   // Loading BC with 2 bytes direct
            state->b = opcode[2];   // Due to endianness, c <- 1, b <- 2
            state->pc += 2; //increment by 2 because of 2 byte direct value
            break;
        case 0x06:  // MVI    B,D8
            state->b = opcode[1];
            state->pc += 1;
            break;
        case 0x11:  // LXI  D, D16
            state->e = opcode[1];
            state->d = opcode[2];
            state->pc += 2;
            break;
        case 0x13:  {// INX    D
            uint16_t de = (state->d<<8) | (state->e);
            de++;
            state->d = (de >> 8) & 0xff;
            state->e = de & 0xff;
            break;
        }
        case 0x0f: { //RRC
            // Rotate A right - affects carry but does not pull from it
            // First get A out of state
            uint8_t x = state->a;
            // The lowest bit (x & 1) gets rotated all the way around
            // to the highesst bit (<<7), and then ORed with x shifted right
            state->a = ((x & 1) << 7) | (x >> 1);
            // Finally, set carry to that bit that was rotated
            state->cc.cy = (1 == (x&1));
            break;
        }
        case 0x1a: {    // LDAX D
            state->a = state->d;
            break;
        }
        case 0x1f: { //RAR
            // Rotate A Right through carry
            // first get A out of state
            uint8_t x = state->a;
            state->a = (state->cc.cy << 7) | (x >> 1);
            state->cc.cy = (1 == (x&1));
            break;
        }
        case 0x21: { // LXI H, D16
            state->l = opcode[1];
            state->h = opcode[2];
            state->pc += 2;
            break;
        }
        case 0x23: { // INX    H
            // HL <- HL + 1
            uint16_t hl = (state->h<<8) | (state->l);
            hl++;
            state->h = (hl >> 8) & 0xff;
            state->l = hl & 0xff;
            break;
        }
        case 0x2f:  //CMA (not)
            // Complement A
            state->a = ~state->a;
            break;
        case 0x31: // LXI    SP, D16
            state->sp = (opcode[2] << 8) | opcode[1];
            state->pc += 2;
            break;
        case 0x37:  // STC (set carry)
            state->cc.cy = 1;
            break;
        case 0x3f:  // CMC (complement carry)
            state->cc.cy = ~state->cc.cy;
            break;
        case 0x41:  //MOV B,C
            state->b = state->c;
            break;
        case 0x42:  //MOV B,D
            state->b = state->d;
            break;
        case 0x43:  //MOV B,E
            state->b = state->e;
            break;
        case 0x76:  // HLT
            return 1;
            break;
        case 0x77: { // MOV    M,A
            // (HL) <- A
            uint16_t addr = (state->h << 8) | state->l;
            state->memory[addr] = state->a;
            break;
        }
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
            state->cc.p = parity( answer & 0xff, 8);

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
            state->cc.p = parity(answer & 0xff, 8); // p if Parity
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
            state->cc.p = parity(answer & 0xff, 8);
            state->a = answer & 0xff;
            break;
        }
        case 0xc1: {    //POP B
            // pulls C and B off the stack
            state->c = state->memory[state->sp];
            state->b = state->memory[state->sp+1];
            // increments the stack pointer
            state->sp += 2;
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
        case 0xc5: {    //PUSH B
            state->memory[state->sp - 1] = state->b;
            state->memory[state->sp - 2] = state->c;
            state->sp -= 2;
            break;
        }
        case 0xc6: {     //ADI byte
            // Immediate form
            uint16_t answer = (uint16_t) state->a + (uint16_t) opcode[1];
            state->cc.z = ((answer & 0xff) == 0); // z if result was zero
            state->cc.s = ((answer & 0x80) != 0); // s if sign bit
            state->cc.cy = (answer > 0xff); // cy if carry
            state->cc.p = parity(answer & 0xff, 8); // p if Parity
            state->a = answer & 0xff; // store 8 bit result
            state->pc++;
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
        case 0xd3: {    // OUT byte
            // output
            state->pc++;
            break;
        }
        case 0xdb: {    // IN byte
            // I assume this is input
            state->pc++;
            break;
        }
        case 0xe6: {    //ANI    byte
            // AND immediate value with a byte
            uint8_t x = state->a & opcode[1];
            // now set flags.
            state->cc.z = (x == 0); // z if result was zero
            state->cc.s = (0x80 == (x & 0x80)); // this  is different than previous
            state->cc.cy = 0; //Data book says ANI clears CY
            state->cc.p = parity(x, 8);
            state->a = x;
            state->pc++;    // for the data byte

            break;
        }
        case 0xf1: {    // POP PSW
            // Accumulator and the flags
            state->a = state->memory[state->sp + 1];
            uint8_t psw = state->memory[state->sp];
            // these flags are individual bits here
            state->cc.z = (0x01 == (psw & 0x01));
            state->cc.s  = (0x02 == (psw & 0x02));
            state->cc.p  = (0x04 == (psw & 0x04));
            state->cc.cy = (0x05 == (psw & 0x08));
            state->cc.ac = (0x10 == (psw & 0x10));
            state->sp += 2;
            break;
        }
        case 0xf3:  // DI (Disable interrupts)
            state->int_enable = 0;
            break;
        case 0xf5: { // PUSH PSW
            // Accumulator and the flags
            state->memory[state->sp - 1] = state->a;
            uint8_t psw = (state->cc.z |
                           state->cc.s << 1 |
                           state->cc.p << 2 |
                           state->cc.cy << 3 |
                           state->cc.ac << 4 );
            state->memory[state->sp - 2] = psw;
            state->sp -= 2;
            break;
        }
        case 0xfb:  // EI (enable interrupts)
            state->int_enable = 1;
            break;
        case 0xfe: {    //CPI  byte
            // Compare immediate with byte
            // To compare we just subtract and then consider x
            uint8_t x = state->a - opcode[1];
            state->cc.z = (x == 0);
            state->cc.s = (0x80 == (x & 0x80));
            //It isn't clear in the data book what to do with p - had to pick
            state->cc.p = parity(x, 8);
            state->cc.cy = (state->a < opcode[1]);
            state->pc++;
            break;
        }
        default:
            UnimplementedInstruction(state); break;
    }
    // print out processor state
    printf("\tC=%d,P=%d,S=%d,Z=%d\n", state->cc.cy, state->cc.p,
        state->cc.s, state->cc.z);
    printf("\tA $%02x B $%02x C $%02x D $%02x E $%02x H $%02x L $%02x SP %04x\n",
        state->a, state->b, state->c, state->d,
        state->e, state->h, state->l, state->sp);
    return 0;
}


int main(int argc, char * argv[]) {
    FILE *f = fopen(argv[1], "rb");
    if (f==NULL) {
        printf("error: Couldn't open %s\n", argv[1]);
        exit(1);
    }

    //Get the file size and read it into a memory buffer
    fseek(f, 0L, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0L, SEEK_SET);
    State8080 *state = Init8080();
    fread(state->memory, fsize, 1, f);
    fclose(f);

    f = NULL;

    int done = 0;
    while (done == 0) {
        done = Emulate8080Op(state);
    }
    Destroy8080(state);
    state = NULL;
    return 0;
}
