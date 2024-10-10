#include <stdio.h>

#include "cpu.h"

CPU* make_cpu(Memory* mem)
{
    CPU* cpu = calloc(1, sizeof(CPU));
    cpu->mem = mem;

    return cpu;
}

void print_reg(CPU* cpu)
{
    printf("AF: 0x%02x \tBC: 0x%02x \tDE: 0x%02x \tHL: 0x%02x \tSP: 0x%02x \tPC: 0x%02x\n", cpu->af, cpu->bc, cpu->de, cpu->hl, cpu->sp, cpu->pc);
}

uint8_t get_inst(CPU* cpu)
{
    // printf("%d\n", cpu->mem->ram[cpu->pc]);
    return cpu->mem->ram[cpu->pc++];
}

uint8_t* get_reg_8(CPU* cpu, int index)
{
    uint8_t* regMap[] = {
        &cpu->b,
        &cpu->c,
        &cpu->d,
        &cpu->e,
        &cpu->h,
        &cpu->l,
        &cpu->mem->ram[cpu->hl],
        &cpu->a
    };

    return regMap[index];
}

uint16_t* get_reg_16(CPU* cpu, int index)
{
    uint16_t* regMap[] = {
        &cpu->bc,
        &cpu->de,
        &cpu->hl,
        &cpu->sp
    };

    return regMap[index];
}

uint8_t* get_reg_a16(CPU* cpu, int index)
{
    uint8_t* regMap[] = {
        &cpu->mem->ram[cpu->bc],
        &cpu->mem->ram[cpu->de],
        &cpu->mem->ram[cpu->hl],
        &cpu->mem->ram[cpu->hl]
    };

    if (index == 2) cpu->hl++;
    else if (index == 3) cpu->hl--;

    return regMap[index];
}

int noop(CPU* cpu, uint8_t inst)
{
    return 1;
}

int stop(CPU* cpu, uint8_t inst)
{
    get_inst(cpu);
    return 1;
}

int halt(CPU* cpu, uint8_t inst)
{
    return 1;
}

int di(CPU* cpu, uint8_t inst)
{
    return 1;
}

int ei(CPU* cpu, uint8_t inst)
{
    return 1;
}

int inc_16(CPU* cpu, uint8_t inst)
{
    (&cpu->af)[1 + (inst >> 4)]++;
    return 2;
}

int dec_16(CPU* cpu, uint8_t inst)
{
    (&cpu->af)[1 + (inst >> 4)]--;
    return 2;
}

int inc_8(CPU* cpu, uint8_t inst)
{
    uint8_t regIndex = ((inst >> 3) & 1) + 2 * (inst >> 4);
    uint8_t* reg = get_reg_8(cpu, regIndex);

    cpu->n = 0;
    cpu->half_carry = ((*reg)++ & 0xf) + (1 & 0xf) & 0x10;
    cpu->z = (*reg) == 0;

    return regIndex == 6 ? 3 : 1;
}

int dec_8(CPU* cpu, uint8_t inst)
{
    uint8_t regIndex = ((inst >> 3) & 1) + 2 * (inst >> 4);
    uint8_t* reg = get_reg_8(cpu, regIndex);

    cpu->n = 1;
    cpu->half_carry = ((*reg)-- & 0xf) - (1 & 0xf) & 0x10;
    cpu->z = (*reg) == 0;

    return regIndex == 6 ? 3 : 1;
}

int ld_8(CPU* cpu, uint8_t inst)
{
    uint8_t destRegIndex = (inst - 0x40) >> 3;
    uint8_t* destReg = get_reg_8(cpu, destRegIndex);

    uint8_t srcRegIndex = inst & 0x7;
    uint8_t* srcReg = get_reg_8(cpu, srcRegIndex);

    *destReg = *srcReg;

    return destRegIndex == 6 || srcRegIndex == 6 ? 2 : 1;
}

int ld_8_d8(CPU* cpu, uint8_t inst)
{
    uint8_t destRegIndex = inst >> 3;
    uint8_t* destReg = get_reg_8(cpu, destRegIndex);

    *destReg = get_inst(cpu);

    return destRegIndex == 6 ? 3 : 2;
}

