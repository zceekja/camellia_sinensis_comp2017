#include "btreestore.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

void * init_store(uint16_t branching, uint8_t n_processors) {
    // Your code here
    //printf("----------INIT-START----------\n");
    void* k = malloc(sizeof(uint32_t)*branching*2);
    void* infos = malloc(sizeof(struct info*)*branching*100);
    void* c = malloc(8*(branching+1));
    struct node_bt* root = (struct node_bt*) malloc(sizeof(struct node_bt));
    void* helper = malloc(10);
    *(void**) helper = (void*)root;
    root->num_keys = 0;
    root->is_leaf = 1;
    root->keys = (uint32_t*)k;
    root->values = infos;
    root->parent = root; //here
    root->children = c;
    void* tmp = helper;
    tmp+=sizeof(struct node_bt*);
    *(uint16_t*)tmp = branching;
    //printf("init node is %p\n", root);
    return helper;
}

void recur_free(struct node_bt* m){
    free(m->keys);
    for(int i =0; i < m->num_keys; i++){

        if(m->is_leaf == 0){
            recur_free( (struct node_bt*)*(void**)(m->children + i*8));
            if( i == m-> num_keys -1){
                recur_free(  (struct node_bt*)*(void**)(m->children + (i+1)*8));
            }
        }
        free(((struct info*)*(void**)((m->values)+(i*8)))->data);
        free(*(void**)((m->values)+(i*8)));
  
    }
    free(m->values);
    free(m->children);
    free(m);
}
void free_memory(void* helper){
    struct node_bt* root = (struct node_bt*)*(void**)helper;
    int total = 0;
    int* total_ptr = &total;
    recur_free(root);
}
void close_store(void * helper) {

    free_memory(helper);
    free(helper);
    return;
}
uint32_t search_key(void* helper, uint32_t key){
    uint16_t bf = *(uint16_t*) (helper+sizeof(struct node_bt*)); 
    struct node_bt* current = (struct node_bt*) helper;
    while (1){
        for ( int i = 0; i < current->num_keys; i++){
            if(*(current->keys + i) == key){
                return 1; // found
            }
            else if (*(current->keys + i) > key){
                if (current->is_leaf == 0){
                    current = current->children + i;
                    break;
                }
            }
            if ( current->num_keys -1 == i){
                if (current-> is_leaf == 1){
                    return 0; //not found
                }
                else{
                    current = current->children + i + 1;
                }
            }
        }
    }
}
uint32_t search_and_insert(void* helper, uint32_t key){
    uint16_t bf = *(uint16_t*) (helper+sizeof(struct node_bt*)); 
    struct node_bt* current = (struct node_bt*)*(void**)helper;
    while (1){
        for ( int i = 0; i < current->num_keys; i++){
            if(*(current->keys + i) == key){
                return 1; // found
            }
            else if (*(current->keys + i) > key){
                if (current->is_leaf == 0){
                    current = current->children + i;
                    break;
                }
            }
            if ( current->num_keys -1 == i){
                if (current-> is_leaf == 1){
                    return 0; //not found
                }
                else{
                    current = current->children + i + 1;
                }
            }
        }
    }
}

