#include <stdio.h>
#include "shell.h"

#define OPCODE(instr) ((instr) >> 26 & 0x3F)
#define FUNCT(instr) ((instr)&0x3F)
#define RS(instr) ((instr) >> 21 & 0x1F)
#define RT(instr) ((instr) >> 16 & 0x1F)
#define RD(instr) ((instr) >> 11 & 0x1F)
#define SHAMT(instr) ((instr) >> 6 & 0x1F)
#define IMMEDIATE(instr) ((int32_t)(instr & 0xFFFF))

void execute_r_instruction(uint32_t instruction)
{
    uint8_t funct = FUNCT(instruction);
    uint32_t rs = RS(instruction);
    uint32_t rt = RT(instruction);
    uint32_t rd = RD(instruction);
    uint32_t shamt = SHAMT(instruction);

    switch (funct)
    {
    case 0x20: // add
        NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] + CURRENT_STATE.REGS[rt];
        break;
    case 0x21: // addu
        NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] + CURRENT_STATE.REGS[rt];
        break;
    case 0x22: // sub
        NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] - CURRENT_STATE.REGS[rt];
        break;
    case 0x23: // subu
        NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] - CURRENT_STATE.REGS[rt];
        break;
    case 0x18: // mult
    {
        int64_t result = (int64_t)CURRENT_STATE.REGS[rs] * (int64_t)CURRENT_STATE.REGS[rt];
        NEXT_STATE.HI = (uint32_t)(result >> 32);
        NEXT_STATE.LO = (uint32_t)result;
    }
    break;
    case 0x19: // multu
    {
        uint64_t result = (uint64_t)CURRENT_STATE.REGS[rs] * (uint64_t)CURRENT_STATE.REGS[rt];
        NEXT_STATE.HI = (uint32_t)(result >> 32);
        NEXT_STATE.LO = (uint32_t)result;
    }
    break;
    case 0x1A: // div
        if (CURRENT_STATE.REGS[rt] != 0)
        {
            NEXT_STATE.LO = (int32_t)CURRENT_STATE.REGS[rs] / (int32_t)CURRENT_STATE.REGS[rt];
            NEXT_STATE.HI = (int32_t)CURRENT_STATE.REGS[rs] % (int32_t)CURRENT_STATE.REGS[rt];
        }
        break;
    case 0x1B: // divu
        if (CURRENT_STATE.REGS[rt] != 0)
        {
            NEXT_STATE.LO = CURRENT_STATE.REGS[rs] / CURRENT_STATE.REGS[rt];
            NEXT_STATE.HI = CURRENT_STATE.REGS[rs] % CURRENT_STATE.REGS[rt];
        }
        break;
    case 0x10: // mfhi
        NEXT_STATE.REGS[rd] = CURRENT_STATE.HI;
        break;
    case 0x12: // mflo
        NEXT_STATE.REGS[rd] = CURRENT_STATE.LO;
        break;
    case 0x11: // mthi
        NEXT_STATE.HI = CURRENT_STATE.REGS[rs];
        break;
    case 0x13: // mtlo
        NEXT_STATE.LO = CURRENT_STATE.REGS[rs];
        break;
    case 0x0C: // syscall
        if (CURRENT_STATE.REGS[2] == 0x0A)
        {
            RUN_BIT = FALSE;
        }
        break;
    case 0x00: // sll
        NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rt] << shamt;
        break;
    case 0x02: // srl
        NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rt] >> shamt;
        break;
    case 0x03: // sra
        NEXT_STATE.REGS[rd] = (int32_t)CURRENT_STATE.REGS[rt] >> shamt;
        break;
    case 0x04: // sllv
        NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rt] << (CURRENT_STATE.REGS[rs] & 0x1F);
        break;
    case 0x06: // srlv
        NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rt] >> (CURRENT_STATE.REGS[rs] & 0x1F);
        break;
    case 0x07: // srav
        NEXT_STATE.REGS[rd] = (int32_t)CURRENT_STATE.REGS[rt] >> (CURRENT_STATE.REGS[rs] & 0x1F);
        break;
    case 0x08: // jr
        NEXT_STATE.PC = CURRENT_STATE.REGS[rs];
        break;
    case 0x09: // jalr
        NEXT_STATE.PC = CURRENT_STATE.REGS[rs];
        NEXT_STATE.REGS[rd] = CURRENT_STATE.PC + 4;
        break;
    default:
        printf("未知的R型指令\n");
        break;
    }
}

