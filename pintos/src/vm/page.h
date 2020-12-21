#ifndef VM_PAGE_H
#define VM_PAGE_H
#include <hash.h>
#include "vm/swap.h"
#include "filesys/off_t.h"

enum page_status {
    ALL_ZERO,   /* Filled with zeros. */
    ON_FRAME,   /* Active in memory. */
    ON_SWAP,    /* Being Swapped. */
    FROM_FILESYS/* In filesystem. */
};

/* Supplemental page table. It is perprocessable. */
struct supplemental_page_table {
    struct hash page_map;
};

struct supplemental_page_table_entry {
    void *upage;     /* Virtual address of the page. */
    void *kpage;     /* Kernel page(frame) associated, */

    struct hash_elem elem;
    enum page_status status;
    bool dirty;
    //if ON_SWAP status
    swap_index_t swap_index;    /* Store the swap index if the page is swapped out. */

    //if FROM_FILESYS status
    struct file *file;
    off_t file_offset;
    uint32_t read_bytes, zero_bytes;
    bool writable;
};

/* Functions for manipulating supplemental page table. */
struct supplemental_page_table *vm_supt_create(void);
void vm_supt_destroy(struct supplemental_page_table *supt);
bool vm_supt_install_frame(struct supplemental_page_table *supt, void *upage, void *kpage);
bool vm_supt_install_zero(struct supplemental_page_table *supt, void *);
bool vm_supt_install_filesys(struct supplemental_page_table *supt, void *page,
    struct file *file, off_t offset, uint32_t read_bytes, uint32_t zero_bytes,bool writable);

bool vm_supt_set_swap(struct supplemental_page_table *supt, void *, swap_index_t);
struct supplemental_page_table_entry *vm_supt_look_up (struct supplemental_page_table *supt, void *);
bool vm_supt_has_entry(struct supplemental_page_table *supt, void *page);
bool vm_supt_set_dirty(struct supplemental_page_table *supt, void *, bool);
bool vm_load_page(struct supplemental_page_table *supt, uint32_t *pagedir, void *upage);
/*bool vm_supt_mm_unmap(struct supplemental_page_table *supt, uint32_t *pagedir, 
    void *page, struct file *file, off_t offset, size_t bytes); */
void vm_pin_page(struct supplemental_page_table *supt, void *page);
void vm_unpin_page(struct supplemental_page_table *supt, void *page);

#endif

