#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "darm.h"
#include "darmu.h"

void darmu_init(darmu_t *d, uint8_t *stack, uint32_t stack_size)
{
    memset(d, 0, sizeof(darmu_t));

    d->regs[SP] = 0xb00b0000;
    darmu_mapping_add(d, stack, stack_size, d->regs[SP] - stack_size);
}

int darmu_mapping_add(darmu_t *d, uint8_t *image, uint32_t raw_size,
    uint32_t address)
{
    if(d->mapping_count == DARMU_MAPPINGS_COUNT) return -1;

    d->mappings[d->mapping_count].image = image;
    d->mappings[d->mapping_count].raw_size = raw_size;
    d->mappings[d->mapping_count].address = address;
    d->mapping_count++;
    return 0;
}

uint32_t darmu_mapping_lookup_virtual(const darmu_t *d, uint8_t *raw)
{
    for (uint32_t i = 0; i < d->mapping_count; i++) {
        const darmu_mapping_t *m = &d->mappings[i];
        if(raw >= m->image && raw < m->image + m->raw_size) {
            return raw - m->image + m->address;
        }
    }
    return 0;
}

uint32_t *darmu_mapping_lookup_raw(const darmu_t *d, uint32_t address)
{
    for (uint32_t i = 0; i < d->mapping_count; i++) {
        const darmu_mapping_t *m = &d->mappings[i];
        if(address >= m->address && address < m->address + m->raw_size) {
            return (uint32_t *) &m->image[address - m->address];
        }
    }
    fprintf(stderr, "Invalid virtual address: 0x%08x\n", address);
    fflush(stderr);

    static uint32_t null;
    return &null;
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

extern void (*g_handlers[I_INSTRCNT])(darmu_t *du, const darm_t *d);

int darmu_single_step(darmu_t *du)
{
    darm_t d;

    // calculate the raw offset of the program counter
    uint32_t opcode = darmu_read32(du, du->regs[PC]);

    // disassemble the instruction
    int ret = darm_armv7_disasm(&d, opcode);
    if(ret < 0) {
        fprintf(stderr, "Invalid instruction.. 0x%08x\n", opcode);
        return ret;
    }

    uint32_t pc = du->regs[PC];

    if(g_handlers[d.instr] == NULL) {
        darm_str_t str;
        darm_str(&d, &str);
        fprintf(stderr, "[-] instruction '%s' unhandled!\n", str.instr);
        return -1;
    }
    else {
        g_handlers[d.instr](du, &d);
    }

    // increase the program counter if it hasn't been altered
    if(pc == du->regs[PC]) {
        du->regs[PC] += 4;
    }

    return 0;
}

uint8_t darmu_read8(const darmu_t *d, uint32_t addr)
{
    return *(uint8_t *) darmu_mapping_lookup_raw(d, addr);
}

uint16_t darmu_read16(const darmu_t *d, uint32_t addr)
{
    return *(uint16_t *) darmu_mapping_lookup_raw(d, addr);
}

uint32_t darmu_read32(const darmu_t *d, uint32_t addr)
{
    return *darmu_mapping_lookup_raw(d, addr);
}

void darmu_write8(const darmu_t *d, uint32_t addr, uint8_t value)
{
    *(uint8_t *) darmu_mapping_lookup_raw(d, addr) = value;
}

void darmu_write16(const darmu_t *d, uint32_t addr, uint16_t value)
{
    *(uint16_t *) darmu_mapping_lookup_raw(d, addr) = value;
}

void darmu_write32(const darmu_t *d, uint32_t addr, uint32_t value)
{
    *darmu_mapping_lookup_raw(d, addr) = value;
}