int ld_16(CPU* cpu, uint8_t inst)
{
    uint16_t destRegIndex = inst >> 4;
    uint16_t* destReg = get_reg_16(cpu, destRegIndex);

    *destReg = get_inst(cpu) | (get_inst(cpu) << 8);

    return 3;
}

int ld_a16(CPU* cpu, uint8_t inst)
{
    uint16_t destRegIndex = inst >> 4;
    uint8_t* destReg = get_reg_a16(cpu, destRegIndex);

    *destReg = cpu->a;

    return 2;
}

int ld_a_a16(CPU* cpu, uint8_t inst)
{
    uint16_t srcRegIndex = inst >> 4;
    uint8_t* srcReg = get_reg_a16(cpu, srcRegIndex);

    cpu->a = *srcReg;

    return 2;
}

int ld_a16_sp(CPU* cpu, uint8_t inst)
{
    uint16_t addr = get_inst(cpu) | (get_inst(cpu) << 8);

    cpu->mem->ram[addr] = cpu->sp & 0xff;
    cpu->mem->ram[addr + 1] = cpu->sp >> 8;

    return 5;
}

int ld_a_8(CPU* cpu, uint8_t inst)
{
    uint16_t addr = get_inst(cpu) | 0xff00;

    uint8_t* reg1 = &cpu->a;
    uint8_t* reg2 = &cpu->mem->ram[addr];

    if (inst < 0xf0)
    {
        uint8_t* temp = reg1;
        reg1 = reg2;
        reg2 = temp;
    }

    *reg1 = *reg2;

    return 3;
}

int ld_a_c(CPU* cpu, uint8_t inst)
{
    uint16_t addr = cpu->c | 0xff00;

    uint8_t* reg1 = &cpu->a;
    uint8_t* reg2 = &cpu->mem->ram[addr];

    if (inst < 0xf0)
    {
        uint8_t* temp = reg1;
        reg1 = reg2;
        reg2 = temp;
    }

    *reg1 = *reg2;

    return 2;
}

int ld_a_16(CPU* cpu, uint8_t inst)
{
    uint16_t addr = get_inst(cpu) | (get_inst(cpu) << 8);

    uint8_t* reg1 = &cpu->a;
    uint8_t* reg2 = &cpu->mem->ram[addr];

    if (inst < 0xf0)
    {
        uint8_t* temp = reg1;
        reg1 = reg2;
        reg2 = temp;
    }

    *reg1 = *reg2;

    return 4;
}

int ld_hl_sp_s8(CPU* cpu, uint8_t inst)
{
    int8_t imm = get_inst(cpu);

    int sum = cpu->sp + imm;

    cpu->half_carry = (imm & 0xf) + (cpu->sp & 0xf) & 0x10;
    cpu->carry = (imm & 0xff) + (cpu->sp & 0xff) & 0x100;

    cpu->hl = sum;

    cpu->z = 0;
    cpu->n = 0;

    return 3;
}

int ld_sp_hl(CPU* cpu, uint8_t inst)
{
    cpu->sp = cpu->hl;

    return 2;
}

int add_hl(CPU* cpu, uint8_t inst)
{
    int destRegIndex = inst >> 4;
    uint16_t* reg = get_reg_16(cpu, destRegIndex);

    int sum = cpu->hl + (*reg);

    cpu->n = 0;
    cpu->half_carry = ((*reg) & 0xfff) + (cpu->hl & 0xfff) & 0x1000;
    cpu->carry = sum > 0xffff;

    cpu->hl = sum;

    return 2;
}

int add_8(CPU* cpu, uint8_t inst)
{
    uint8_t regIndex = inst & 0x7;
    uint8_t* reg = get_reg_8(cpu, regIndex);

    int sum = cpu->a + *reg;

    cpu->half_carry = ((*reg) & 0xf) + (cpu->a & 0xf) & 0x10;

    cpu->a = sum;

    cpu->z = cpu->a == 0;
    cpu->n = 0;
    cpu->carry = sum > 0xff;

    return regIndex == 6 ? 2 : 1;
}

