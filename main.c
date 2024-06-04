#include<stdio.h>
#include"fs.h"


int main(){
    //logical_blocks=malloc(BLOCK_SIZE*100);
    copy_to_logical_blocks(logical_blocks,&deb);
    printf("%lu\n",sizeof(struct disk_inode));  //48
    printf("%lu\n",sizeof(struct disk_inode*)); //8
    printf("%d\n",sizeof(struct super));
    printf("%d\n",sizeof(struct filesystem));
    return 0;
}


/*
gcc main.c buffer.c super.c inode.c panic.c block.c -o myexe

*/