#include<stdio.h>
#include"fs.h"


int main(){
    //logical_blocks=malloc(BLOCK_SIZE*100);
    //init_super()
    copy_to_logical_blocks(logical_blocks,&deb);
    init_inodes();
    init_buffers();
    display_buffers();
    display_inodes();

    mount_root(DEVICE_NUM);
    puts("\n-----------------AFter mount-------------------------\n");
    display_inodes();
    display_buffers();
    display_super();
    printf("%lu\n",sizeof(struct disk_inode));  //48
    printf("%lu\n",sizeof(struct disk_inode*)); //8
    printf("%d\n",sizeof(struct super));
    printf("%d\n",sizeof(struct filesystem));
    return 0;
}


/*
gcc main.c buffer.c super.c inode.c panic.c block.c -o myexe

*/