void execute_j_instruction(uint32_t instruction)
{
    uint32_t target = (instruction & 0x3FFFFFF) << 2;
    NEXT_STATE.PC = (CURRENT_STATE.PC & 0xF0000000) | target;
}

void execute_i_instruction(uint32_t instruction)
{
    uint8_t opcode = OPCODE(instruction);
    uint32_t rs = RS(instruction);
    uint32_t rt = RT(instruction);
    int32_t immediate = IMMEDIATE(instruction);

    switch (opcode)
    {
    case 0x04: // beq
        if (CURRENT_STATE.REGS[rs] == CURRENT_STATE.REGS[rt])
        {
            NEXT_STATE.PC = CURRENT_STATE.PC + 4 + (immediate << 2);
        }
        break;
    case 0x05: // bne
        if (CURRENT_STATE.REGS[rs] != CURRENT_STATE.REGS[rt])
        {
            NEXT_STATE.PC = CURRENT_STATE.PC + 4 + (immediate << 2);
        }
        break;
    case 0x06: // blez
        if (CURRENT_STATE.REGS[rs] <= 0)
        {
            NEXT_STATE.PC = CURRENT_STATE.PC + 4 + (immediate << 2);
        }
        break;
    case 0x07: // bgtz
        if (CURRENT_STATE.REGS[rs] > 0)
        {
            NEXT_STATE.PC = CURRENT_STATE.PC + 4 + (immediate << 2);
        }
        break;
    case 0x08: // addi
        NEXT_STATE.REGS[rt] = CURRENT_STATE.REGS[rs] + immediate;
        break;
    case 0x09: // addiu
        NEXT_STATE.REGS[rt] = CURRENT_STATE.REGS[rs] + immediate;
        break;
    case 0x0A: // slti
        NEXT_STATE.REGS[rt] = (int32_t)CURRENT_STATE.REGS[rs] < immediate ? 1 : 0;
        break;
    case 0x0B: // sltiu
        NEXT_STATE.REGS[rt] = CURRENT_STATE.REGS[rs] < (uint32_t)immediate ? 1 : 0;
        break;
    case 0x0C: // andi
        NEXT_STATE.REGS[rt] = CURRENT_STATE.REGS[rs] & (uint32_t)immediate;
        break;
    case 0x0D: // ori
        NEXT_STATE.REGS[rt] = CURRENT_STATE.REGS[rs] | (uint32_t)immediate;
        break;
    case 0x0E: // xori
        NEXT_STATE.REGS[rt] = CURRENT_STATE.REGS[rs] ^ (uint32_t)immediate;
        break;
    case 0x0F: // lui
        NEXT_STATE.REGS[rt] = immediate << 16;
        break;
    case 0x20: // lb
    {
        int32_t address = CURRENT_STATE.REGS[rs] + immediate;
        uint32_t data = mem_read_32(address);
        if (data & 0x80)
        {
            data |= 0xFFFFFF00;
        }
        NEXT_STATE.REGS[rt] = data;
    }
    break;
    case 0x21: // lh
    {
        int32_t address = CURRENT_STATE.REGS[rs] + immediate;
        uint32_t data = mem_read_32(address);
        if (data & 0x8000)
        {
            data |= 0xFFFF0000;
        }
        NEXT_STATE.REGS[rt] = data;
    }
    break;
    case 0x23: // lw
    {
        int32_t address = CURRENT_STATE.REGS[rs] + immediate;
        uint32_t data = mem_read_32(address);
        NEXT_STATE.REGS[rt] = data;
    }
    break;
    case 0x24: // lbu
    {
        int32_t address = CURRENT_STATE.REGS[rs] + immediate;
        uint32_t data = mem_read_32(address);
        NEXT_STATE.REGS[rt] = data;
    }
    break;
    case 0x25: // lhu
    {
        int32_t address = CURRENT_STATE.REGS[rs] + immediate;
        uint32_t data = mem_read_32(address);
        NEXT_STATE.REGS[rt] = data;
    }
    break;
    case 0x28: // sb
    {
        int32_t address = CURRENT_STATE.REGS[rs] + immediate;
        uint8_t data = CURRENT_STATE.REGS[rt] & 0xFF;
        mem_write_32(address, data);
    }
    break;
    case 0x29: // sh
    {
        int32_t address = CURRENT_STATE.REGS[rs] + immediate;
        uint16_t data = CURRENT_STATE.REGS[rt] & 0xFFFF;
        mem_write_32(address, data);
    }
    break;
    case 0x2B: // sw
    {
        int32_t address = CURRENT_STATE.REGS[rs] + immediate;
        uint32_t data = CURRENT_STATE.REGS[rt];
        mem_write_32(address, data);
    }
    break;
    case 0x01: // bltz, bgez, bltzal, bgezal
    {
        uint32_t rs = RS(instruction);
        int32_t offset = (int32_t)(immediate << 2);

        switch (rt)
        {
        case 0x00: // bltz
            if ((int32_t)CURRENT_STATE.REGS[rs] < 0)
            {
                NEXT_STATE.PC = CURRENT_STATE.PC + 4 + offset;
            }
            break;
        case 0x01: // bgez
            if ((int32_t)CURRENT_STATE.REGS[rs] >= 0)
            {
                NEXT_STATE.PC = CURRENT_STATE.PC + 4 + offset;
            }
            break;
        case 0x10: // bltzal
            if ((int32_t)CURRENT_STATE.REGS[rs] < 0)
            {
                NEXT_STATE.REGS[31] = CURRENT_STATE.PC + 4;
                NEXT_STATE.PC = CURRENT_STATE.PC + 4 + offset;
            }
            break;
        case 0x11: // bgezal
            if ((int32_t)CURRENT_STATE.REGS[rs] >= 0)
            {
                NEXT_STATE.REGS[31] = CURRENT_STATE.PC + 4;
                NEXT_STATE.PC = CURRENT_STATE.PC + 4 + offset;
            }
            break;
        default:
            printf("未知的分支指令\n");
            break;
        }
    }
    break;

    default:
        printf("未知的I型指令\n");
        break;
    }
}

void process_instruction()
{
    uint32_t instruction = mem_read_32(CURRENT_STATE.PC);
    uint8_t opcode = OPCODE(instruction);

    switch (opcode)
    {
    case 0x00: // R型指令
        execute_r_instruction(instruction);
        break;
    case 0x02: // j
        execute_j_instruction(instruction);
        break;
    case 0x03: // jal
        execute_j_instruction(instruction);
        NEXT_STATE.REGS[31] = CURRENT_STATE.PC + 4;
        break;
    case 0x04: // beq
    case 0x05: // bne
    case 0x06: // blez
    case 0x07: // bgtz
    case 0x08: // addi
    case 0x09: // addiu
    case 0x0A: // slti
    case 0x0B: // sltiu
    case 0x0C: // andi
    case 0x0D: // ori
    case 0x0E: // xori
    case 0x0F: // lui
    case 0x20: // lb
    case 0x21: // lh
    case 0x23: // lw
    case 0x24: // lbu
    case 0x25: // lhu
    case 0x28: // sb
    case 0x29: // sh
    case 0x2B: // sw
    case 0x01: // bltz, bgez, bltzal, bgezal
        execute_i_instruction(instruction);
        break;
    default:
        printf("未知的指令\n");
        break;
    }

    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}
