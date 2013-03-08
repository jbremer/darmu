#ifndef __DARMU__
#define __DARMU__

// mapping from file offset to virtual offset
typedef struct _darmu_mapping_t {
    // data blob
    uint8_t *image;

    // raw size of this blob
    uint32_t raw_size;

    // virtual address of this blob
    uint32_t address;
} darmu_mapping_t;

#define DARMU_MAPPINGS_COUNT 32

typedef struct _darmu_t {
    uint8_t *image;
    uint8_t *stack;

    uint32_t regs[16];
    struct {
        uint32_t reserved : 28;
        uint8_t N : 1; // negative result
        uint8_t Z : 1; // zero result
        uint8_t C : 1; // carry bit
        uint8_t V : 1; // overflowed
    } flags;

    // each file offset to virtual offset mapping
    uint32_t mapping_count;
    darmu_mapping_t mappings[DARMU_MAPPINGS_COUNT];
} darmu_t;

void darmu_init(darmu_t *d, uint8_t *stack, uint32_t stack_size);

int darmu_mapping_add(darmu_t *d, uint8_t *image, uint32_t raw_size,
    uint32_t address);
uint32_t darmu_mapping_lookup_virtual(const darmu_t *d, uint8_t *raw);
uint32_t *darmu_mapping_lookup_raw(const darmu_t *d, uint32_t address);

uint32_t darmu_register_get(darmu_t *d, uint32_t idx);
void darmu_register_set(darmu_t *d, uint32_t idx, uint32_t value);

uint32_t darmu_flags_get(darmu_t *d);
void darmu_flags_set(darmu_t *d, uint32_t value);

int darmu_single_step(darmu_t *du);

uint8_t darmu_read8(const darmu_t *d, uint32_t addr);
uint16_t darmu_read16(const darmu_t *d, uint32_t addr);
uint32_t darmu_read32(const darmu_t *d, uint32_t addr);

void darmu_write8(const darmu_t *d, uint32_t addr, uint8_t value);
void darmu_write16(const darmu_t *d, uint32_t addr, uint16_t value);
void darmu_write32(const darmu_t *d, uint32_t addr, uint32_t value);

#endif
