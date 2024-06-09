#include"fs.h"
#include<string.h>
#include<stdio.h>
struct super super_block;
struct filesystem deb;

struct ii_node inode_table[TOTAL_INODES]; //in-core inode table
struct file file_table[FILE_TABLE_SIZE]; //file table
struct super* get_super(int dev){
    if(super_block.s_dev==dev){
        return &super_block;
    }
}

void copy_to_logical_blocks(char* logical_blocks,struct filesystem* fs){

    //copying boot block to 1st logical block
    memcpy(logical_blocks,fs->boot_blk,BLOCK_SIZE);
    printf("copied boot block into 1st logical block having address from %p to %p\n",logical_blocks,logical_blocks+1023);
    //copying super block to 2nd logical block
    memcpy(logical_blocks+BLOCK_SIZE,&super_block,BLOCK_SIZE);
    printf("copied super block into 2nd logical block having address from %p to %p\n",logical_blocks+BLOCK_SIZE,logical_blocks+(2*BLOCK_SIZE));
    //copying disk inodes to logical blocks starting from 3rd block
    memcpy(logical_blocks+2*BLOCK_SIZE,fs->dilb,sizeof(fs->dilb)); //18 blocks getting used
    printf("copied super block into 2nd logical block having address from %p to %p\n",logical_blocks+(2*BLOCK_SIZE),logical_blocks+(2*BLOCK_SIZE)+sizeof(fs->dilb));
    
    // int block_for_data=sizeof(fs->dilb)/BLOCK_SIZE;

    // printf("Blocks for dilb needed are %d\n",block_for_data);

    // printf("Blocks for dilb needed are %d\n",(sizeof(fs->dilb)-777)%BLOCK_SIZE);


    //Assume disk inodes need 2 blocks actually need one and a half so approx 2
    
    //hence logical blocks start from 4th block onwards
    
    //copying data blocks area to logical blocks after disk inodes

    memcpy(logical_blocks+(4*BLOCK_SIZE),fs->d_blk_area,BLOCK_SIZE*96);
    printf("copied disk data into logical blocks   from %p to %p\n",logical_blocks+(4*BLOCK_SIZE),logical_blocks+(BLOCK_SIZE*100));    
}


static inline void init_uarea(){
    uarea.curr_dir=NULL;
    memset(uarea.fd_arrays,-1,sizeof(uarea.fd_arrays));
    uarea.root=NULL;
    uarea.io.address=NULL;
    uarea.io.count=0;
    uarea.io.mode=0;
    uarea.io.offset=0;
}

static inline void init_file_table(){
    for(int i=0;i<FILE_TABLE_SIZE;i++){
        file_table[i].f_count=0;
        file_table[i].f_flags=0;
        file_table[i].f_inode=NULL;
        file_table[i].f_mode=0;
        file_table[i].f_pos_read=0;
        file_table[i].f_pos_write=0;
    
    }
}

static inline void init_super(){
    super_block.s_dev=DEVICE_NUM;
    super_block.s_free_blk_ls=(struct blk_node*)malloc(sizeof(struct blk_node));
    super_block.s_free_blk_ls->next=NULL;
    int index=0;
    for(int i=99;i>=0;i--,index++){
            super_block.s_free_blk_ls->array[index]=i;
    }
    /*
    
        index of next free block
    */
    super_block.s_next_free_blk=95;
    super_block.s_locked=false;
    super_block.s_mod=false;
    super_block.s_dev=DEVICE_NUM;
    for(int i=TOTAL_INODES-1;i>=0;i--){
        super_block.s_free_inodes_ls[i]=TOTAL_INODES-1-i;;
    }
    super_block.s_next_free_inode=TOTAL_INODES-1; 
    super_block.remembered_inode=TOTAL_INODES;   

}

void mount_root(int dev){
    struct buffer_head* bh=NULL;
    init_uarea();
    init_file_table();
    init_super();
  
    uarea.root=ialloc(dev);
    uarea.root->filetype=DIR;
    printf("uarea's root inode number is %d\n",uarea.root->i_num);
    //uarea.root=iget(DEVICE_NUM,0); //root inode is kept 0//will make it 1 later
    if(uarea.root==NULL){
        printf("Failed getting inode number 0\n");
    }
    uarea.curr_dir=uarea.root;
    uarea.curr_dir->filetype=DIR;
    bh=bread(dev,1);

    printf(" Super block data is %s\n",bh->b_data);

    brelse(bh);    

}


void display_super(){
    puts("Displaying super block\n");
    for(int i=0;i<=super_block.s_next_free_inode;i++){
        printf("%d\t",super_block.s_free_inodes_ls[i]);
    }
    puts("\n");
}