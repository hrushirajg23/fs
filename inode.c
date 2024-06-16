
#include<string.h>
#include<stdio.h>
#include "kernel.h"
#include "fs.h"
#include<time.h>

int total_inodes=100;

#define _hashfn(dev,num) ((unsigned)(num%dev))

struct inode_pool i_pool;
struct inode_list i_list;


static void write_inode(struct ii_node* inode);
static void read_inode(struct ii_node* inode);

static struct ii_node* get_inode(int num){
    int index=_hashfn(DEVICE_NUM,num);
    struct ii_node* inode=i_pool.i_heads[index].head;

    while(inode!=NULL){
        if(inode->i_num==num){
            return inode;
        }
        inode=inode->next;
    }
    return NULL;
}

static inline void wait_on_inode(struct ii_node* inode){
    while(inode->i_status.i_lock){  //while inode lock==1
        continue;
    }
}

static inline void lock_inode(struct ii_node* inode){
    wait_on_inode(inode);
    inode->i_status.i_lock=1;
}

static inline void unlock_inode(struct ii_node* inode){
    inode->i_status.i_lock=false;
}


static struct ii_node* create_inode(int i_num){
        struct ii_node* inode=(struct ii_node*)malloc(sizeof(struct ii_node));
        inode->i_num=i_num;
        inode->i_access_time=0;
        inode->i_change_time=0;
        inode->i_refcount=0;
        inode->i_dev=DEVICE_NUM;
        inode->i_mode=0x777;  //temporarily allowing all permissions
        inode->i_size=0;
        inode->i_nlinks=0;
        inode->filetype=0;
        inode->next=NULL;
        inode->prev=NULL;
        inode->nextfree=NULL;
        inode->prevfree=NULL;
        inode->i_status.i_lock=false;
        inode->i_status.i_dirt=false;
        inode->i_status.update=true;
        if(inode->i_num==1){
            inode->i_status.i_mount=true;  //'/ is a mount point'
            //inode->i_status.i_lock=true;
        }
        else{
            inode->i_status.i_mount=false;
        }
        memset(inode->i_disk,0,sizeof(inode->i_disk));
        return inode;
}
void sync_inodes(void){
    int i=0;
    struct ii_node* inode=NULL;

    // for()
}

static void insert_into_ipool(struct ii_node* inode){
    int index=_hashfn(inode->i_dev,inode->i_num);
    /*
        in hash_queue
    */
   if(i_pool.i_heads[index].head==NULL && i_pool.i_heads[index].tail==NULL){
        i_pool.i_heads[index].head=inode;
        i_pool.i_heads[index].tail=inode;
    }
    else{  //insert first in queue
        inode->next=i_pool.i_heads[index].head;
        i_pool.i_heads[index].head->prev=inode;
        i_pool.i_heads[index].head=inode;
    } 

    /*
        in free list (remember freelist doubly circular ahe bro)
    */
    if(i_list.head==NULL && i_list.tail==NULL){
        i_list.head=inode;
        i_list.tail=inode;
    }
    else{ //add at end of free list
        i_list.tail->nextfree=inode;
        inode->prevfree=i_list.tail;
        i_list.tail=inode;
        i_list.head->prevfree=i_list.tail;
    i_list.tail->nextfree=i_list.head;
    }
    

}
static void insert_into_ilist(struct ii_node* inode){
    if(i_list.head==NULL && i_list.tail==NULL){
        i_list.head=inode;
        i_list.tail=inode;
    }
    else{
        i_list.tail->nextfree=inode;
        inode->prevfree=i_list.tail;
        i_list.tail=inode;

        i_list.tail->nextfree=i_list.head;
        i_list.head->prevfree=i_list.tail;
    }
}
struct ii_node* get_i_from_pool(int dev,int i_num){
    int index=_hashfn(dev,i_num);
    struct ii_node* inode=i_pool.i_heads[index].head;
    while(inode!=NULL){
        if(inode->i_num==i_num){
            return inode;
        }
        inode=inode->next;
    }
    return NULL;
}