int sub_8(CPU* cpu, uint8_t inst)
{
    uint8_t regIndex = inst & 0x7;
    uint8_t* reg = get_reg_8(cpu, regIndex);

    int diff = cpu->a - *reg;

    cpu->half_carry = (cpu->a & 0xf) - ((*reg) & 0xf) & 0x10;

    cpu->a = diff;

    cpu->z = cpu->a == 0;
    cpu->n = 1;
    cpu->carry = diff < 0;

    return regIndex == 6 ? 2 : 1;
}

int and_8(CPU* cpu, uint8_t inst)
{
    uint8_t regIndex = inst & 0x7;
    uint8_t* reg = get_reg_8(cpu, regIndex);

    cpu->a &= *reg;

    cpu->z = cpu->a == 0;
    cpu->n = 0;
    cpu->half_carry = 1;
    cpu->carry = 0;

    return regIndex == 6 ? 2 : 1;
}

int or_8(CPU* cpu, uint8_t inst)
{
    uint8_t regIndex = inst & 0x7;
    uint8_t* reg = get_reg_8(cpu, regIndex);

    cpu->a |= *reg;

    cpu->z = cpu->a == 0;
    cpu->n = 0;
    cpu->half_carry = 0;
    cpu->carry = 0;

    return regIndex == 6 ? 2 : 1;
}

int xor_8(CPU* cpu, uint8_t inst)
{
    uint8_t regIndex = inst & 0x7;
    uint8_t* reg = get_reg_8(cpu, regIndex);

    cpu->a ^= *reg;

    cpu->z = cpu->a == 0;
    cpu->n = 0;
    cpu->half_carry = 0;
    cpu->carry = 0;

    return regIndex == 6 ? 2 : 1;
}

int cp_8(CPU* cpu, uint8_t inst)
{
    uint8_t regIndex = inst & 0x7;
    uint8_t* reg = get_reg_8(cpu, regIndex);

    int diff = cpu->a - *reg;

    cpu->half_carry = (cpu->a & 0xf) - ((*reg) & 0xf) & 0x10;

    cpu->z = !diff;
    cpu->n = 1;
    cpu->carry = diff < 0;

    return regIndex == 6 ? 2 : 1;
}

int adc_8(CPU* cpu, uint8_t inst)
{
    uint8_t regIndex = inst & 0x7;
    uint8_t* reg = get_reg_8(cpu, regIndex);

    int sum = *reg + cpu->carry + cpu->a;

    cpu->half_carry = ((*reg) & 0xf) + (cpu->a & 0xf) + (cpu->carry & 0xf) & 0x10;

    cpu->a = sum;

    cpu->z = cpu->a == 0;
    cpu->n = 0;
    cpu->carry = sum > 0xff;

    return regIndex == 6 ? 2 : 1;
}

int sbc_8(CPU* cpu, uint8_t inst)
{
    uint8_t regIndex = inst & 0x7;
    uint8_t* reg = get_reg_8(cpu, regIndex);

    int sum = cpu->a - *reg - cpu->carry;

    cpu->half_carry = (cpu->a & 0xf) - ((*reg) & 0xf) - (cpu->carry & 0xf) & 0x10;

    cpu->a = sum;

    cpu->z = cpu->a == 0;
    cpu->n = 1;
    cpu->carry = sum < 0;

    return regIndex == 6 ? 2 : 1;
}

int add_d8(CPU* cpu, uint8_t inst)
{
    uint8_t reg = get_inst(cpu);

    int sum = cpu->a + reg;

    cpu->half_carry = (reg & 0xf) + (cpu->a & 0xf) & 0x10;

    cpu->a = sum;

    cpu->z = cpu->a == 0;
    cpu->n = 0;
    cpu->carry = sum > 0xff;

    return 2;
}

int add_s8(CPU* cpu, uint8_t inst)
{
    int8_t reg = get_inst(cpu);

    int sum = cpu->sp + reg;

    cpu->half_carry = (reg & 0xf) + (cpu->sp & 0xf) & 0x10;
    cpu->carry = (reg & 0xff) + (cpu->sp & 0xff) & 0x100;

    cpu->sp = sum;

    cpu->z = cpu->sp == 0;
    cpu->n = 0;

    return 4;
}

