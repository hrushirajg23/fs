#include"fs.h"
#include<string.h>
#include<stdio.h>
struct super super_block;
struct filesystem deb;
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
    memcpy(logical_blocks+BLOCK_SIZE,fs->sb,BLOCK_SIZE);
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



