#include "btreestore.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
int main() {
    // Your own testing code here
    void * helper = init_store(3, 3);
    int key1 = 2;
    char* text1 = "hello";
    uint32_t encryption_key[4];
    uint64_t nonce = 3;
    int key2 = 3;
    char* text2 = "hello";
    int key3 = 1;
    char* text3 = "hello";
    int key4 = 8;
    char* text4 = "hello";
    int key5 = 80;
    int key6 = 5;
    int key7 = 4;
    int key8 = 20;
    int key9 =21;
    int key10 =22;
    btree_insert( key1, (void *)text1 ,  6,  encryption_key,  nonce, helper);
    btree_insert( key2, (void *)text2 ,  6,  encryption_key,  nonce, helper);
    btree_insert( key3, (void *)text3 ,  6,  encryption_key,  nonce, helper);
    btree_insert( key4, (void *)text3 ,  6,  encryption_key,  nonce, helper);
    btree_insert( key5, (void *)text2 ,  6,  encryption_key,  nonce, helper);
    btree_insert( key6, (void *)text3 ,  6,  encryption_key,  nonce, helper);
    btree_insert( key7, (void *)text3 ,  6,  encryption_key,  nonce, helper);
    btree_insert( key8, (void *)text1 ,  6,  encryption_key,  nonce, helper);
    btree_insert( key9, (void *)text1 ,  6,  encryption_key,  nonce, helper);
    btree_insert( key10, (void *)text1 ,  6,  encryption_key,  nonce, helper);
    struct node * list = NULL;
    uint64_t num = btree_export(helper, &list);
    printf("HEHEHEHEHEHEHEH\n");
    // num and elements of list e.g. list[i] will be inspected
    printf("total nodes is %llu\n", num);
    for (int i = 0; i < num; i++){
        printf("number of key is : %d\n",(list+i)->num_keys);
        printf("key is :");
        for(int j =0; j < (list+i)->num_keys; j++){
            printf("%d ",*((list+i)->keys +j));
        }
        printf("\n");
    }
    free(list); // expected that list will point to dynamic memory

    close_store(helper);

    return 0;
}