int sub_d8(CPU* cpu, uint8_t inst)
{
    uint8_t reg = get_inst(cpu);

    int diff = cpu->a - reg;

    cpu->half_carry = (cpu->a & 0xf) - (reg & 0xf) & 0x10;

    cpu->a = diff;

    cpu->z = cpu->a == 0;
    cpu->n = 1;
    cpu->carry = diff < 0;

    return 2;
}

int and_d8(CPU* cpu, uint8_t inst)
{
    uint8_t reg = get_inst(cpu);

    cpu->a &= reg;

    cpu->z = cpu->a == 0;
    cpu->n = 0;
    cpu->half_carry = 1;
    cpu->carry = 0;

    return 2;
}

int or_d8(CPU* cpu, uint8_t inst)
{
    uint8_t reg = get_inst(cpu);

    cpu->a |= reg;

    cpu->z = cpu->a == 0;
    cpu->n = 0;
    cpu->half_carry = 0;
    cpu->carry = 0;

    return 2;
}

int xor_d8(CPU* cpu, uint8_t inst)
{
    uint8_t reg = get_inst(cpu);

    cpu->a ^= reg;

    cpu->z = cpu->a == 0;
    cpu->n = 0;
    cpu->half_carry = 0;
    cpu->carry = 0;

    return 2;
}

int cp_d8(CPU* cpu, uint8_t inst)
{
    uint8_t reg = get_inst(cpu);

    int diff = cpu->a - reg;

    cpu->half_carry = (cpu->a & 0xf) - (reg & 0xf) & 0x10;

    cpu->z = !diff;
    cpu->n = 1;
    cpu->carry = diff < 0;

    return 2;
}

int adc_d8(CPU* cpu, uint8_t inst)
{
    uint8_t reg = get_inst(cpu);

    int sum = reg + cpu->carry + cpu->a;

    cpu->half_carry = (reg & 0xf) + (cpu->a & 0xf) + (cpu->carry & 0xf) & 0x10;

    cpu->a = sum;

    cpu->z = cpu->a == 0;
    cpu->n = 0;
    cpu->carry = sum > 0xff;

    return 2;
}

int sbc_d8(CPU* cpu, uint8_t inst)
{
    uint8_t reg = get_inst(cpu);

    int sum = cpu->a - reg - cpu->carry;

    cpu->half_carry = (cpu->a & 0xf) - (reg & 0xf) - (cpu->carry & 0xf) & 0x10;

    cpu->a = sum;

    cpu->z = cpu->a == 0;
    cpu->n = 1;
    cpu->carry = sum < 0;

    return 2;
}

int rlca(CPU* cpu, uint8_t inst)
{
    cpu->z = 0;
    cpu->n = 0;
    cpu->half_carry = 0;
    cpu->carry = cpu->a >> 7;

    cpu->a = (cpu->a << 1) | cpu->carry;

    return 1;
}

int rrca(CPU* cpu, uint8_t inst)
{
    cpu->z = 0;
    cpu->n = 0;
    cpu->half_carry = 0;
    cpu->carry = cpu->a & 1;

    cpu->a = (cpu->a >> 1) | (cpu->carry << 7);

    return 1;
}

int rla(CPU* cpu, uint8_t inst)
{
    bool oldCarry = cpu->carry;

    cpu->z = 0;
    cpu->n = 0;
    cpu->half_carry = 0;
    cpu->carry = cpu->a >> 7;

    cpu->a = (cpu->a << 1) | oldCarry;

    return 1;
}

int rra(CPU* cpu, uint8_t inst)
{
    bool oldCarry = cpu->carry;

    cpu->z = 0;
    cpu->n = 0;
    cpu->half_carry = 0;
    cpu->carry = cpu->a & 1;

    cpu->a = (cpu->a >> 1) | (oldCarry << 7);

    return 1;
}

int jr(CPU* cpu, uint8_t inst)
{
    cpu->pc += (int8_t)get_inst(cpu);

    return 3;
}

int jr_8(CPU* cpu, uint8_t inst)
{
    bool flag = (inst >= 0x30) ? cpu->carry : cpu->z;
    if ((inst & 0xf) < 0x8) flag = !flag;

    int8_t dist = get_inst(cpu);

    cpu->pc += flag ? dist : 0;

    return flag ? 3 : 2;
}