void insert_overflow(void* helper, struct node_bt* current, int flag){

    uint16_t bf = *(uint16_t*) (helper+sizeof(struct node_bt*)); 
    void* right_k = malloc(sizeof(uint32_t)*bf*2);
    void* right_infos =(struct info*) malloc(sizeof(struct info*)*bf*2);
    void* right_c = malloc(8*(1+bf));
    
    memcpy(right_k,(current->keys) + (bf+1)/2,sizeof(uint32_t)*(bf-1)/2);
    memcpy(right_infos,(current->values)+(bf+1)/2*8,8*(bf-1)/2);
    memcpy(right_c,current->children+(8*(bf+1)/2), (bf+1)/2* 8);
    struct node_bt* new_n_right = (struct node_bt*) malloc(sizeof(struct node_bt));
    new_n_right->num_keys = (bf)/2;
    //printf("--2--\n");
    new_n_right->keys = (uint32_t*)right_k;
    //printf("new right node is %p\n", new_n_right);
    new_n_right->values = right_infos;
    new_n_right->children = right_c;
    if (flag == 1){
        new_n_right->is_leaf = 1;
    }
    else{
        new_n_right->is_leaf = 0;
        struct node_bt* cd; 
        for(int i = 0; i < (bf-1)/2 +1; i++){
            cd = (struct node_bt*)(*(void**)(right_c + i*8));

            if( cd->parent != cd){
                cd->parent = new_n_right;
            }
        }
    }


    if(current->parent == current){
        void* k = malloc(sizeof(uint32_t)*bf*2);
        void* infos = (struct info*)malloc(sizeof(struct info*)*bf*2);
        void* c = malloc(8*(1+bf));
        *(void**) c = (void*)current;
        *(void**)(c+8) = (void*)new_n_right;
        memcpy(k,current->keys + (bf-1)/2, sizeof(uint32_t));
        memcpy(infos,current->values +(bf-1)/2*8, 8);
        struct node_bt* new_n = (struct node_bt*) malloc(sizeof(struct node_bt));
        new_n->num_keys = 1;
        new_n->is_leaf = 0;
        new_n->keys = (uint32_t*)k;
        new_n->values = infos;
        new_n->parent = new_n;
        new_n->children = c;
        new_n_right->parent = new_n;
        current->parent = new_n;
        current->num_keys = floor((bf-1)/2.0);
        *(void**) helper = (void*)new_n;
        
        return;
    }
    else{

        struct node_bt* mother = current->parent;
        mother->num_keys+= 1;
        int i = 0;
        while(1){
            if(current == *(void**)((mother->children) + 8*(i))){
                break;
            }
            i++;
        }
        memmove((mother->keys)+i+1, (mother->keys) +i, ((mother->num_keys)-1-i)* sizeof(uint32_t));
        memmove((mother->values)+(i+1)*8, (mother->values)+i*8,((mother->num_keys)-1-i)*8);
        memcpy(mother->keys +i,current->keys + (bf-1)/2, sizeof(uint32_t));
        memcpy((mother->values)+(i)*8, (current->values)+(bf-1)/2*8,8);

        memmove((mother->children)+(8*(i+2)),(mother->children)+(8*(i+1)), (mother->num_keys-1-i)* 8);
        *(void**)((mother->children) + 8*(i+1)) = (void*)new_n_right;
        new_n_right->parent = current->parent;
        current->num_keys = (bf-1)/2;
        if( mother->num_keys >= bf){

            insert_overflow(helper,mother,0);//TODO recursive shit

        }

    }
}

int btree_insert(uint32_t key, void * plaintext, size_t count, uint32_t encryption_key[4], uint64_t nonce, void * helper) {
    uint16_t bf = *(uint16_t*) (helper+sizeof(struct node_bt*)); 
    struct node_bt* current = (struct node_bt*)*(void**)helper;
    struct info* v = (struct info*) malloc(sizeof(struct info));
    void* d = malloc(sizeof(uint64_t)*ceil(count/8.0));
    void* p = malloc(sizeof(uint64_t)*ceil(count/8.0));
     for(int i =0; i < 8*ceil(count/8.0); i++ ){
        *(char*)(d+i) = 0;
        *(char*)(p+i) = 0;
    }   
    memcpy(p,plaintext,count);
    encrypt_tea_ctr( p, encryption_key, nonce,  d, ceil(count/8.0));
    free(p);
    v->size = count;
    v->key[0] = encryption_key[0];
    v->key[1] = encryption_key[1];
    v->key[2] = encryption_key[2];
    v->key[3] = encryption_key[3];
    v->nonce = nonce;
    v->data = d;

    while (1){
        if(current-> is_leaf ==1){
            if(current->num_keys== 0){
                current->num_keys += 1;
                *(current->keys) = key;
                *(void**)(current->values) = (void*) v;
                return 0;
            }
            for(int i=0; i < current->num_keys;i++){
                if(*(current->keys + i) == key){
                    free(d);
                    free(v);
                    return 1; // Error
                }
                else if (*((current->keys) + i) > key){
                    current->num_keys += 1;
                    memmove((current->keys)+i+1, (current->keys) +i, (current->num_keys-1)* sizeof(uint32_t));
                    memmove((current->values)+(i+1)*8, (current->values) +i*8, ((current->num_keys)-1)*sizeof(struct info*));
                    *((current->keys) + i) = key;
                    *(void**)((current->values) +i*8) = (void*)v;
                    if (current->num_keys < bf){
                        return 0;
                    }
                    else{
                        insert_overflow(helper,current,1);//TODO recursive shit
                        return 0;
                    }
                }
                if( i+1 == current->num_keys){
                    current->num_keys += 1;
                    *((current->keys)+ i+1) = key;
                    *(void**)((current->values) +(i+1)*8) = (void*)v;
                    if( current->num_keys <bf){
                        return 0;
                    }
                    else{
                        insert_overflow(helper,current,1);//TODO recursive shit
                        return 0;
                    }
                }
            }
        }
        else{
            for ( int i = 0; i < current->num_keys; i++){
                //printf("--1--\n");
                if(*(current->keys + i) == key){
                    //printf("--1--\n");
                    free(d);
                    free(v);
                    return 1; // Error
                }
                else if (*((current->keys) + i) > key){
                    //printf("--2--\n");
                    current = (struct node_bt*)*(void**)(current->children + (i)*8);
                    break;
                     
                }

                //printf("i is %d\n",i);
                if (i + 1 == current->num_keys){

                    current = (struct node_bt*)*(void**)(current->children + (i+1)*8);

                    break;

                }
            }
        }
    }
    return 0;
}

