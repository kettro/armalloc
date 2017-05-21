#ifndef _ARMALLOC_DEF_H_
#define _ARMALLOC_DEF_H_

/*
 * -------------------------------------------------
 * |                 MEMORY SPACE                  |
 * -------------------------------------------------
 * |       DIV(0)              |  ~~~  |  DIV(n)   |
 * -------------------------------------------------
 * |   B(0)    | ~~~~~ |  B(m) |
 * -----------------------------
 * | C |~~~| C |
 * -------------
 * |b~b|
 * -----
 */



#define DIVISIONS_PER_MEMSPACE      5

#define BLOCK_SIZE_MAX_SHIFT        11
#define BLOCK_SIZE_MAX              2048
#define BLOCKS_PER_DIVISION_SHIFT   7
#define BLOCKS_PER_DIVISION         128
#define BLOCKS_PER_CONTAINER_SHIFT  4
#define BLOCKS_PER_CONTAINER        16

#define BYTES_PER_BLOCK_SHIFT       7
#define BYTES_PER_BLOCK             128
#define BYTES_PER_DIVISION_SHIFT    14
#define BYTES_PER_DIVISION          16384 // (1<<14)

#define CONTAINERS_PER_DIVISION     8
#define BITS_PER_CONTAINER          16
#define SIXTY_FOUR_BITS             0xFFFFFFFFFFFFFFFF

#define SYS_MEM_START               (unsigned long int)0x20004000
#define SYS_MEM_SIZE                16384 // (1<<14)

#endif
