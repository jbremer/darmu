#include <stdio.h>
#include <stdint.h>
#include "darmu.h"
#include "darm.h"

#define I(x) static void _##x(darmu_t *du, const darm_t *d)

I(STMDB) {
    uint32_t Rn = du->regs[d->Rn];
    uint32_t reglist = d->reglist;

    while (reglist != 0) {
        uint32_t reg = 32 - __builtin_clz(reglist) - 1;
        darmu_write32(du, Rn -= 4, du->regs[reg]);
        reglist &= ~(1 << reg);
    }
    if(d->W == B_SET) {
        du->regs[d->Rn] = Rn;
    }
}

I(ldr_str) {
    uint32_t offset = darm_get_offset(d, du->regs[d->Rm]);
    uint32_t addr = du->regs[d->Rn];

    if(d->Rn == PC) addr += 8;

    if(d->U == B_SET) addr += (d->P == B_SET ? offset : 0);
    else              addr -= (d->P == B_SET ? offset : 0);

    switch ((uint32_t) d->instr) {
    case I_LDRB:
        du->regs[d->Rt] = darmu_read8(du, addr);
        break;

    case I_LDRH:
        du->regs[d->Rt] = darmu_read16(du, addr);
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

    if(d->W == B_SET) {
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

I(ADD) {
    du->regs[d->Rd] = du->regs[d->Rn] + darm_get_offset(d, du->regs[d->Rm]);
}

I(CMP) {
    // TODO real flag support
    du->flags = du->regs[d->Rn] == darm_get_offset(d, du->regs[d->Rm]) ?
        C_EQ : C_NE;
}

I(MOV) {
    du->regs[d->Rd] = du->regs[d->Rm];
}

I(SUB) {
    du->regs[d->Rd] = du->regs[d->Rn] - darm_get_offset(d, du->regs[d->Rm]);
}

// define an instruction handler
#define D(x) [I_##x] = _##x

// this instruction is an alias
#define A(x, y) [I_##x] = _##y

void (*g_handlers[I_INSTRCNT])(darmu_t *du, const darm_t *d) = {
    D(STMDB), A(PUSH, STMDB), D(B), D(ADD), D(CMP), D(MOV), D(BL), D(SUB),

    A(STR, ldr_str), A(LDR, ldr_str), A(STRB, ldr_str), A(LDRB, ldr_str),
    A(STRD, ldr_str), A(LDRD, ldr_str), A(STRH, ldr_str), A(LDRH, ldr_str),
};
