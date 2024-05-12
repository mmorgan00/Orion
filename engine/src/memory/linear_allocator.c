#include "linear_allocator.h"

#include "core/omemory.h"
#include "core/logger.h"

void linear_allocator_create(u64 total_size, void* memory, linear_allocator* out_allocator) {
    if (out_allocator) {
        out_allocator->total_size = total_size;
        out_allocator->allocated = 0;
        out_allocator->owns_memory = memory == 0;
        if (memory) {
            out_allocator->memory = memory;
        } else {
            out_allocator->memory = oallocate(total_size, MEMORY_TAG_LINEAR_ALLOCATOR);
        }
    }
}
void linear_allocator_destroy(linear_allocator* allocator) {
    if (allocator) {
        allocator->allocated = 0;
        if (allocator->owns_memory && allocator->memory) {
            ofree(allocator->memory, allocator->total_size, MEMORY_TAG_LINEAR_ALLOCATOR);
        } 
        allocator->memory = 0;
        allocator->total_size = 0;
        allocator->owns_memory = false;
    }
}

void* linear_allocator_allocate(linear_allocator* allocator, u64 size) {
    if (allocator && allocator->memory) {
        if (allocator->allocated + size > allocator->total_size) {
            u64 remaining = allocator->total_size - allocator->allocated;
            OERROR("linear_allocator_allocate - Tried to allocate %lluB, only %lluB remaining.", size, remaining);
            return 0;
        }

        void* block = ((u8*)allocator->memory) + allocator->allocated;
        allocator->allocated += size;
        return block;
    }

    OERROR("linear_allocator_allocate - provided allocator not initialized.");
    return 0;
}

void linear_allocator_free_all(linear_allocator* allocator) {
    if (allocator && allocator->memory) {
        allocator->allocated = 0;
        ozero_memory(allocator->memory, allocator->total_size);
    }
}