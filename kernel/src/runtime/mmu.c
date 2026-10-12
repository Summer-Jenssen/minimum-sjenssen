
#include <stdint.h>
#include <stddef.h>
#include "minemu/syscall.h"
#include "minemu/mmu.h"
#include <stdbool.h>
uint32_t user_page_vaddrs[MAX_USER_IMAGE_PAGES];
uint32_t user_page_flags[MAX_USER_IMAGE_PAGES];
size_t user_page_count;

//returns 1 if flags match, 0 if not
static int check_flags_and_if_addr_exists(uint32_t page_vaddr, uint32_t flags) {
    int addr_exists = 0;
    for (size_t index = 0; index < user_page_count; ++index) {
        if (user_page_vaddrs[index] == page_vaddr) {
            addr_exists = 1;
            if ((user_page_flags[index] & flags) != flags) {
                return 0;
            }
        }
    }
    if(addr_exists == 0){
        return 0;
    }
    return 1;
}

//returns 1 if valid, 0 if not. isWritable and isReadable should be set to 0 for false, 1 for true. This way you can check for perms.
int validate_mem_range(uint32_t vaddr, uint32_t len, int isWritable, int isReadable){
    uint32_t flags = 0;
    if(isReadable == 1){
        flags = flags | MINEMU_PTE_READABLE;
    }
    if(isWritable == 1){
        flags = flags | MINEMU_PTE_WRITABLE;
    }

    while (len != 0) {
        uint32_t page_vaddr = vaddr & MINEMU_PTE_PAGE_MASK;
        if (check_flags_and_if_addr_exists(page_vaddr, flags) == 0) {
            return false;
        }
        uint32_t offset = vaddr - page_vaddr;
        uint32_t chunk = MINEMU_PAGE_SIZE - offset;
        if (chunk > len) {
            chunk = len;
        }
        vaddr += chunk;
        len -= chunk;
    }
    return true;
}