int btree_retrieve(uint32_t key, struct info * found, void * helper) {
    uint16_t bf = *(uint16_t*) (helper+sizeof(struct node_bt*)); 
    struct node_bt* current = (struct node_bt*)*(void**)helper;
    
    while (1){
        if(current-> is_leaf ==1){
            //printf("---1---\n");
            if(current->num_keys== 0){
                return 1;
            }
            for(int i=0; i < current->num_keys;i++){
                if(*(current->keys + i) == key){
                    found->key[0] = ((struct info*)*(void**)(current->values + i*8))->key[0];
                    found->key[1] = ((struct info*)*(void**)(current->values + i*8))->key[1];
                    found->key[2] = ((struct info*)*(void**)(current->values + i*8))->key[2];
                    found->key[3] = ((struct info*)*(void**)(current->values + i*8))->key[3];
                    found->nonce = ((struct info*)*(void**)(current->values + i*8))->nonce;
                    found->size = ((struct info*)*(void**)(current->values + i*8))->size;
                    found->data = ((struct info*)*(void**)(current->values + i*8))->data;
                    return 0; // found key
                }
            }
            return 1; // not found
        }
        // current node is not leaf
        else{
            for ( int i = 0; i < current->num_keys; i++){
                if(*(current->keys + i) == key){
                    found->key[0] = ((struct info*)*(void**)(current->values + i*8))->key[0];
                    found->key[1] = ((struct info*)*(void**)(current->values + i*8))->key[1];
                    found->key[2] = ((struct info*)*(void**)(current->values + i*8))->key[2];
                    found->key[3] = ((struct info*)*(void**)(current->values + i*8))->key[3];
                    found->nonce = ((struct info*)*(void**)(current->values + i*8))->nonce;
                    found->size = ((struct info*)*(void**)(current->values + i*8))->size;
                    found->data = ((struct info*)*(void**)(current->values + i*8))->data;
                    return 0; // Found key
                }
                else if (*((current->keys) + i) > key){
                    current = (struct node_bt*)*(void**)(current->children + (i)*8);
                    break;
                }
                if (i + 1 == current->num_keys){
                    current = (struct node_bt*)*(void**)(current->children + (i+1)*8);
                    break;
                }
            }
        }
    }
    return 1;
}