int jp(CPU* cpu, uint8_t inst)
{
    cpu->pc = get_inst(cpu) | (get_inst(cpu) << 8);

    return 4;
}

int jp_16(CPU* cpu, uint8_t inst)
{
    bool flag = (inst >= 0xd0) ? cpu->carry : cpu->z;
    if ((inst & 0xf) < 0x8) flag = !flag;

    uint16_t newPC = get_inst(cpu) | (get_inst(cpu) << 8);

    if (flag) cpu->pc = newPC;

    return flag ? 4 : 3;
}

int jp_hl(CPU* cpu, uint8_t inst)
{
    cpu->pc = cpu->hl;

    return 1;
}

int daa(CPU* cpu, uint8_t inst)
{
    if (!cpu->n)
    {
        if (cpu->carry || cpu->a > 0x99) { cpu->a += 0x60; cpu->carry = 1; }
        if (cpu->half_carry || (cpu->a & 0x0f) > 0x09) { cpu->a += 0x6; }
    }
    else
    {
        if (cpu->carry) { cpu->a -= 0x60; }
        if (cpu->half_carry) { cpu->a -= 0x6; }
    }

    cpu->z = (cpu->a == 0);
    cpu->half_carry = 0;

    return 1;
}

int cpl(CPU* cpu, uint8_t inst)
{
    cpu->a = ~cpu->a;

    cpu->n = 1;
    cpu->half_carry = 1;

    return 1;
}

int scf(CPU* cpu, uint8_t inst)
{
    cpu->n = 0;
    cpu->half_carry = 0;
    cpu->carry = 1;

    return 1;
}

int ccf(CPU* cpu, uint8_t inst)
{
    cpu->n = 0;
    cpu->half_carry = 0;
    cpu->carry = !cpu->carry;

    return 1;
}

int ret(CPU* cpu, uint8_t inst)
{
    cpu->pc = cpu->mem->ram[cpu->sp++] | (cpu->mem->ram[cpu->sp++] << 8);

    return 4;
}

int reti(CPU* cpu, uint8_t inst)
{
    cpu->pc = cpu->mem->ram[cpu->sp++] | (cpu->mem->ram[cpu->sp++] << 8);

    return 4;
}

int ret_8(CPU* cpu, uint8_t inst)
{
    bool flag = (inst >= 0xd0) ? cpu->carry : cpu->z;
    if ((inst & 0xf) < 0x8) flag = !flag;

    if (flag) cpu->pc = cpu->mem->ram[cpu->sp++] | (cpu->mem->ram[cpu->sp++] << 8);

    return flag ? 5 : 2;
}

int pop(CPU* cpu, uint8_t inst)
{
    uint16_t* regMap[] = {
        &cpu->bc,
        &cpu->de,
        &cpu->hl,
        &cpu->af
    };

    uint16_t* reg = regMap[(inst >> 4) - 0xc];

    *reg = cpu->mem->ram[cpu->sp++] | (cpu->mem->ram[cpu->sp++] << 8);

    cpu->f &= 0xf0;

    return 3;
}

int push(CPU* cpu, uint8_t inst)
{
    uint16_t* regMap[] = {
        &cpu->bc,
        &cpu->de,
        &cpu->hl,
        &cpu->af
    };

    uint16_t* reg = regMap[(inst >> 4) - 0xc];

    cpu->mem->ram[--cpu->sp] = *reg >> 8;
    cpu->mem->ram[--cpu->sp] = *reg & 0xff;

    return 4;
}

int call(CPU* cpu, uint8_t inst)
{
    uint16_t newPC = get_inst(cpu) | (get_inst(cpu) << 8);

    cpu->mem->ram[--cpu->sp] = cpu->pc >> 8;
    cpu->mem->ram[--cpu->sp] = cpu->pc & 0xff;

    cpu->pc = newPC;

    return 6;
}

