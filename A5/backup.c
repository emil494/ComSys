#include "simulate.h"

// Simuler RISC-V program i givet lager og fra given start adresse
long int simulate(struct memory *mem, struct assembly *as, int start_addr, FILE *log_file) {
    int32_t regs[32];
    int32_t pc = start_addr;
    int32_t ins;
    int32_t imm;
    int32_t adr;
    int terminate = 0;
    while(!terminate) {
        ins = memory_rd_w(mem, pc);
        printf("%s\n", assembly_get(as, pc));
        printf("%x\n", ins);
        pc += 4;
        
        int32_t opcode  = ins & 0x7f;
        int32_t funct3  = (ins >> 12) & 0x7;
        int32_t funct7  = (ins >> 25) & 0x7f;
        
        int rd  = (ins >> 7) & 0x1f;
        int rs1 = (ins >> 15) & 0x1f;
        int rs2 = (ins >> 20) & 0x1f;
        regs[0] = 0;

        imm = 0; // Reset variables
        adr = 0;

        switch (opcode)
        {
        case LUI:
            regs[rd] =  ins & 0xfffff000; //((ins >> 12) & 0xfffff) << 12;
            break;
        case AUIPC:
            regs[rd] = (pc - 4) + (ins & 0xfffff000);
            break;
        case JAL:
            imm |= ((ins >> 21) & 0x3ff) << 1; 
            imm |= ((ins >> 20) & 0x1) << 11;
            imm |= ((ins >> 12) & 0xff) << 12;
            imm |= ((ins >> 31) & 0x1) << 20;
            if (imm & 0x100000) imm |= 0xfff00000; 
            regs[rd] = pc;
            pc += imm - 4;
            break;
        case JALR:
            imm = (ins >> 20) & 0xfff;
            if (imm & 0x800) imm |= 0xfffff000;
            regs[rd] = pc;
            pc = (regs[rs1] + imm) & ~1;
            break;
        case ECALL:
            switch (regs[17])
            {
            case 1:
                regs[10] = getchar();
                break;
            case 2:
                putchar(regs[10]);
                break;
            case 3:
            case 93:
                terminate = 1;
                break;
            default:
                break;
            }
            break;
        case BRANCH:
            imm |= ((ins >> 8) & 0xf) << 1;
            imm |= ((ins >> 25) & 0x3f) << 5;
            imm |= ((ins >> 7) & 0x1) << 11;
            imm |= (ins >> 31) << 12;
            if (imm & 0x1000) imm |= 0xffffe000;
            pc -= 4; // Adjust for pc increment

            switch (funct3)
            {
            case BEQ:
                if(regs[rs1] == regs[rs2]){
                    pc += imm;
                } else pc += 4;
                break;
            case BNE:
                if (regs[rs1] != regs[rs2]){
                    pc += imm;
                } else pc += 4;
                break;
            case BLT:
                if (regs[rs1] < regs[rs2]){
                    pc += imm;
                } else pc += 4;
                break;
            case BGE:
                if (regs[rs1] >= regs[rs2]){
                    pc += imm;
                } else pc += 4;
                break;
            case BLTU:
                if ((uint32_t)regs[rs1] < (uint32_t)regs[rs2]){
                    pc += imm;
                } else pc += 4;
                break;
            case BGEU:
                if ((uint32_t)regs[rs1] >= (uint32_t)regs[rs2]){
                    pc += imm;
                } else pc += 4;
                break;
            default:
                perror("BRANCH: Undefined funct3");
                exit(1);
                break;
            }
            break;
        case LOAD:
            adr = (ins >> 20) & 0xfff;
            if (adr & 0x800) adr |= 0xfffff000;
            adr += regs[rs1];
            switch (funct3)
            {
            case LB:
                regs[rd] = memory_rd_b(mem, adr);
                break;
            case LH:
                regs[rd] = memory_rd_h(mem, adr);
                break;
            case LW:
                regs[rd] = memory_rd_w(mem, adr);
                break;
            case LBU:
                regs[rd] = (uint32_t)memory_rd_b(mem, adr);
                break;
            case LHU:
                regs[rd] = (uint32_t)memory_rd_h(mem, adr);
                break;
            default:
                break;
            }
            break;
        case STORE:
            adr = ((ins >> 25) & 0x7f) << 5 | ((ins >> 7) & 0x1f);
            if (adr & 0x800) adr |= 0xfffff000;
            adr += regs[rs1];
            switch (funct3) {
            case SB:
                memory_wr_b(mem, adr, regs[rs2]);
                break;
            case SH:
                memory_wr_h(mem, adr, regs[rs2]);
                break;
            case SW:
                memory_wr_w(mem, adr, regs[rs2]);
                break;
            default:
                break;
            }
            break;
        case OP_IMM:
            imm = (ins >> 20) & 0xFFF;
            switch (funct3) {
            case ADDI:
                regs[rd] = regs[rs1] + imm;
                break;
            case SLLI:
                regs[rd] = regs[rs1] << imm;
                break;
            case SLTI:
                regs[rd] = (regs[rs1] < (int)imm) ? 1 : 0;
                break;
            case SLTIU:
                regs[rd] = ((uint32_t)regs[rs1] < imm) ? 1 : 0;
                break;
            case XORI:
                regs[rd] = regs[rs1] ^ imm;
                break;
            case SRI:
                switch (funct7) {
                case SRLI:
                    regs[rd] = (uint32_t)regs[rs1] >> imm;
                    break;
                case SRAI:
                    regs[rd] = regs[rs1] >> imm;
                    break;
                default:
                    break;
                }
                break;
            case ORI:
                regs[rd] = regs[rs1] | imm;
                break;
            case ANDI:
                regs[rd] = regs[rs1] & imm;
                break;
            default:
                break;
            }
            break;
        case OP:
            if (funct7 == RVM) { // Check for RV32M
                switch (funct3) {
                case ADUB:
                    switch (funct7) {
                    case ADD:
                        regs[rd] = regs[rs1] + regs[rs2];
                        break;
                    case SUB:
                        regs[rd] = regs[rs1] - regs[rs2];
                        break;
                    default:
                        break;
                    }
                    break;
                case SLL:
                    regs[rd] = regs[rs1] << regs[rs2];
                    break;
                case SLT:
                    regs[rd] = (regs[rs1] < regs[rs2]) ? 1 : 0;
                    break;
                case SLTU:
                    regs[rd] = (regs[rs1] < regs[rs2]) ? 1 : 0;
                    break;
                case XOR:
                    regs[rd] = regs[rs1] ^ regs[rs2];
                    break;
                case SR:
                    switch (funct7) {
                    case SRL:
                        regs[rd] = (uint32_t)regs[rs1] >> (uint32_t)regs[rs2];
                        break;
                    case SRA:
                        regs[rd] = regs[rs1] >> regs[rs2];
                        break;
                    default:
                        break;
                    }
                    break;
                case OR:
                    regs[rd] = regs[rs1] | regs[rs2];
                    break;
                case AND:
                    regs[rd] = regs[rs1] & regs[rs2];
                    break;
                default:
                    break;
                }
                break;
            } else {
                switch (funct3) {
                case MUL:
                    regs[rd] = regs[rs1] * regs[rs2];
                    break;
                case MULH:
                    regs[rd] = (regs[rs1] * regs[rs2]) >> 16;
                    break;
                case MULHSU:
                    regs[rd] = (regs[rs1] * regs[rs2]) >> 16;
                    break;
                case MULHU:
                    regs[rd] = (regs[rs1] * regs[rs2]) >> 16;
                    break;
                case DIV:
                    regs[rd] = regs[rs1] / regs[rs2];
                    break;
                case DIVU:
                    regs[rd] = regs[rs1] / regs[rs2];
                    break;
                case REM:
                    regs[rd] = regs[rs1] % regs[rs2];
                    break;
                case REMU:
                    regs[rd] = regs[rs1] % regs[rs2];
                    break;
                default:
                    break;
                }
            }
        default:
            perror("Undefined opcode");
            exit(1);
            break;
        }
    }
    return 0;
}
