#include<stdio.h>
#include"fs.h"
#include<string.h>

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
     printf("blkno of root is : %d\n",uarea.curr_dir->i_disk[0]);

    struct dir_entry* entry=(struct dir_entry*)malloc(sizeof(struct dir_entry));
    
    puts("block 4 logical \n");
    memcpy(&entry->inode,&logical_blocks[(BLOCK_SIZE*4)],sizeof(unsigned short));
    memcpy(entry->name,&logical_blocks[(BLOCK_SIZE*4)+sizeof(unsigned short)],sizeof(entry->name));
    printf("%u\n",entry->inode);
    printf("%s\n",entry->name);

    puts("block 4 logical \n");
    memcpy(&entry->inode,&logical_blocks[(BLOCK_SIZE*4)+sizeof(struct dir_entry)],sizeof(unsigned short));
    memcpy(entry->name,&logical_blocks[(BLOCK_SIZE*4)+sizeof(unsigned short)+sizeof(struct dir_entry)],sizeof(entry->name));
    printf("%u\n",entry->inode);
    printf("%s\n",entry->name);
    
    puts("block 4 logical \n");
    memcpy(&entry->inode,&logical_blocks[(BLOCK_SIZE*4)+(sizeof(struct dir_entry)*2)],sizeof(unsigned short));
    memcpy(entry->name,&logical_blocks[(BLOCK_SIZE*4)+sizeof(unsigned short)+(sizeof(struct dir_entry)*2)],sizeof(entry->name));
    printf("%u\n",entry->inode);
    printf("%s\n",entry->name);
    
     puts("block 4 logical \n");
    memcpy(&entry->inode,&logical_blocks[(BLOCK_SIZE*4)+(sizeof(struct dir_entry)*3)],sizeof(unsigned short));
    memcpy(entry->name,&logical_blocks[(BLOCK_SIZE*4)+sizeof(unsigned short)+(sizeof(struct dir_entry)*3)],sizeof(entry->name));
    printf("%u\n",entry->inode);
    printf("%s\n",entry->name);

     puts("block 4 logical \n");
    memcpy(&entry->inode,&logical_blocks[(BLOCK_SIZE*4)+(sizeof(struct dir_entry)*4)],sizeof(unsigned short));
    memcpy(entry->name,&logical_blocks[(BLOCK_SIZE*4)+sizeof(unsigned short)+(sizeof(struct dir_entry)*4)],sizeof(entry->name));
    printf("%u\n",entry->inode);
    printf("%s\n",entry->name);

     puts("block 4 logical \n");
    memcpy(&entry->inode,&logical_blocks[(BLOCK_SIZE*4)+(sizeof(struct dir_entry)*5)],sizeof(unsigned short));
    memcpy(entry->name,&logical_blocks[(BLOCK_SIZE*4)+sizeof(unsigned short)+(sizeof(struct dir_entry)*5)],sizeof(entry->name));
    printf("%u\n",entry->inode);
    printf("%s\n",entry->name);
    


    printf("%u\n",sizeof(struct disk_inode));  //48
    printf("%u\n",sizeof(struct disk_inode*)); //8
    printf("%d\n",sizeof(struct super));
    printf("%d\n",sizeof(struct filesystem));
    return 0;
}


/*
gcc main.c buffer.c super.c inode.c panic.c block.c namei.c syscalls.c file_dev.c -o myexe

*/

