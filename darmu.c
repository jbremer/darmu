#include <stdio.h>
#include <string.h>
#include "darm.h"
#include "darmu.h"

void darmu_init(darmu_t *d, uint8_t *image, uint8_t *stack)
{
    memset(d, 0, sizeof(darmu_t));
    d->image = image;
    d->stack = stack;
}

int darmu_mapping_add(darmu_t *d, uint32_t raw, uint32_t raw_size,
    uint32_t virtual, uint32_t virtual_size)
{
    if(d->mapping_count == DARMU_MAPPINGS_COUNT) return -1;

    d->mappings[d->mapping_count].raw = raw;
    d->mappings[d->mapping_count].raw_size = raw_size;
    d->mappings[d->mapping_count].virtual = virtual;
    d->mappings[d->mapping_count].virtual_size = virtual_size;
    d->mapping_count++;
    return 0;
}

uint32_t darmu_mapping_lookup_virtual(darmu_t *d, uint32_t raw)
{
    for (uint32_t i = 0; i < d->mapping_count; i++) {
        darmu_mapping_t *m = &d->mappings[i];
        if(raw >= m->raw && raw < m->raw + m->raw_size) {
            return raw - m->raw + m->virtual;
        }
    }
    return 0;
}

uint32_t darmu_mapping_lookup_raw(darmu_t *d, uint32_t virtual)
{
    for (uint32_t i = 0; i < d->mapping_count; i++) {
        darmu_mapping_t *m = &d->mappings[i];
        if(virtual >= m->virtual && virtual < m->virtual + m->virtual_size) {
            return virtual - m->virtual + m->raw;
        }
    }
    return 0;
}

uint32_t darmu_register_get(darmu_t *d, uint32_t idx)
{
    return idx < 16 ? d->regs[idx] : 0;
}

void darmu_register_set(darmu_t *d, uint32_t idx, uint32_t value)
{
    if(idx < 16) {
        d->regs[idx] = value;
    }
}

uint32_t darmu_flags_get(darmu_t *d)
{
    return d->flags;
}

void darmu_flags_set(darmu_t *d, uint32_t value)
{
    d->flags = value;
}

#define I(x) static void _##x(darmu_t *du, const darm_t *d)

#undef I

// define an instruction handler
#define I(x) [I_##x] = _##x

// this instruction is an alias
#define I2(x, y) [I_##x] = _##y

static void (*g_handlers[I_INSTRCNT])(darmu_t *du, const darm_t *d) = {
};

int darmu_single_step(darmu_t *du)
{
    darm_t d;

    // calculate the raw offset of the program counter
    uint32_t offset = darmu_mapping_lookup_raw(du, du->regs[PC]);

    // read the instruction
    uint32_t opcode = *(uint32_t *) &du->image[offset];

    // disassemble the instruction
    int ret = darm_armv7_disasm(&d, opcode);
    if(ret < 0) {
        fprintf(stderr, "Invalid instruction.. 0x%08x\n", opcode);
        return ret;
    }

    if(g_handlers[d.instr] == NULL) {
        darm_str_t str;
        darm_str(&d, &str);
        fprintf(stderr, "[-] instruction '%s' unhandled!\n", str.instr);
        return -1;
    }

    g_handlers[d.instr](du, &d);

    // increase the program counter
    du->regs[PC] += 4;

    return 0;
}
