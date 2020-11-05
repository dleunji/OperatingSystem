#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "list.h"
#include "bitmap.h"
#include "hash.h"
#include "hex_dump.h"

int findIndex(int mode, char * string);

int main(int argc,char* argv[]){
    int mode = 0; //0: list, 1: bitmap 2: hash
    char ch;
    char command[10];
    char type[10];
    char var[10];

    struct list *list[10];
    struct bitmap *bitmap[10];
    struct hash *hash[10];
    char temp[10];
    
    struct list *new_list;
    struct hash *new_hash;

    struct list_elem *e,*e2,*e3;
    struct list_item *e_item;

    list_less_func *less;
    hash_less_func *less2;
    hash_hash_func *h;
    hash_action_func *action;

    struct hash_elem *he,*he2;
    struct hash_iterator *hi;
    void *aux;

    size_t size;
    size_t bit_data;

    int index,index2,pos,pos2;
    int first,last;
    int data;

    if(argc != 0){
        while(1){
            scanf("%s",command);
            if (!strcmp(command,"quit")||command == NULL)
                break;
            
            ////////////////////////////////////////////
            ////////////////////CREATE////////////////// 
            ////////////////////////////////////////////
            else if (!strcmp(command,"create")){
                scanf("%s",type);
                if (!strcmp(type,"list")){
                    mode = 0;
                    scanf("%s",temp);
                    index = findIndex(mode,temp);
                    new_list = (struct list*)malloc(sizeof(struct list));
                    list_init(new_list);
                    list[index] = new_list;  
                }
                else if(!strcmp(type,"bitmap")){
                    mode = 1;
                    scanf("%s %zu",temp,&size);
                    index = findIndex(mode,temp);
                    bitmap[index] = bitmap_create(size); 
                }
                else if(!strcmp(type,"hashtable")){
                    mode = 2;

                    scanf("%s",temp);
                    index = findIndex(mode,temp);
                    new_hash = (struct hash*)malloc(sizeof(struct hash));
                    hash[index] = new_hash;
                    less2 = hash_less;
                    h = hash_func;
                    hash_init(new_hash,h,less2,aux);
                    hash[index] = new_hash;
                }
            }
            ////////////////////////////////////////////
            ////////////////////DELETE////////////////// 
            ////////////////////////////////////////////
            else if (command == "delete"){
                if(mode == 0){
                    scanf("%s",temp);
                    index = findIndex(mode,temp);
                    while(!list_empty(list[index])){
                        e = list_pop_front(list[index]);
                    }
                }
                else if(mode == 1){
                    scanf("%s",temp);
                    index = findIndex(mode,temp);
                    bitmap_destroy(bitmap[index]);
                }
                else {
                    scanf("%s",temp);
                    index = findIndex(mode,temp);
                    hash_destroy(hash[index],action);
                }
            }
            ////////////////////////////////////////////
            /////////////////DUMPDATA///////////////////
            ////////////////////////////////////////////
            else if (!strcmp(command, "dumpdata")){
                scanf("%s",temp);
                index = findIndex(mode,temp);
                if(mode == 0){
                    if(!list_empty(list[index])){
                        for(e = list_begin(list[index]);e !=list_end(list[index]);e = list_next(e)){
                            e_item = list_entry(e,struct list_item,elem);
                            printf("%d ",e_item -> data);
                        }
                        printf("\n");
                    }
                }
                else if(mode == 1){
                    size = bitmap_size(bitmap[index]);
                    for(int i = 0;i < size;i++){
                        printf("%d",bitmap_test(bitmap[index],i));
                    }
                    printf("\n");
                }
                else {
                    if(!hash_empty(hash[index])){
                        hi = (struct hash_iterator *)malloc(sizeof(struct hash_iterator));

                        hash_first(hi,hash[index]);
                        while(hash_next(hi)){
                            he = hash_cur(hi);
                            printf("%d ",he->data);
                        }
                        printf("\n");
                    }
                }
            }
            ////////////////////////////////////////////
            ////////////////////LIST////////////////////
            ////////////////////////////////////////////
            else if(!strcmp(command, "list_empty")){
                scanf("%s",temp);
                index = findIndex(mode,temp);
                if(list_empty(list[index]))
                    printf("true\n");
                else
                    printf("false\n");
            }
            else if(!strcmp(command, "list_size")){
                scanf("%s",temp);
                index = findIndex(mode,temp);
                printf("%zu\n",list_size(list[index]));
            }
            else if(!strcmp(command, "list_max")){
                scanf("%s",temp);
                index = findIndex(mode,temp);
                less = list_less;
                e = list_max(list[index],less,aux);
                e_item = list_entry(e,struct list_item,elem);
                printf("%d\n",e_item -> data);
            }
            else if(!strcmp(command, "list_min")){
                scanf("%s",temp);
                index = findIndex(mode,temp);
                less = list_less;
                e = list_min(list[index],less,aux);
                e_item = list_entry(e,struct list_item,elem);
                printf("%d\n",e_item -> data);

            }
            else if(!strcmp(command,"list_insert")){
                scanf("%s",temp);
                index = findIndex(mode,temp);
                scanf("%d %d",&pos,&data);
                if(pos == 0){
                    e_item = (struct list_item*)malloc(sizeof(struct list_item));
                    e_item -> data = data;
                    e = &(e_item->elem);
                    list_push_front(list[index],e);
                }
                else {
                    if(!list_empty(list[index])){
                        e = list_begin(list[index]);
                        for(int i = 0;i < pos;i++){
                            e = list_next(e);
                        }
                        e_item = (struct list_item*)malloc(sizeof(struct list_item));
                        e_item -> data = data;
                        e2 = &(e_item->elem);
                        list_insert(e,e2);
                    }
                }
            }
            else if(!strcmp(command,"list_insert_ordered")){
                scanf("%s",temp);
                index = findIndex(mode,temp);
                scanf("%d",&data);
                e_item = (struct list_item*)malloc(sizeof(struct list_item));
                e_item -> data = data;
                e = &(e_item->elem);
                less = list_less;
                list_insert_ordered(list[index],e,less,aux);
            }

            else if(!strcmp(command,"list_push_front")){
                scanf("%s",temp);
                index = findIndex(mode,temp);
                scanf("%d",&data);

                e_item = (struct list_item*)malloc(sizeof(struct list_item));
                e_item -> data = data;
                e = &(e_item->elem);
                list_push_front(list[index],e);
            }
            else if(!strcmp(command, "list_push_back")){
                scanf("%s",temp);
                index = findIndex(mode,temp);
                scanf("%d",&data);

                e_item = (struct list_item*)malloc(sizeof(struct list_item));
                e_item -> data = data;
                e = &(e_item->elem);
                list_push_back(list[index],e);
            }
            else if(!strcmp(command, "list_front")){
                scanf("%s",temp);
                index = findIndex(mode,temp);
                e = list_front(list[index]);
                e_item = list_entry(e,struct list_item,elem);
                printf("%d\n",e_item -> data);

            }
            else if(!strcmp(command, "list_back")){
                scanf("%s",temp);
                index = findIndex(mode,temp);
                e = list_back(list[index]);
                e_item = list_entry(e,struct list_item,elem);
                printf("%d\n",e_item -> data);
            }
            else if(!strcmp(command, "list_remove")){
                scanf("%s %d",temp,&pos);
                index = findIndex(mode,temp);
                if(pos == 0)
                    list_pop_front(list[index]);
                else {
                    if(!list_empty(list[index])){
                        e = list_begin(list[index]);
                        for(int i = 0;i < pos;i++){
                            e = list_next(e);
                        }
                        list_remove(e);
                    }
                }
            }
            else if(!strcmp(command, "list_pop_front")){
                scanf("%s",temp);
                index = findIndex(mode,temp);
                list_pop_front(list[index]);
            }
            else if(!strcmp(command, "list_pop_back")){
                scanf("%s",temp);
                index = findIndex(mode,temp);
                list_pop_back(list[index]);               
            } 
            else if(!strcmp(command, "list_reverse")){
                scanf("%s",temp);
                index = findIndex(mode,temp);
                list_reverse(list[index]);
            }
            else if(!strcmp(command, "list_shuffle")){
                scanf("%s",temp);
                index = findIndex(mode,temp);
                list_shuffle(list[index]);
            }
            else if(!strcmp(command, "list_swap")){
                scanf("%s %d %d",temp,&pos,&pos2);
                index = findIndex(mode,temp);

                e = list_begin(list[index]);
                e2 = list_begin(list[index]);

                if(pos != pos2){
                    if (pos > 0){
                        for(int i = 0;i < pos;i++){
                            e = list_next(e);
                        }
                    }
                    if(pos2 > 0){
                        for(int i = 0;i < pos2;i++){
                            e2 = list_next(e2);
                        }
                    }
                    list_swap(e,e2);
                }
            }
            else if(!strcmp(command, "list_sort")){
                scanf("%s",temp);
                index = findIndex(mode,temp);
                less = list_less;
                list_sort(list[index],less,aux);
            }
            else if(!strcmp(command, "list_splice")){
                scanf("%s %d",temp,&pos);
                index = findIndex(mode,temp);
                scanf("%s %d %d",temp,&first,&last);
                index2 = findIndex(mode,temp);

                e = list_begin(list[index2]);
                e2 = list_begin(list[index2]);

                if(first > 0){
                    for(int i =0;i < first;i++){
                        e = list_next(e);
                    }
                }
                if(last > 0){
                    for(int i =0;i < last;i++){
                        e2 = list_next(e2);
                    }
                }
                e3 = list_begin(list[index]);
                if(pos > 0){
                    for(int i =0;i < pos;i++){
                        e3 = list_next(e3);
                    }
                }
                list_splice(e3,e,e2);
            }
            else if(!strcmp(command, "list_unique")){
                scanf("%s",temp);
                index = findIndex(mode,temp);

                less = list_less;
                ch = getchar();
                if(ch == '\n'){
                    list_unique(list[index],NULL,less,aux);
                }
                else {
                    scanf("%s",temp);
                    index2 = findIndex(mode,temp);
                    list_unique(list[index],list[index2],less,aux);
                }
            }
            ////////////////////////////////////////////
            ///////////////////BITMAP/////////////////// 
            ////////////////////////////////////////////
            else if(!strcmp(command,"bitmap_test")){
                scanf("%s %d",temp,&pos);
                index = findIndex(mode,temp);
                data = bitmap_test(bitmap[index],pos);
                if(data == 0)
                    printf("false\n");
                else
                    printf("true\n");
            }
            else if(!strcmp(command, "bitmap_mark")){
                scanf("%s %zu",temp,&bit_data);
                index = findIndex(mode,temp);
                bitmap_mark(bitmap[index],bit_data);
            }
            else if (!strcmp(command, "bitmap_set")){
                scanf("%s %d",temp,&pos);
                index = findIndex(mode,temp);
                scanf("%s",temp);
                if (!strcmp(temp,"true"))
                    data = 1;
                else
                    data = 0;
                bitmap_set(bitmap[index],pos,data);
            }
            else if (!strcmp(command, "bitmap_set_all")){
                scanf("%s",temp);
                index = findIndex(mode,temp);
                scanf("%s",temp);
                if (!strcmp(temp,"true"))
                    data = 1;
                else
                    data = 0;
                bitmap_set_all(bitmap[index],data);
            }
            else if (!strcmp(command, "bitmap_set_multiple")){
                scanf("%s %d %d",temp,&pos,&pos2);
                index = findIndex(mode,temp);
                scanf("%s",temp);
                if (!strcmp(temp,"true"))
                    data = 1;
                else
                    data = 0;
                bitmap_set_multiple(bitmap[index],pos,pos2,data);
            }
            else if(!strcmp(command, "bitmap_all")){
                scanf("%s %d %d",temp,&pos,&pos2);
                index = findIndex(mode,temp);
                if(bitmap_all(bitmap[index],pos,pos2) == 0)
                    printf("false\n");
                else
                    printf("true\n");
            }
            else if(!strcmp(command, "bitmap_any")){
                scanf("%s %d %d",temp,&pos,&pos2);
                index = findIndex(mode,temp);
                if(bitmap_any(bitmap[index],pos,pos2) == 0)
                    printf("false\n");
                else
                    printf("true\n");
            }
            else if(!strcmp(command, "bitmap_contains")){
                scanf("%s %d %d",temp,&pos,&pos2);
                index = findIndex(mode,temp);
                scanf("%s",temp);
                if (!strcmp(temp,"true"))
                    data = 1;
                else
                    data = 0;

                if (bitmap_contains(bitmap[index],pos,pos2,data)==0)
                    printf("false\n");
                else
                    printf("true\n");
            }
            else if(!strcmp(command, "bitmap_count")){
                scanf("%s %d %d",temp,&pos,&pos2);
                index = findIndex(mode,temp);
                scanf("%s",temp);
                if (!strcmp(temp,"true"))
                    data = 1;
                else
                    data = 0;
                printf("%zu\n",bitmap_count(bitmap[index],pos,pos2,data));
            }
            else if(!strcmp(command,"bitmap_dump")){
                scanf("%s %d %d",temp,&pos,&pos2);
                index = findIndex(mode,temp);
                bitmap_dump(bitmap[index]);
            }
            else if(!strcmp(command,"bitmap_expand")){
                scanf("%s %d",temp,&data);
                index = findIndex(mode,temp);
                bitmap[index] = bitmap_expand(bitmap[index],data);
            }
            else if(!strcmp(command,"bitmap_flip")){
                scanf("%s %d",temp,&data);
                index = findIndex(mode,temp);
                bitmap_flip(bitmap[index],data);
            }
            else if(!strcmp(command,"bitmap_none")){
                scanf("%s %d %d",temp,&pos,&pos2);
                index = findIndex(mode,temp);
                if(bitmap_none(bitmap[index],pos,pos2) == 0)
                    printf("false\n");
                else
                    printf("true\n");
            }
            else if(!strcmp(command,"bitmap_reset")){
                scanf("%s %d",temp,&data);
                index = findIndex(mode,temp);
                bitmap_reset(bitmap[index],data);
            }
            else if(!strcmp(command,"bitmap_scan")){
                scanf("%s %d %d",temp,&pos,&pos2);
                index = findIndex(mode,temp);
                scanf("%s",temp);
                if (!strcmp(temp,"true"))
                    data = 1;
                else
                    data = 0;
                printf("%zu\n",bitmap_scan(bitmap[index],pos,pos2,data));
            }
            else if(!strcmp(command,"bitmap_scan_and_flip")){
                scanf("%s %d %d",temp,&pos,&pos2);
                index = findIndex(mode,temp);
                scanf("%s",temp);
                if (!strcmp(temp,"true"))
                    data = 1;
                else
                    data = 0;
                printf("%zu\n",bitmap_scan_and_flip(bitmap[index],pos,pos2,data));
            }
            else if(!strcmp(command, "bitmap_size")){
                scanf("%s",temp);
                index = findIndex(mode,temp);
                size = bitmap_size(bitmap[index]);
                printf("%zu\n",size);
            } 
            ////////////////////////////////////////////
            ////////////////////HASH//////////////////// 
            ////////////////////////////////////////////
            else if(!strcmp(command, "hash_insert")){
                scanf("%s %d",temp,&data);
                index = findIndex(mode,temp);
                he = (struct hash_elem*)malloc(sizeof(struct hash_elem));
                he -> data = data;
                hash_insert(hash[index],he);
            }
            else if(!strcmp(command,"hash_replace")){
                scanf("%s %d",temp,&data);
                index = findIndex(mode,temp);
                he = (struct hash_elem*)malloc(sizeof(struct hash_elem));
                he -> data = data;
                hash_replace(hash[index],he);
            }
            else if(!strcmp(command,"hash_find")){
                scanf("%s %d",temp,&data);
                index = findIndex(mode,temp);
                he = (struct hash_elem*)malloc(sizeof(struct hash_elem));
                he -> data = data;
                he2 = hash_find(hash[index],he);
                if(he2 != NULL)
                    printf("%d\n",data);
            }
            else if(!strcmp(command,"hash_delete")){
                scanf("%s %d",temp,&data);
                index = findIndex(mode,temp);
                he = (struct hash_elem*)malloc(sizeof(struct hash_elem));
                he -> data = data;
                hash_delete(hash[index],he);
            }
            else if(!strcmp(command,"hash_empty")){
                scanf("%s %d",temp,&data);
                index = findIndex(mode,temp);
                if(hash_empty(hash[index]) == 1)
                    printf("true\n");
                else
                    printf("false\n");
            }
            else if(!strcmp(command,"hash_size")){
                scanf("%s %d",temp,&data);
                index = findIndex(mode,temp);
                printf("%zu\n",hash_size(hash[index]));
            }
            else if(!strcmp(command,"hash_clear")){
                scanf("%s %d",temp,&data);
                index = findIndex(mode,temp);
                hash_clear(hash[index],NULL);
            }
            else if(!strcmp(command,"hash_apply")){
                scanf("%s",temp);
                index = findIndex(mode,temp);
                scanf("%s",temp);
                hash[index] -> aux = temp;
                action = hash_action;
                hash_apply(hash[index],action);
            }
        }  
    }
}

int findIndex (int mode, char *string){
    int index;
    if(mode == 0){//list
        if(strlen(string) == 6){
            return 10;
        }
        else {
            index = string[4] - '0';
            return index;
        }
    }

    else if(mode == 1){//bitmap
        if(strlen(string) == 4){
            return 10;
        }
        else {
            index = string[2] - '0';
            return index;
        }
    }

    else { //hash
        if(strlen(string) == 6){
            return 10;
        }
        else {
            index = string[4] - '0';
            return index;
        }
    }
}