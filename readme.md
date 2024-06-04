#Now I've created a filesystem structure 


struct filesystem{
     char boot_blk[BLOCK_SIZE]; //1024 bytes
     //struct super sb;  //216 bytes and hence allocating 1024 bytes i.e 1 block to it
     char sb[BLOCK_SIZE]; 
     struct disk_inode dilb[50];  //36*512 = 18,432 bytes
     char d_blk_area[BLOCK_SIZE*96]; //96 blocks each of 1024 bytes
}


#The following is provided a block structure with 100 blocks
char logical_blocks[BLOCK_SIZE*100]

#The first 3 members of structure will get copied into 4 blocks

/*
     boot block -> 1st logical block  =index 0 
     super block -> 2nd logical block =index 1
     disk inode list -> 3rd and 4th block =index 2&3

     from index 4(4*BLOCK_SIZE) i.e from block index 4
     logical blocks will start which will be available to store data on file

*/