static struct ii_node* deletelast(struct ii_node* inode){
    struct ii_node* get=NULL;
    if(i_list.head==NULL && i_list.tail==NULL){
        return NULL;
    }
    else if(i_list.head==i_list.tail){  //only one inode exists
        get=inode;
        inode->prevfree=NULL;
        inode->nextfree=NULL;
        i_list.head=NULL;
        i_list.tail=NULL;
        return get;
    }
    else{
        get=i_list.tail;
        i_list.tail->prevfree->nextfree=i_list.head;
        i_list.head->prevfree=i_list.tail->prevfree;
        i_list.tail=i_list.tail->prevfree;

        i_list.tail->nextfree=i_list.head;
        i_list.head->prevfree=i_list.tail;
    }
    return get;
}
static struct ii_node* deletefirst(struct ii_node* inode){
    struct ii_node* get=NULL;
    if(inode->prevfree==NULL && inode->nextfree==NULL){
        return NULL;
    }
    else if(inode->prevfree==inode->nextfree){  //only one inode exists
        get=inode;
        inode->prevfree=NULL;
        inode->nextfree=NULL;
        i_list.head=NULL;
        i_list.tail=NULL;
        return get;
    }
    else{
        get=i_list.head;
        i_list.tail->nextfree=i_list.head->nextfree;
        i_list.head->nextfree->prevfree=i_list.tail;
        i_list.head=i_list.head->nextfree;
        i_list.tail->nextfree=i_list.head;
        i_list.head->prevfree=i_list.tail;
    }
    return get;
}

static struct ii_node* remove_from_ilist(struct ii_node* inode){
    struct ii_node* get=NULL;
    if(i_list.head==NULL && i_list.tail==NULL){
        return NULL;
    }
    else if(inode->prevfree==inode->nextfree){  //only one inode exists
        get=deletelast(inode);
        get->nextfree=NULL;
        get->prevfree=NULL;
    }
    else{
        if(inode->nextfree==i_list.head){  //removing if inode is last in circular list
            get=deletelast(inode);
            get->nextfree=NULL;
            get->prevfree=NULL;
            puts("Case delete last \n");
        }
        else if(inode->prevfree==i_list.tail){  //removing if inode is first in circular list
            get=deletefirst(inode);
            get->nextfree=NULL;
            get->prevfree=NULL;
            puts("Case delete first\n");
        }
        else{       //if in middle
            inode->nextfree->prevfree=inode->prevfree;
            inode->prevfree->nextfree=inode->nextfree;
            inode->nextfree=NULL;
            inode->prevfree=NULL;
            get=inode;
            puts("Case delete from middle\n");
        }

    }
    return get;
    
}

static struct ii_node* remove_from_ipool(struct ii_node* inode){
    int index=_hashfn(inode->i_dev,inode->i_num);
    struct ii_node* get=NULL;
    if(i_pool.i_heads[index].head==NULL && i_pool.i_heads[index].tail==NULL){
        return NULL;
    }
    else if(i_pool.i_heads[index].head==i_pool.i_heads[index].tail){
        get=i_pool.i_heads[index].head;
        i_pool.i_heads[index].head=NULL;
        i_pool.i_heads[index].tail=NULL;
    }
    else{
        if(i_pool.i_heads[index].head==inode){  //only execute else step of delete-first = doubly LL
            get=i_pool.i_heads[index].head;
            i_pool.i_heads[index].head=i_pool.i_heads[index].head->next;
            i_pool.i_heads[index].head->prev=NULL;
            
        }
        else if(i_pool.i_heads[index].tail==inode){ //delete last of doubly LL
            get=i_pool.i_heads[index].tail;
            i_pool.i_heads[index].tail=i_pool.i_heads[index].tail->prev;
            i_pool.i_heads[index].tail->next=NULL;
            
        }
        else{
                get=inode;
                get->next->prev=get->prev;
                get->prev->next=get->next;
                get->next=NULL;
                get->prev=NULL;            
        }
    }
     get->next=NULL;
    get->prev=NULL;
    return get;

}


/*
    @fn: init_inodes
    @brief: initalise all in-core inodes
*/
void init_inodes(){
    i_pool.size=DEVICE_NUM;
    for(int i=0;i<i_pool.size;i++){
        i_pool.i_heads[i].head=NULL;
        i_pool.i_heads[i].tail=NULL;
    }
    i_list.head=NULL;
    i_list.tail=NULL;
    int i=0;
    for(i=0;i<TOTAL_INODES;i++){
        struct ii_node* inode=create_inode(i);
        //printf("Created inode %d\n",i);
        insert_into_ipool(inode);
        //insert_into_ilist(inode);
    }
};