int btree_decrypt(uint32_t key, void * output, void * helper) {
    uint16_t bf = *(uint16_t*) (helper+sizeof(struct node_bt*)); 
    struct node_bt* current = (struct node_bt*)*(void**)helper; 

    while (1){
        if(current-> is_leaf ==1){
            //printf("---1---\n");
            if(current->num_keys== 0){
                // no key
                return 1;
            }
            for(int i=0; i < current->num_keys;i++){
                if(*(current->keys + i) == key){
                    uint32_t size = ((struct info*)*(void**)(current->values + i*8))->size;
                    uint32_t encription_key[4];
                    encription_key[0] =((struct info*)*(void**)(current->values + i*8))->key[0];
                    encription_key[1] =((struct info*)*(void**)(current->values + i*8))->key[1];
                    encription_key[2] =((struct info*)*(void**)(current->values + i*8))->key[2];
                    encription_key[3] =((struct info*)*(void**)(current->values + i*8))->key[3];
                    void* c = ((struct info*)*(void**)(current->values + i*8))->data;
                    uint64_t nonce =  ((struct info*)*(void**)(current->values + i*8))->nonce;
                    void* p = malloc(sizeof(uint64_t)*ceil(size/8.0));
                    for(int i =0; i < 8*ceil(size/8.0); i++ ){
                        *(char*)(p+i) = 0;
                    } 
                    decrypt_tea_ctr( c, encription_key,  nonce,  p, ceil(size/8.0)); 
                    memcpy(output,p,size);
                    free(p);
                    return 0; // found key
                }
            }
            return 1; // not found
        }
        // current node is not leaf
        else{
            for ( int i = 0; i < current->num_keys; i++){
                if(*(current->keys + i) == key){
                    uint32_t size = ((struct info*)*(void**)(current->values + i*8))->size;
                    uint32_t encription_key[4];
                    encription_key[0] =((struct info*)*(void**)(current->values + i*8))->key[0];
                    encription_key[1] =((struct info*)*(void**)(current->values + i*8))->key[1];
                    encription_key[2] =((struct info*)*(void**)(current->values + i*8))->key[2];
                    encription_key[3] =((struct info*)*(void**)(current->values + i*8))->key[3];
                    void* c = ((struct info*)*(void**)(current->values + i*8))->data;
                    uint64_t nonce =  ((struct info*)*(void**)(current->values + i*8))->nonce;
                    void* p = malloc(sizeof(uint64_t)*ceil(size/8.0));
                    for(int i =0; i < 8*ceil(size/8.0); i++ ){
                        *(char*)(p+i) = 0;
                    } 
                    decrypt_tea_ctr( c, encription_key,  nonce,  p, ceil(size/8.0)); 
                    memcpy(output,p,sizeof(uint64_t)*ceil(size/8.0));
                    free(p);
                    return 0; // Found key
                }
                else if (*((current->keys) + i) > key){
                    current = (struct node_bt*)*(void**)(current->children + (i)*8);
                    break;
                }
                if (i + 1 == current->num_keys){
                    current = (struct node_bt*)*(void**)(current->children + (i+1)*8);
                    break;
                }
            }
        }
    }
    return 1;
    return -1;
}

int btree_delete(uint32_t key, void * helper) {
    // Your code here
    printf("--delete--\n");

    return -1;
}

void recur_count(struct node_bt* m, int* total){
    *total += 1;
    for(int i =0; i < m->num_keys; i++){
        if(m->is_leaf == 0){
            recur_count(  (struct node_bt*)*(void**)(m->children + i*8), total);
            if( i == m-> num_keys -1){
                recur_count(  (struct node_bt*)*(void**)(m->children + (i+1)*8), total);
            }
        }
    }
}
int count_node(void* helper){

    struct node_bt* root = (struct node_bt*)*(void**)helper;
    int total = 0;
    int* total_ptr = &total;
    recur_count(root,total_ptr);
    return total;
}
void recur_count_add(struct node_bt* m, int* total, struct node* nodes){
    *total += 1;
    int num = *total -1;
  
    struct node* current = nodes + num;
    void* k = malloc(sizeof(uint32_t)*m->num_keys);
    void* tmp = k;
    memcpy(k,m->keys, sizeof(uint32_t)* (m->num_keys));
    current->keys = (uint32_t*)k;
    current->num_keys = m-> num_keys;
    if( m->is_leaf ==1){
        return;
    }
    for(int i =0; i < m->num_keys; i++){
        recur_count_add( (struct node_bt*)*(void**)(m->children + i*8), total,nodes);
        if( i == m-> num_keys -1){
            recur_count_add( (struct node_bt*)*(void**)(m->children + (i+1)*8) , total,nodes);
        }
    }
}
void add_node_to_list(void* helper, struct node* nodes){
    struct node_bt* root = (struct node_bt*)*(void**)helper;
    int total = 0;
    int* total_ptr = &total;
    recur_count_add(root,total_ptr,nodes);
}

uint64_t btree_export(void * helper, struct node ** list) {
    uint16_t bf = *(uint16_t*) (helper+sizeof(struct node_bt*)); 
    struct node_bt* root = (struct node_bt*)*(void**)helper; 
    int node_nums = count_node(helper);
    if( root->num_keys == 0){
        *list = NULL;
    }

    struct node * nodes = malloc(node_nums * sizeof(struct node));
    *list = nodes;
    add_node_to_list(helper, nodes);
    return node_nums;
}

