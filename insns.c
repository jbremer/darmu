#include <stdio.h>
#include <stdint.h>
#include "darmu.h"
#include "darm.h"

static int64_t s_int(uint32_t x)
{
    return x >> 31 ? ((int64_t) x - 0x100000000ul) : x;
}

static uint32_t add_with_carry(uint32_t x, uint32_t y, uint32_t carry_in,
    uint32_t *carry_out, uint32_t *overflow)
{
    uint64_t unsigned_sum = (uint64_t) x + (uint64_t) y + carry_in;
    int64_t signed_sum    = s_int(x) + s_int(y) + carry_in;
    uint32_t result       = unsigned_sum & 0xffffffff;
    *carry_out            = result != unsigned_sum;
    *overflow             = s_int(result) != signed_sum;
    return result;
}

#define I(x) static void _##x(darmu_t *du, const darm_t *d)

I(stm) {
    uint32_t Rn = du->regs[d->Rn];
    uint32_t reglist = d->reglist;

    switch ((uint32_t) d->instr) {
    case I_STM:
        break;

    case I_STMDA:
        Rn -= 4 * __builtin_popcount(reglist) - 4;
        break;

    case I_STMDB: case I_PUSH:
        Rn -= 4 * __builtin_popcount(reglist);
        break;

    case I_STMIB:
        Rn += 4;
        break;
    }

    while (reglist != 0) {
        uint32_t reg = 32 - __builtin_clz(reglist) - 1;
        darmu_write32(du, Rn -= 4, du->regs[reg]);
        reglist &= ~(1 << reg);
    }

    if(d->W == B_SET) {
        du->regs[d->Rn] = Rn;
    }
}

I(ldm) {
    uint32_t Rn = du->regs[d->Rn];
    uint32_t reglist = d->reglist;

    switch ((uint32_t) d->instr) {
    case I_LDM: case I_POP:
        break;

    case I_LDMDA:
        Rn -= 4 * __builtin_popcount(reglist) - 4;
        break;

    case I_LDMDB:
        Rn -= 4 * __builtin_popcount(reglist);
        break;

    case I_LDMIB:
        Rn += 4;
        break;
    }

    while (reglist != 0) {
        uint32_t reg = __builtin_ctz(reglist);
        du->regs[reg] = darmu_read32(du, Rn);
        reglist &= ~(1 << reg), Rn += 4;
    }

    if(d->W == B_SET) {
        du->regs[d->Rn] = Rn;
    }
}

I(ldr_str) {
    uint32_t addr, offset, carry;

    offset = darmu_get_offset(d, R_INVLD, du->regs[d->Rm], 0, &carry);
    addr = du->regs[d->Rn];

    if(d->Rn == PC) addr += 8;

    if(d->P == B_SET && d->U != B_INVLD) {
        if(d->U == B_SET) addr += offset;
        else              addr -= offset;
    }

    switch ((uint32_t) d->instr) {
    case I_LDRB:
        du->regs[d->Rt] = darmu_read8(du, addr);
        break;

    case I_LDRSB:
        du->regs[d->Rt] = (uint32_t)(int8_t) darmu_read8(du, addr);
        break;

    case I_LDRH:
        du->regs[d->Rt] = darmu_read16(du, addr);
        break;

    case I_LDRSH:
        du->regs[d->Rt] = (uint32_t)(int16_t) darmu_read16(du, addr);
        break;

    case I_LDR:
        du->regs[d->Rt] = darmu_read32(du, addr);
        break;

    case I_LDRD:
        du->regs[d->Rt] = darmu_read32(du, addr);
        du->regs[d->Rt+1] = darmu_read32(du, addr+4);
        break;

    case I_STRB:
        darmu_write8(du, addr, du->regs[d->Rt]);
        break;

    case I_STRH:
        darmu_write16(du, addr, du->regs[d->Rt]);
        break;

    case I_STR:
        darmu_write32(du, addr, du->regs[d->Rt]);
        break;

    case I_STRD:
        darmu_write32(du, addr, du->regs[d->Rt]);
        darmu_write32(du, addr+4, du->regs[d->Rt+1]);
        break;
    }

    if(d->P == B_UNSET && d->U != B_INVLD) {
        if(d->U == B_SET) addr += offset;
        else              addr -= offset;
    }

    if(d->P == B_UNSET || d->W == B_SET) {
        du->regs[d->Rn] = addr;
    }
}

I(B) {
    du->regs[PC] += d->imm + 8;
}

I(BL) {
    du->regs[LR] = du->regs[PC] + 4;
    du->regs[PC] += d->imm + 8;
}