/*  @fn: display_inode
    @brief: displays inode hashqueue and inode freelist
*/
void display_inodes(){
    struct ii_node* inode=NULL;
    for(int i=0;i<NR_HASH;i++){
        printf("For header number %d\n",i);
        inode=i_pool.i_heads[i].head;
        while(inode!=NULL){
            printf("inode number %d\n",inode->i_num);
            inode=inode->next;
        }
        puts("\n");
    }
    puts("Displaying free inodes\n");
    inode=i_list.head;
    do{
        printf("i_num %d\t",inode->i_num);
        inode=inode->nextfree;
    }while(inode!=i_list.head);
    puts("\n");
}

/*
    @fn: iget
    @return: locked inode
    @param: FileSystem number
            inode number

*/
struct ii_node* iget(int dev,int i_num){
    struct ii_node* get=NULL;
    repeat:
        if((get=get_i_from_pool(dev,i_num))){
            if(get->i_status.i_lock){ //if it is locked
                //sleep(event: until inodes gets unlocked)
                goto repeat;
            }
            if(get->filetype==0){
                get=remove_from_ilist(get);
                printf("Removed from ilist inode number %d\n",get->i_num);  
            }
            get->i_refcount++;
            return get;
        }
        if(i_list.head==NULL && i_list.tail==NULL){
            panic("No free inodes\n");
            return NULL;
        }
        //now remove first and change its number and place it on correct hashqueue accordingly
        get=deletefirst(i_list.head);
        get=remove_from_ipool(get);
        get->i_num=i_num;/* Reseting inode_num and placing it on correct hashqueue */
       get->i_dev=dev;
        insert_into_ipool(get);
        //int block=2+(get->i_num-1)/INODES_PER_BLOCK;
        read_inode(get);
        get->i_refcount++;
        return get;
}

/*
    @fn: iput
    @param: address of inode
    @brief: rel
*/
void iput(struct ii_node* inode){
    lock_inode(inode);
    inode->i_refcount--;
    if(inode->i_refcount==0){
        if(inode->i_nlinks==0){
            //free disk blocks of respective file
            inode->filetype=0;
            ifree(inode->i_num);
        }
        if(inode->i_status.update==true){
            write_inode(inode);
        }
        insert_into_ilist(inode);
    }
    unlock_inode(inode);
}


/*
    @fn: read_inode
    @param: address of inode
    @brief: This function reads an disk_inode into in-core inode
    It does this via accessing the block number of disk_inodes kept
    then gets a buffer for the block and reads the inodes into the buffer
    It then knows which index to acces as buffer contains an array of inodes


*/

static void read_inode(struct ii_node* inode){
    struct super* sb=NULL;
    struct buffer_head* bh=NULL;
    int block=0;


    lock_inode(inode);
    sb=get_super(inode->i_dev);
    //Linux uses:
    //block=2+sb->s_imap_blocks+sb->s_zmap_blocks+(inode->i_num-1)/INODES_PER_BLOCK;
    /*
        As we'll not be performing bit-operations
    */
    block=2+(inode->i_num-1)/INODES_PER_BLOCK;

    /*
        Suppose INODES_PER_BLOCK =28 
        i.e 0th index to 27th index i.e inode 1 to 28 will be in logical block 3 
        and 28 to 56 will be in logical block 4

    
    
    */
  //  block=deb.dilb[inode->i]
    /*
        s_imap_blocks: A bit-map for indication of free/locked inodes

        s_zmap_blocks: A bit-map for indication of blocks status, whether a block is free or not
    
    */
    if(!(bh=bread(inode->i_dev,block)))
        panic("unable to read inode-block\n");
    
    *(struct disk_inode*)inode=((struct disk_inode*)bh->b_data)[(inode->i_num-1)%INODES_PER_BLOCK];
    brelse(bh);
    unlock_inode(inode);


}
/*

    @fn: write_inode
    @param: inode address
    @brief: This function write an in-core into disk_inode
    It does this via accessing the block number of disk_inodes kept
    then gets a buffer for the block and reads the inodes into the buffer
    It then knows which index to acces as buffer contains an array of inodes
    
*/
static void write_inode(struct ii_node* inode){
    struct super* sb=NULL;
    struct buffer_head* bh=NULL;
    int block=-1;

    lock_inode(inode);
    sb=get_super(inode->i_dev);
   // block=2+sb->s_imap_blocks+sb->s_zmap_blocks+(inode->i_num-1)/INODES_PER_BLOCK;
    block=2+(inode->i_num-1)/INODES_PER_BLOCK;
    if(!(bh=bread(inode->i_dev,block)))
        panic("unable to read i-node block\n");
    int index=(inode->i_num-1)%INODES_PER_BLOCK; //for zero-indexing of array
    ((struct disk_inode* )bh->b_data)[index]= *(struct disk_inode*)inode;
    /*
        converting 1024 bytes of bh->b_data into array of disk inodes
        INODES per block =21
        if inode->i_num=5
        index=4%21= 4th index
        %INODES_PER_BLOCK ensures that more than 21 inodes cannot fit inside block
        *(struct disk_inode*)inode is putting data of incore inode into disk_inode 

    */

    bh->b_status.b_dirt=1;   //delayed write for buffer
    inode->i_status.i_dirt=false;
    brelse(bh);
    unlock_inode(inode);
    
    //writing 
}