void encrypt_tea(uint32_t plain[2], uint32_t cipher[2], uint32_t key[4]) {
    int n = 1024;
    cipher[0] = plain[0];
    cipher[1] = plain[1];
    uint32_t delta = 0x9E3779B9;
    uint32_t sum = 0;
    uint32_t tmp1;
    uint32_t tmp2;
    uint32_t tmp3;
    uint32_t tmp4;
    uint32_t tmp5;
    uint32_t tmp6;
    while (n){
        sum = (sum +delta)%(uint32_t)pow(2,32);
        tmp1 = ((cipher[1] <<4)+key[0])%(uint32_t)pow(2,32);
        tmp2 = (cipher[1]+ sum)%(uint32_t)pow(2,32);
        tmp3 = ((cipher[1]>> 5)+key[1])%(uint32_t)pow(2,32);
        cipher[0] = (cipher[0] + (tmp1 ^ tmp2 ^ tmp3))%(uint32_t)pow(2,32);
        tmp4 = ((cipher[0] << 4)+ key[2])%(uint32_t)pow(2,32);
        tmp5 = (cipher[0]+sum)%(uint32_t)pow(2,32);
        tmp6 = ((cipher[0]>>5)+key[3])%(uint32_t)pow(2,32);
        cipher[1] = (cipher[1] + (tmp4 ^ tmp5^ tmp6))%(uint32_t)pow(2,32);
        n--;
    }
    return;
}

void decrypt_tea(uint32_t cipher[2], uint32_t plain[2], uint32_t key[4]) {
    // Your code here
    int n = 1024;
    uint32_t delta = 0x9E3779B9;
    uint32_t sum = 0xDDE6E400;
    uint32_t tmp1;
    uint32_t tmp2;
    uint32_t tmp3;
    uint32_t tmp4;
    uint32_t tmp5;
    uint32_t tmp6;
    while (n){
        tmp4 = ((cipher[0] <<4)+key[2])%(uint32_t)pow(2,32);
        tmp5 = (cipher[0]+ sum)%(uint32_t)pow(2,32);
        tmp6 = ((cipher[0]>> 5)+key[3])%(uint32_t)pow(2,32);
        cipher[1] = (cipher[1] - (tmp4 ^ tmp5 ^ tmp6))%(uint32_t)pow(2,32);
        tmp1 = ((cipher[1] << 4)+ key[0])%(uint32_t)pow(2,32);
        tmp2 = (cipher[1]+sum)%(uint32_t)pow(2,32);
        tmp3 = ((cipher[1]>>5)+key[1])%(uint32_t)pow(2,32);
        cipher[0] = (cipher[0] - (tmp1 ^ tmp2^ tmp3))%(uint32_t)pow(2,32);
        sum = (sum - delta)%(uint32_t)pow(2,32);
        plain[0] = cipher[0];
        plain[1] = cipher[1];
        n--;
    }
    return;
}


uint64_t TEA_encrypt(uint64_t plain, uint32_t key[4]){
    uint64_t sum = 0;
    unsigned delta = 0x9E3779B9;
    uint32_t cipher[2];
    memcpy(cipher ,&plain, sizeof(uint64_t));
    uint64_t n = pow(2,32);
    uint32_t tmp1;
    uint32_t tmp2;
    uint32_t tmp3;
    uint32_t tmp4;
    uint32_t tmp5;
    uint32_t tmp6;
    for(int i = 0; i<1024; i++){
        sum = (sum + delta)%n;
        tmp1 = ((cipher[1]<<4) + key[0]) %n;
        tmp2 = (cipher[1]+ sum)%n;    
        tmp3 = ((cipher[1]>>5) + key[1])%n;
        cipher[0] = (cipher[0] + (tmp1 ^ tmp2 ^ tmp3)) % n;
        tmp4 = ((cipher[0]<<4) +key[2]) %n;
        tmp5 = (cipher[0]+ sum)%n;
        tmp6 = ((cipher[0]>>5) + key[3])%n;
        cipher[1] = (cipher[1]+ (tmp4^ tmp5 ^ tmp6))%n;
    }
    uint64_t cipher_ret;
    memcpy(&cipher_ret, cipher, sizeof(uint64_t));
    return cipher_ret;
}

void encrypt_tea_ctr(uint64_t * plain, uint32_t key[4], uint64_t nonce, uint64_t * cipher, uint32_t num_blocks) {
    uint64_t tmp1;
    uint64_t tmp2;
    for (uint64_t i = 0; i < num_blocks; i++){
        tmp1 = i ^ nonce;
        tmp2 = TEA_encrypt(tmp1,key);
        cipher[i] = plain[i] ^ tmp2;
    }
    return;
}

void decrypt_tea_ctr(uint64_t * cipher, uint32_t key[4], uint64_t nonce, uint64_t * plain, uint32_t num_blocks) {
    uint64_t tmp1;
    uint64_t tmp2;
    for (uint64_t i = 0; i < num_blocks; i++){
        tmp1 = i ^ nonce;
        tmp2 = TEA_encrypt(tmp1,key);
        plain[i] = cipher[i] ^ tmp2;
    }
    return;
}
