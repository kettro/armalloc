#include "armalloc.h"
#include "armalloc_def.h"

typedef union u_ledger_t{
  uint16_t container[8];
  uint64_t first_four_containers;
} ledger_t;

// Local Variables
ledger_t ledger[DIVISIONS_PER_MEMSPACE]; // Store all alloc-able blocks
uint8_t division_count[DIVISIONS_PER_MEMSPACE]; // Store # of blocks left
uint16_t mask[DIVISIONS_PER_MEMSPACE] = { 0x01, 0x03, 0xF, 0xFF, 0xFFFF }; // block mask for setting/clearing

// Functions:

/* Function: alloc()
 * Purpose: To find and allocate a sufficiently sized block of memory
 * Returns: Pointer to the allocated block of memory
 * Arguments: Size request, in bytes: minimum size of desired block
 */
void* alloc(uint16_t request)
{
  // Declare automatics
  uint8_t division, container, bit; // coordinates
  uint16_t* ledger_ptr; // eventual pointer to the ledger entry
  ledger_t* ledger_division_ptr; // temporary pointer to the division entry in the ledger

  // Ensure the request is smaller than the Max Block Size
  if(request > BLOCK_SIZE_MAX){ return NULL; }

  // The division category sorta based on the size: 0x80. 0x100, etc
  // Each successive division is twice the size of the previous one
  for(division = 0; division < DIVISIONS_PER_MEMSPACE; division++){
    // Size 0: request <= 0x80 * 2^0
    // Size 1: request <= 0x80 * 2^1 ...
    if(request <= BYTES_PER_BLOCK << division){ break; }
  }

  // check for free blocks in the division category
  for(;!division_count[division];  division++){
    // if the division == 0, this skips an if-check
    if(division < DIVISIONS_PER_MEMSPACE){ continue; }
    /* else if(division == _SIZE_MAX) */return NULL; // Ledger Error: Incorrect count
  }

  // find the first container with a free spot
  // check if the first 4 containers are allocated: if so, default the container index to 4
  ledger_division_ptr = &(ledger[division]);
  container = (ledger_division_ptr->first_four_containers < SIXTY_FOUR_BITS) ? 0 : 4;
  for(; ledger_division_ptr->container[container] == 0xFFFF; container++){
    if(container == CONTAINERS_PER_DIVISION - 1){ return NULL; }
  }

  // Find the first free bit- loop 0/4/8/C -> BITMAX
  ledger_ptr = &(ledger_division_ptr->container[container]); // Save the finalized location as a pointer - faster than using the struct
  // check the value of the container, if >1 byte, can start there
  // costs an extra 8 cycles per check, but caps the cycle increase to 4 checks
  bit = (*ledger_ptr & 0xF)<0xF ? 0 : (*ledger_ptr & 0xFF)<0xFF ? 4 : (*ledger_ptr & 0xFFF)<0xFFF ? 8 : 12;
  for(;*ledger_ptr & (1<<bit); bit++){
    if(bit >= BITS_PER_CONTAINER - 1){ return NULL; } // ledger error: incorrect count kept
  }

  // update the ledger
  *ledger_ptr |= mask[division] << bit;
  division_count[division]--; // Update the count

  // Generate the pointer from the coordinates
  // 1<<7 = blocks/division, 1<<4 = blocks/container
  uint16_t index = (division << BLOCKS_PER_DIVISION_SHIFT) + (container << BLOCKS_PER_CONTAINER_SHIFT) + bit;
  uint32_t offset = index << BLOCKS_PER_DIVISION_SHIFT; // 1<<7 = blocks in a division

  // Return the pointer: the offset + the memory start
  return (void*)((char*)SYS_MEM_START + offset);
}

/* Function: dealloc()
 * Purpose: To free memory previously allocated, now no longer in use. 
 * Returns: none
 * Arguments: A pointer the the start of a block of memory. 
 */
void dealloc(void* pointer)
{
  uint16_t index;
  uint8_t division, container, bit; // coordinates
  uint16_t* ledger_ptr;
  // calculate the index from the pointer
  // 1<<7 = blocks/division
  index = (uint16_t)(((uintptr_t)pointer - SYS_MEM_START) >> BLOCKS_PER_DIVISION_SHIFT);

  // calculate the coordinates:
  // => Modulo is emulated by: x % 2^n == x & (2^n - 1)
  division = index >> BLOCKS_PER_DIVISION_SHIFT;
  container = (index & (BLOCKS_PER_DIVISION - 1)) >> BLOCKS_PER_CONTAINER_SHIFT;
  bit = (index & BLOCKS_PER_CONTAINER - 1);

  ledger_ptr = &(ledger[division].container[container]);

  // Check if bit is set:
  if(!(*ledger_ptr & (1<<bit))){ return; } // is free

  // Mark as free, update the ledger
  *ledger_ptr &= ~(mask[division] << bit);
  division_count[division]++;

  pointer = NULL;
}

/* Function: alloc_init()
 * Purpose: To initialize the number of blocks per division category
 * Returns: none
 * Arguments: none
 */
void alloc_init(void)
{
  // Initialisation code for the allocation utility;
  // to be run by the OS at startup.
  int i;
  // Each larger Size is twice as big as the smaller
  for(i = 0; i < DIVISIONS_PER_MEMSPACE; i++){
    division_count[i] = (BLOCKS_PER_DIVISION >> i);
  }
}