int call_16(CPU* cpu, uint8_t inst)
{
    bool flag = (inst >= 0xd0) ? cpu->carry : cpu->z;
    if ((inst & 0xf) < 0x8) flag = !flag;

    uint16_t newPC = get_inst(cpu) | (get_inst(cpu) << 8);

    if (flag)
    {
        cpu->mem->ram[--cpu->sp] = cpu->pc >> 8;
        cpu->mem->ram[--cpu->sp] = cpu->pc & 0xff;

        cpu->pc = newPC;
    }

    return flag ? 6 : 3;
}

int rst(CPU* cpu, uint8_t inst)
{
    cpu->mem->ram[--cpu->sp] = cpu->pc >> 8;
    cpu->mem->ram[--cpu->sp] = cpu->pc & 0xff;

    cpu->pc = 8 * (2 * ((inst >> 4) - 0xc) + ((inst >> 3) & 1));

    return 4;
}

int cb(CPU* cpu, uint8_t inst);

int rlc(CPU* cpu, uint8_t inst)
{
    int index = inst & 0b111;
    uint8_t* reg = get_reg_8(cpu, index);

    cpu->n = 0;
    cpu->half_carry = 0;
    cpu->carry = *reg >> 7;

    *reg <<= 1;
    *reg |= cpu->carry;

    cpu->z = *reg == 0;

    return index == 6 ? 4 : 2;
}

int rrc(CPU* cpu, uint8_t inst)
{
    int index = inst & 0b111;
    uint8_t* reg = get_reg_8(cpu, index);

    cpu->n = 0;
    cpu->half_carry = 0;
    cpu->carry = *reg & 1;

    *reg >>= 1;
    *reg |= cpu->carry << 7;

    cpu->z = *reg == 0;

    return index == 6 ? 4 : 2;
}

int rl(CPU* cpu, uint8_t inst)
{
    int index = inst & 0b111;
    uint8_t* reg = get_reg_8(cpu, index);
    bool oldCarry = cpu->carry;

    cpu->n = 0;
    cpu->half_carry = 0;
    cpu->carry = *reg >> 7;

    *reg <<= 1;
    *reg |= oldCarry;

    cpu->z = *reg == 0;

    return index == 6 ? 4 : 2;
}

int rr(CPU* cpu, uint8_t inst)
{
    int index = inst & 0b111;
    uint8_t* reg = get_reg_8(cpu, index);
    bool oldCarry = cpu->carry;

    cpu->n = 0;
    cpu->half_carry = 0;
    cpu->carry = *reg & 1;

    *reg >>= 1;
    *reg |= oldCarry << 7;

    cpu->z = *reg == 0;

    return index == 6 ? 4 : 2;
}

int sla(CPU* cpu, uint8_t inst)
{
    int index = inst & 0b111;
    uint8_t* reg = get_reg_8(cpu, index);

    cpu->n = 0;
    cpu->half_carry = 0;
    cpu->carry = *reg >> 7;

    *reg <<= 1;

    cpu->z = *reg == 0;

    return index == 6 ? 4 : 2;
}

int sra(CPU* cpu, uint8_t inst)
{
    int index = inst & 0b111;
    uint8_t* reg = get_reg_8(cpu, index);
    bool oldBit7 = *reg >> 7;

    cpu->n = 0;
    cpu->half_carry = 0;
    cpu->carry = *reg & 1;

    *reg >>= 1;
    *reg |= oldBit7 << 7;

    cpu->z = *reg == 0;

    return index == 6 ? 4 : 2;
}

int swap(CPU* cpu, uint8_t inst)
{
    int index = inst & 0b111;
    uint8_t* reg = get_reg_8(cpu, index);

    cpu->n = 0;
    cpu->half_carry = 0;
    cpu->carry = 0;

    uint8_t temp = 0xff & *reg;
    *reg >>= 4;
    *reg |= temp << 4;

    cpu->z = *reg == 0;

    return index == 6 ? 4 : 2;
}

int srl(CPU* cpu, uint8_t inst)
{
    int index = inst & 0b111;
    uint8_t* reg = get_reg_8(cpu, index);

    cpu->n = 0;
    cpu->half_carry = 0;
    cpu->carry = *reg & 1;

    *reg >>= 1;

    cpu->z = *reg == 0;

    return index == 6 ? 4 : 2;
}

