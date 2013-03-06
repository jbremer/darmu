#include <stdio.h>
#include "darm.h"
#include "darmu.h"

void darmu_init(darmu_t *d, unsigned char *image, unsigned char *stack)
{
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

int darmu_mapping_lookup_virtual(darmu_t *d, unsigned int raw)
{
    for (uint32_t i = 0; i < d->mapping_count; i++) {
        darmu_mapping_t *m = &d->mappings[i];
        if(raw >= m->raw && raw < m->raw + m->raw_size) {
            return raw - m->raw + m->virtual;
        }
    }
    return 0;
}

int darmu_mapping_lookup_raw(darmu_t *d, unsigned int virtual)
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

int darmu_single_step(darmu_t *du)
{
    darm_t d;

    // calculate the raw offset of the program counter
    uint32_t offset = darmu_mapping_lookup_raw(du, du->regs[PC]);

    // read the instruction
    uint32_t opcode = *(uint32_t *) &du->image[offset];

    // disassemble the instruction
    int ret = darm_armv7_disasm(&d, opcode);
    if(ret < 0) return ret;

    switch ((uint32_t) d.instr) {
    default: {
        darm_str_t str;
        darm_str(&d, &str);
        fprintf(stderr, "[-] instruction '%s' unhandled!\n", str.instr);
        return -1;
    }}

    // increase the program counter
    du->regs[PC] += 4;

    return 0;
}
