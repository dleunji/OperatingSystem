#include <stdio.h>
#include <hash.h>
#include <list.h>

#include "frame.h"
#include "swap.h"
#include "page.h"
#include "threads/thread.h"
#include "threads/palloc.h"
#include "threads/malloc.h"
#include "userprog/pagedir.h"
#include "threads/vaddr.h"
#include "lib/kernel/hash.h"
#include "lib/kernel/list.h"

/* To ensure critical sections on frame operations */
static struct lock frame_lock;
/* For mapping from physical address to frame table entry. */
static struct hash frame_map;

/* Using Second Chance Algorithm, manage the frames */
static struct list frame_list; 
static struct list_elem *victim_ptr;

static unsigned frame_hash_func(const struct hash_elem *elem, void *aux);
static bool frame_less_func(const struct hash_elem *he1, const struct hash_elem *he2, void *aux);

/* Frame Table Entry */
struct frame_table_entry{
    void *kpage;    /*Kernel page mapped to physical address*/

    struct hash_elem helem; /*frame_map*/
    struct list_elem lelem; /*frame_list*/

    void *upage;    /*upage : The address of virtual memory or user page that loads the frame.*/
    bool pinned;    /*To prevent swapped out during doing sth*/
    struct thread *t;
};
struct frame_table_entry *next_frame(void);
struct frame_table_entry *second_chance(uint32_t *pagedir);

/* Initialize */
void vm_frame_init(){
    lock_init(&frame_lock);
    hash_init(&frame_map, frame_hash_func,frame_less_func,NULL);
    list_init(&frame_list);
    victim_ptr = NULL;
}

/* Create a frame page corresponding to user virtual address upage. 
After the page mapping, return the kernel address of created page frame. */
void* vm_frame_alloc(enum palloc_flags flag, void *upage){
    lock_acquire(&frame_lock);

    void *frame_page = palloc_get_page(PAL_USER| flag);
    if(frame_page == NULL){
        /* 1. Swap out the page. */
        struct frame_table_entry *out_page = second_chance(thread_current()->pagedir);

        ASSERT(out_page!=NULL &&out_page->t !=NULL);

        /* 2. Clear the page mapping and replace it. */
        ASSERT(out_page->t->pagedir != (void*)0xcccccccc);
        pagedir_clear_page(out_page->t->pagedir,out_page->upage);

        bool is_dirty = false;

        is_dirty = is_dirty || pagedir_is_dirty(out_page->t->pagedir,out_page->upage)
            || pagedir_is_dirty(out_page->t->pagedir,out_page->kpage);

        swap_index_t swap_idx = vm_swap_out(out_page->kpage);
        vm_supt_set_swap(out_page->t->supt, out_page->upage, swap_idx);
        vm_supt_set_dirty(out_page->t->supt, out_page->upage,is_dirty);
        vm_frame_do_free(out_page->kpage,true);

        frame_page = palloc_get_page(PAL_USER|flag);
        ASSERT(frame_page != NULL);

    }
    struct frame_table_entry *frame = malloc(sizeof(struct frame_table_entry));
    if(frame == NULL){
        lock_release(&frame_lock);
        return NULL;
    }
    frame->t = thread_current();
    frame->upage = upage;
    frame->kpage = frame_page;
    frame->pinned = true;

    hash_insert(&frame_map, &frame->helem);
    list_push_back(&frame_list, &frame->lelem);

    lock_release(&frame_lock);
    return frame_page;
}

/* Free the page frame. 
Remove the entry in the frame table, free the memory resource. */ 
void vm_frame_free(void *kpage){
    lock_acquire(&frame_lock);
    vm_frame_do_free(kpage,true);
    lock_release(&frame_lock);
}
/* Just remove the entry from table, do not palloc free */
void vm_frame_remove_entry(void *kpage){
    lock_acquire(&frame_lock);
    vm_frame_do_free(kpage,false);
    lock_release(&frame_lock);
}

void vm_frame_do_free(void *kpage, bool free_page){
    ASSERT(lock_held_by_current_thread(&frame_lock) == true);
    ASSERT(is_kernel_vaddr(kpage));
    ASSERT(pg_ofs(kpage) == 0);

    //For hash_find
    struct frame_table_entry f_tmp;
    f_tmp.kpage = kpage;

    struct hash_elem *h = hash_find(&frame_map, &(f_tmp.helem));
    if(h == NULL){
        PANIC("The page isn't in page table.");
    }

    struct frame_table_entry *frame;
    frame = hash_entry(h, struct frame_table_entry,helem);

    hash_delete(&frame_map, &frame->helem);
    list_remove(&frame->lelem);

    if(free_page) palloc_free_page(kpage);
    free(frame);
}

struct frame_table_entry* next_frame(void){
    if(list_empty(&frame_list))
        PANIC("Frame table is empty.");
    if(victim_ptr == NULL || victim_ptr == list_end(&frame_list))
        victim_ptr = list_begin(&frame_list);
    else
        victim_ptr = list_next(victim_ptr);
    struct frame_table_entry *e = list_entry(victim_ptr, struct frame_table_entry, lelem);
    return e;
}

struct frame_table_entry* second_chance(uint32_t *pagedir){
    size_t size = hash_size(&frame_map);
    if(size == 0)
        PANIC("Frame table is empty.");
    
    size_t i;
    for(i = 0;i < size; i++){
        struct frame_table_entry *e = next_frame();
        if(e->pinned) continue;
        //if referenced, give a second chance
        else if(pagedir_is_accessed(pagedir,e->upage)){
            pagedir_set_accessed(pagedir, e->upage, false);
            continue;
        }
        //Victim
        return e;
    }
    PANIC("Not enough memory!");
}
/*For pinning*/
static void vm_frame_set_pinned(void *kpage, bool new_value){
    lock_acquire(&frame_lock);

    //For hash_find
    struct frame_table_entry f_tmp;
    f_tmp.kpage = kpage;

    struct hash_elem *h = hash_find(&frame_map, &(f_tmp.helem));
    if(h == NULL){
        PANIC("Frames does not exist");
    }

    struct frame_table_entry *frame;
    frame = hash_entry(h, struct frame_table_entry, helem);
    frame->pinned = new_value;
    
    lock_release(&frame_lock);
}

void vm_frame_pin(void*kpage){
    vm_frame_set_pinned(kpage, true);
}
void vm_frame_unpin(void *kpage){
    vm_frame_set_pinned(kpage, false);
}

static unsigned frame_hash_func(const struct hash_elem *elem, void *aux UNUSED){
    struct frame_table_entry *frame = hash_entry(elem, struct frame_table_entry, helem);
    return hash_bytes(&frame->kpage, sizeof frame->kpage);
}
static bool frame_less_func(const struct hash_elem *he1, const struct hash_elem *he2, void *aux UNUSED){
    struct frame_table_entry *f1 = hash_entry(he1, struct frame_table_entry, helem);
    struct frame_table_entry *f2 = hash_entry(he2, struct frame_table_entry, helem);
    return f1->kpage < f2->kpage;
}