int bit(CPU* cpu, uint8_t inst)
{
    int index = inst & 0b111;
    uint8_t* reg = get_reg_8(cpu, index);

    int bitIndex = (inst - 0x40) >> 3;

    cpu->n = 0;
    cpu->half_carry = 1;
    cpu->z = !(*reg & (1 << bitIndex));

    return index == 6 ? 3 : 2;
}

int res(CPU* cpu, uint8_t inst)
{
    int index = inst & 0b111;
    uint8_t* reg = get_reg_8(cpu, index);

    int bitIndex = (inst - 0x80) >> 3;

    *reg &= ~(1 << bitIndex); 

    return index == 6 ? 4 : 2;
}

int set(CPU* cpu, uint8_t inst)
{
    int index = inst & 0b111;
    uint8_t* reg = get_reg_8(cpu, index);

    int bitIndex = (inst - 0xc0) >> 3;

    *reg |= 1 << bitIndex; 

    return index == 6 ? 4 : 2;
}

int (*instruction_map[0x100])(CPU* cpu, uint8_t inst) = {
    //  x0      x1      x2      x3          x4          x5          x6          x7      x8              x9          xa          xb      xc          xd      xe          xf
        noop,   ld_16,  ld_a16, inc_16,     inc_8,      dec_8,      ld_8_d8,    rlca,   ld_a16_sp,      add_hl,     ld_a_a16,   dec_16, inc_8,      dec_8,  ld_8_d8,    rrca,   // 0x
        stop,   ld_16,  ld_a16, inc_16,     inc_8,      dec_8,      ld_8_d8,    rla,    jr,             add_hl,     ld_a_a16,   dec_16, inc_8,      dec_8,  ld_8_d8,    rra,    // 1x
        jr_8,   ld_16,  ld_a16, inc_16,     inc_8,      dec_8,      ld_8_d8,    daa,    jr_8,           add_hl,     ld_a_a16,   dec_16, inc_8,      dec_8,  ld_8_d8,    cpl,    // 2x
        jr_8,   ld_16,  ld_a16, inc_16,     inc_8,      dec_8,      ld_8_d8,    scf,    jr_8,           add_hl,     ld_a_a16,   dec_16, inc_8,      dec_8,  ld_8_d8,    ccf,    // 3x
        ld_8,   ld_8,   ld_8,   ld_8,       ld_8,       ld_8,       ld_8,       ld_8,   ld_8,           ld_8,       ld_8,       ld_8,   ld_8,       ld_8,   ld_8,       ld_8,   // 4x
        ld_8,   ld_8,   ld_8,   ld_8,       ld_8,       ld_8,       ld_8,       ld_8,   ld_8,           ld_8,       ld_8,       ld_8,   ld_8,       ld_8,   ld_8,       ld_8,   // 5x
        ld_8,   ld_8,   ld_8,   ld_8,       ld_8,       ld_8,       ld_8,       ld_8,   ld_8,           ld_8,       ld_8,       ld_8,   ld_8,       ld_8,   ld_8,       ld_8,   // 6x
        ld_8,   ld_8,   ld_8,   ld_8,       ld_8,       ld_8,       halt,       ld_8,   ld_8,           ld_8,       ld_8,       ld_8,   ld_8,       ld_8,   ld_8,       ld_8,   // 7x
        add_8,  add_8,  add_8,  add_8,      add_8,      add_8,      add_8,      add_8,  adc_8,          adc_8,      adc_8,      adc_8,  adc_8,      adc_8,  adc_8,      adc_8,  // 8x
        sub_8,  sub_8,  sub_8,  sub_8,      sub_8,      sub_8,      sub_8,      sub_8,  sbc_8,          sbc_8,      sbc_8,      sbc_8,  sbc_8,      sbc_8,  sbc_8,      sbc_8,  // 9x
        and_8,  and_8,  and_8,  and_8,      and_8,      and_8,      and_8,      and_8,  xor_8,          xor_8,      xor_8,      xor_8,  xor_8,      xor_8,  xor_8,      xor_8,  // ax
        or_8,   or_8,   or_8,   or_8,       or_8,       or_8,       or_8,       or_8,   cp_8,           cp_8,       cp_8,       cp_8,   cp_8,       cp_8,   cp_8,       cp_8,   // bx
        ret_8,  pop,    jp_16,  jp,         call_16,    push,       add_d8,     rst,    ret_8,          ret,        jp_16,      cb,     call_16,    call,   adc_d8,     rst,    // cx
        ret_8,  pop,    jp_16,  noop,       call_16,    push,       sub_d8,     rst,    ret_8,          reti,       jp_16,      noop,   call_16,    noop,   sbc_d8,     rst,    // dx
        ld_a_8, pop,    ld_a_c, noop,       noop,       push,       and_d8,     rst,    add_s8,         jp_hl,      ld_a_16,    noop,   noop,       noop,   xor_d8,     rst,    // ex
        ld_a_8, pop,    ld_a_c, di,         noop,       push,       or_d8,      rst,    ld_hl_sp_s8,    ld_sp_hl,   ld_a_16,    ei,     noop,       noop,   cp_d8,      rst     // fx
};

