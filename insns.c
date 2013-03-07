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

// define an instruction handler
#define D(x) [I_##x] = _##x

// this instruction is an alias
#define A(x, y) [I_##x] = _##y

void (*g_handlers[I_INSTRCNT])(darmu_t *du, const darm_t *d) = {
    D(STMDB), A(PUSH, STMDB),
};
