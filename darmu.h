#ifndef __DARMU__
#define __DARMU__

// mapping from file offset to virtual offset
typedef struct _darmu_mapping_t {
    uint32_t raw;
    uint32_t raw_size;

    uint32_t virtual;
    uint32_t virtual_size;
} darmu_mapping_t;

#define DARMU_MAPPINGS_COUNT 32

typedef struct _darmu_t {
    uint8_t *image;
    uint8_t *stack;

    uint32_t regs[16];
    uint32_t flags;

    // each file offset to virtual offset mapping
    uint32_t mapping_count;
    darmu_mapping_t mappings[DARMU_MAPPINGS_COUNT];
} darmu_t;

void darmu_init(darmu_t *d, uint8_t *image, uint8_t *stack);

int darmu_mapping_add(darmu_t *d, uint32_t raw, uint32_t raw_size,
    uint32_t virtual, uint32_t virtual_size);
uint32_t darmu_mapping_lookup_virtual(darmu_t *d, uint32_t raw);
uint32_t darmu_mapping_lookup_raw(darmu_t *d, uint32_t virtual);

uint32_t darmu_register_get(darmu_t *d, uint32_t idx);
void darmu_register_set(darmu_t *d, uint32_t idx, uint32_t value);

uint32_t darmu_flags_get(darmu_t *d);
void darmu_flags_set(darmu_t *d, uint32_t value);

int darmu_single_step(darmu_t *du);

#endif