int (*cb_instruction_map[0x100])(CPU* cpu, uint8_t inst) = {
    //  x0      x1      x2      x3      x4      x5      x6      x7      x8      x9      xa      xb      xc      xd      xe      xf
        rlc,    rlc,    rlc,    rlc,    rlc,    rlc,    rlc,    rlc,    rrc,    rrc,    rrc,    rrc,    rrc,    rrc,    rrc,    rrc,    // 0x
        rl,     rl,     rl,     rl,     rl,     rl,     rl,     rl,     rr,     rr,     rr,     rr,     rr,     rr,     rr,     rr,     // 1x
        sla,    sla,    sla,    sla,    sla,    sla,    sla,    sla,    sra,    sra,    sra,    sra,    sra,    sra,    sra,    sra,    // 2x
        swap,   swap,   swap,   swap,   swap,   swap,   swap,   swap,   srl,    srl,    srl,    srl,    srl,    srl,    srl,    srl,    // 3x
        bit,    bit,    bit,    bit,    bit,    bit,    bit,    bit,    bit,    bit,    bit,    bit,    bit,    bit,    bit,    bit,    // 4x
        bit,    bit,    bit,    bit,    bit,    bit,    bit,    bit,    bit,    bit,    bit,    bit,    bit,    bit,    bit,    bit,    // 5x
        bit,    bit,    bit,    bit,    bit,    bit,    bit,    bit,    bit,    bit,    bit,    bit,    bit,    bit,    bit,    bit,    // 6x
        bit,    bit,    bit,    bit,    bit,    bit,    bit,    bit,    bit,    bit,    bit,    bit,    bit,    bit,    bit,    bit,    // 7x
        res,    res,    res,    res,    res,    res,    res,    res,    res,    res,    res,    res,    res,    res,    res,    res,    // 8x
        res,    res,    res,    res,    res,    res,    res,    res,    res,    res,    res,    res,    res,    res,    res,    res,    // 9x
        res,    res,    res,    res,    res,    res,    res,    res,    res,    res,    res,    res,    res,    res,    res,    res,    // ax
        res,    res,    res,    res,    res,    res,    res,    res,    res,    res,    res,    res,    res,    res,    res,    res,    // bx
        set,    set,    set,    set,    set,    set,    set,    set,    set,    set,    set,    set,    set,    set,    set,    set,    // cx
        set,    set,    set,    set,    set,    set,    set,    set,    set,    set,    set,    set,    set,    set,    set,    set,    // dx
        set,    set,    set,    set,    set,    set,    set,    set,    set,    set,    set,    set,    set,    set,    set,    set,    // ex
        set,    set,    set,    set,    set,    set,    set,    set,    set,    set,    set,    set,    set,    set,    set,    set     // fx
};

int cb(CPU* cpu, uint8_t inst)
{
    uint8_t cbInst = get_inst(cpu);
    int cycles = cb_instruction_map[cbInst](cpu, cbInst);

    return cycles;
}

int execute_inst(CPU* cpu)
{
    uint8_t inst = get_inst(cpu);
    int cycles = instruction_map[inst](cpu, inst);

    return cycles;
}