/*
    @fn: ialloc
    @param: device number
    @brief: allocates inode to a new file
*/
struct ii_node* ialloc(int dev){
    int block=0;
   while(1){

     if(super_block.s_locked==true){
        //sleep(event: super block becomes free)
        continue;
     }
     if(super_block.s_next_free_inode<0){
        super_block.s_locked=true;
        int index=0;
        int c=0;
        block=2+(super_block.remembered_inode-1)/INODES_PER_BLOCK;
        struct buffer_head* bh=bread(dev,block);
        if(bh==NULL){
            panic("Couldn't allocate block in alloc\n");
        }
        c=TOTAL_INODES;
        while(c >=0){ //jo paryant super-block chi list bharat nahi
            struct ii_node* inode=get_inode(super_block.remembered_inode+1+TOTAL_INODES-c);
            index=(TOTAL_INODES-c)%INODES_PER_BLOCK;
            *(struct disk_inode*)inode=((struct disk_inode*)bh->b_data)[index];
            if(inode->filetype==0){
                super_block.s_free_inodes_ls[c]=inode->i_num;
            }
            inode->i_status.update=true;
            if((TOTAL_INODES-c)+super_block.remembered_inode>50){
                puts("inodes over\n");
                break;
            }
            c--;
        }

        brelse(bh);
        super_block.s_locked=false;
        /*
            if(no free inodes found)
                return no inodes
        */
        super_block.remembered_inode+=TOTAL_INODES-c;
    }
    int get=super_block.s_free_inodes_ls[super_block.s_next_free_inode--];
    struct ii_node* inode=iget(dev,get);
    //create_inode(get);
    if(inode->i_status.update==false){
        block=2+(super_block.remembered_inode-1)/INODES_PER_BLOCK;
        struct buffer_head* buf=bread(dev,block);
        int index=0;
        index=(get-1)%INODES_PER_BLOCK;
        *(struct disk_inode*)inode=((struct disk_inode*)buf->b_data)[index];
    }
    inode->i_status.update=true;  //marking inode is read from disk
    inode->i_status.i_dirt=true;//is in-use
    return inode;

   }
}

/*
    @fn: ifree
    @param: inode number
    @brief: make inode free

*/
void ifree(int num){
    
    super_block.s_next_free_inode++;
    if(super_block.s_locked)
        return;
    if(super_block.s_next_free_inode==TOTAL_INODES-1){
        if(num < super_block.remembered_inode){
            super_block.remembered_inode=num;
        }
    }
    else{
        super_block.s_free_inodes_ls[super_block.s_next_free_inode++]=num;
    }
}


/*

    @fn: bmap
    @param: inode of file
            byte offset to file
    
    @return:    
        logical block number corresponding to byte offset   - output 1
        starting byte of the above block                    - output 2
        number of bytes to copy to user(from above 2)       - output 3
        check whether "read-ahead" can be applicable or not and mark the inode  -  output 4

    @attention: only works for direct blocks     
*/


int bmap(struct ii_node* inode,int offset){
    //int size=inode->i_size;
    int index=offset/BLOCK_SIZE;

    return inode->i_disk[index];
}
