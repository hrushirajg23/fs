
#include<stdio.h>
#include<stdlib.h>
#include"fs.h"
#include"kernel.h"
extern int end;
struct buffer_head* start_buffer=(struct buffer_head*) &end;
// struct buffer_head* hash_table[NR_HASH];
// static struct buffer_head* freelist=NULL;
int TOTAL_BUFFERS=0;
struct hash_table h_table;
struct freehead freelist;

#define BUFFER_END 0x100000000 // 4 GB mark

#define _hashfn(dev,block) ((unsigned)(block%dev))
#define hash(dev,block) h_table[_hashfn(dev,block)]




static inline void wait_on_buffer(struct buffer_head* bh){
    while(bh->b_status.b_lock){
        puts("waiting for buffer to get unlocked\n");
        continue;  //semaphores etc can be used 
        /*
            putting on continuous while loop will damage cpu resources
            jar multi-processing implement kela tar he change karava lagel
            something like sleep_on task scheduling vagere karava lagel
        */
    }
}

int sys_sync(void){
    int i=0;
    struct buffer_head* bh=NULL;

    sync_inodes();
    for(i=0;i<TOTAL_BUFFERS;i++){
        wait_on_buffer(bh);
            if(bh->b_status.b_dirt)
                ll_rw_block(WRITE,bh);
    }
    return 0;
}

static int sync_dev(int dev){
    int i=0;
    struct buffer_head* bh=NULL;

    bh=start_buffer;

    for(i=0;i<TOTAL_BUFFERS;i++){
        if(bh->b_dev!=dev){
            continue;
        }
        wait_on_buffer(bh);
        if(bh->b_status.b_dirt)
            ll_rw_block(WRITE,bh);/*
            Asynchronously writing buffer to disk;
            */
    }
}

struct buffer_head* create_buffers(unsigned short blkno,unsigned short dev){
    struct buffer_head* bh=(struct buffer_head*)malloc(sizeof(struct buffer_head));
    bh->b_blocknr=blkno;
    bh->b_data=(char*)malloc(sizeof(char)*BLOCK_SIZE);
    bh->next=NULL;
    bh->prev=NULL;
    bh->nextfree=NULL;
    bh->prevfree=NULL;
    bh->b_dev=dev;
    bh->b_status.b_count=0;
    bh->b_status.b_dirt=false;
    bh->b_status.b_lock=false;
    bh->b_status.b_uptodate=true;
}

static void insert_into_queues(struct buffer_head* bh){
    int index=_hashfn(DEVICE_NUM,bh->b_blocknr);
    /*
    Putting buffer at beginning of queue
    */
    if(h_table.header[index].head==NULL && h_table.header[index].tail==NULL){
        h_table.header[index].head=bh;
        h_table.header[index].tail=bh;
    }
    else{
        bh->next=h_table.header[index].head;
        h_table.header[index].head->prev=bh;
        h_table.header[index].head=bh;
    }
 
    /*putting at end of freelist*/
    /*
        Remember Freelist Doubly Cirular ahe
    
    */
    if(freelist.head==NULL && freelist.tail==NULL){
        freelist.head=bh;
        freelist.tail=bh;
    }
    else{
        freelist.tail->nextfree=bh;
        bh->prevfree=freelist.tail;
        freelist.tail=bh;
    }
    freelist.head->prevfree=freelist.tail;
    freelist.tail->nextfree=freelist.head;
}

static void remove_from_queues(struct buffer_head* bh){
    /*
        removing from hash queue
    */
   

   
}


void display();

static struct buffer_head* find_buffer(int dev,int block){
    struct buffer_head* temp=NULL;
    for(temp=h_table.header[(_hashfn(dev,block))].head;temp!=NULL;temp=temp->next){
        if(temp->b_dev==dev && temp->b_blocknr==block){
            return temp;
        }
    }
    return NULL;
}


struct buffer_head* get_hash_table(int dev,int block){
    struct buffer_head* bh=NULL;
    
repeat:
    if(!(bh=find_buffer(dev,block))){
        return NULL;
    }
    bh->b_status.b_count++;
    wait_on_buffer(bh);
    if(bh->b_dev!=dev || bh->b_blocknr!=block){
        brelse(bh);
        goto repeat; 
    }
    return bh;    
}
void init_buffer(void){
   
   //  void *b=(void*)BUFFER_END;
    h_table.size=NR_HASH;
    for(int i=0;i<h_table.size;i++){
        h_table.header[i].head=NULL; //hash table is doubly ended queue
        h_table.header[i].tail=NULL;
    }
    freelist.head=NULL; //freelist is doubly circular queue
    freelist.tail=NULL;
    int i=1;
    //
    while(i<=TOTAL_BLOCKS){
        struct buffer_head* bh=create_buffers(i,DEVICE_NUM);
        
        insert_into_queues(bh);

        TOTAL_BUFFERS++;
        i++;
    }

}

struct buffer_head* getblk(int dev,int block){
    struct buffer_head* temp=NULL;

repeat:
    if(temp=get_hash_table(dev,block))
    {
        if(temp->b_status.b_lock){  /* Scenario 5 */
            wait_on_buffer(temp);
            goto repeat;
        }
        temp->b_status.b_lock=true; /*Scenario 1*/
        temp->b_status.b_count++;
        remove_from_queues(temp);
        return temp;
    }
    else{
        if(freelist.head==NULL && freelist.tail==NULL){
            /*
            sleep(event any buffer becomes free)
            */
           //sleep_on(&temp->wait);
           goto repeat;
        }
        temp->b_status.b_count++;
        remove_from_queues(temp);
        /*
        Scenario 3 : buffer marked for delayed-write 
        */
       if(temp->b_status.b_dirt){
        sync_dev(dev);/*asynchronously write buffer to disk*/
        goto repeat;
       }
       /*
       Scenario 2 : Remove from old hashqueue and put into new hashqueue
       */
       temp->b_dev=dev;
       temp->b_blocknr=block;
       temp->b_status.b_count=0;
       insert_into_queues(temp);
    }

    return temp;
}

void brelse(struct buffer_head* buf){
    if(!buf){
        return;
    }
    wait_on_buffer(buf);
    if(!(buf->b_status.b_count--)){
        panic("Trying to free buffer\n");
    }
    buf->b_status.b_lock=false;  //unlock the buffer
    //actually call wake_up(&buffer_wait)  
    /*
    ha function sagluya waiting processes la alert karto
    */
}


struct buffer_head* bread(int dev,int block){
    struct buffer_head* bh=NULL;

    if(!(bh=getblk(dev,block)))
        panic("bread: getblk failed\n");
    if(bh->b_status.b_uptodate)
        return bh;
    ll_rw_block(READ,bh);
    puts("Reading block\n");
    if(bh->b_status.b_uptodate){
        return bh;
    }
    brelse(bh);
    return NULL;
}




void bwrite(struct buffer_head* bh){
    //initiate disk write 
    /*
        if(I/O is synchronous){
            sleep(event: until I/O completes)
            brelse(bh);
        }
    */
    if(bh->b_status.b_dirt==true){
        bh->b_status.b_uptodate=false; /*
            marking buffer as "old" i.e not upto-date 
            so it will remain at head of list
        */
    }
}