I(BX) {
    du->regs[PC] = du->regs[d->Rm];
    if(du->regs[PC] & 3) {
        fprintf(stderr, "Branching to Thumb2\n");
    }
}

I(data_proc) {
    uint32_t *dst, src1, src2, carry;

    dst = &du->regs[d->Rd];
    src1 = du->regs[d->Rn];
    src2 = darmu_get_offset(d, d->Rs, du->regs[d->Rm],
        du->regs[d->Rs], &carry);

    // temporary solution.. has to be improved
    if(d->Rn == PC) {
        src1 += 8;
    }

    switch ((uint32_t) d->instr) {
    case I_ADD:
        *dst = src1 + src2;
        break;

    case I_ADC:
        *dst = src1 + src2 + du->flags.C;
        break;

    case I_AND:
        *dst = src1 & src2;
        break;

    case I_BIC:
        *dst = src1 & ~src2;
        break;

    case I_EOR:
        *dst = src1 ^ src2;
        break;

    case I_ORR:
        *dst = src1 | src2;
        break;

    case I_RSB:
        *dst = ~src1 + src2 + 1;
        break;

    case I_RSC:
        *dst = ~src1 + src2 + du->flags.C;
        break;

    case I_SUB:
        *dst = src1 - src2;
        break;

    case I_SBC:
        *dst = src1 - src2 - du->flags.C;
        break;
    }

    if(d->S == B_SET) {
        du->flags.N = *dst >> 31;
        du->flags.Z = *dst == 0;
        du->flags.C = carry;
    }
}

I(cmp_op) {
    uint32_t result, carry, overflow, value;

    value = darmu_get_offset(d, d->Rs, du->regs[d->Rm],
        du->regs[d->Rs], &carry);


    switch ((uint32_t) d->instr) {
    case I_CMP:
        carry = 1;
        value = ~value;
        break;

    case I_CMN:
        carry = 0;
        break;
    }

    result = add_with_carry(du->regs[d->Rn], value, carry, &carry, &overflow);

    du->flags.N = result >> 31;
    du->flags.Z = result == 0;
    du->flags.C = carry;
    du->flags.V = overflow;
}

I(MOV) {
    uint32_t carry;
    du->regs[d->Rd] = darmu_get_offset(d, R_INVLD, du->regs[d->Rm],
        0, &carry);
}

I(MVN) {
    uint32_t carry;
    du->regs[d->Rd] = ~darmu_get_offset(d, R_INVLD, du->regs[d->Rm],
        0, &carry);
}

I(mov_shift) {
    uint32_t value, shift, carry;

    // if Rn isn't set, then this is shifted by an immediate
    if(d->Rn == R_INVLD) {
        value = du->regs[d->Rm];
        shift = d->shift;
    }
    else {
        value = du->regs[d->Rn];
        shift = du->regs[d->Rm] & 0xff;
    }

    du->regs[d->Rd] = darmu_get_offset(d, d->Rn, value, shift, &carry);
}

I(MUL) {
    du->regs[d->Rd] = du->regs[d->Rn] * du->regs[d->Rm];
}

I(MLA) {
    du->regs[d->Rd] = du->regs[d->Rn] * du->regs[d->Rm] + du->regs[d->Ra];
}

I(NOP) {
}

I(SVC) {
}

// define an instruction handler
#define D(x) [I_##x] = _##x

// this instruction is an alias
#define A(x, y) [I_##x] = _##y

void (*g_handlers[I_INSTRCNT])(darmu_t *du, const darm_t *d) = {
    A(STM, stm), A(STMDA, stm), A(STMDB, stm), A(PUSH, stm), A(STMIB, stm),
    A(LDM, ldm), A(POP, ldm), A(LDMDA, ldm), A(LDMDB, ldm), A(LDMIB, ldm),

    D(B), D(MOV), D(BL), D(NOP), D(SVC), D(BX), D(MVN), D(MUL), D(MLA),

    A(CMP, cmp_op), A(CMN, cmp_op),

    A(STR, ldr_str), A(LDR, ldr_str), A(STRB, ldr_str), A(LDRB, ldr_str),
    A(STRD, ldr_str), A(LDRD, ldr_str), A(STRH, ldr_str), A(LDRH, ldr_str),
    A(LDRSB, ldr_str), A(LDRSH, ldr_str),

    A(ASR, mov_shift), A(LSL, mov_shift), A(LSR, mov_shift),
    A(ROR, mov_shift), A(RRX, mov_shift),

    A(ADD, data_proc), A(ADC, data_proc), A(AND, data_proc),
    A(BIC, data_proc), A(EOR, data_proc), A(ORR, data_proc),
    A(RSB, data_proc), A(RSC, data_proc), A(SUB, data_proc),
    A(SBC, data_proc),
};
