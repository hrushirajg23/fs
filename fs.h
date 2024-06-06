#include<sys/types.h>
#include<stdbool.h>
#include<stddef.h>
#include<stdint.h>
#include<stdlib.h>
#ifndef _FILESYS_H
#define _FILESYS_H
#define TOTAL_INODES 28
#define FILE_TABLE_SIZE 64
#define TOTAL_BUFFERS tot_buffs
#define BLOCK_SIZE 1024
#define SUPER_BLOCKS 8   
#define NR_HASH 4

#define READ 0
#define WRITE 1

#define REGULAR 1
#define DIR 2
#define SPECIAL 3

#define DEVICE_NUM 4
//assuming this

#define I_MAP_SLOTS 8
#define Z_MAP_SLOTS 8

#define TOTAL_BLOCKS 100
extern int total_inodes;

#define INODES_PER_BLOCK ((BLOCK_SIZE)/sizeof(struct disk_inode))
#define DIR_ENTRIES_PER_BLOCK ((BLOCK_SIZE)/sizeof(struct dir_entry))


#define NAME_SIZE 20

typedef char buffer_block[BLOCK_SIZE];
struct status{
    bool b_lock;  /*0- free ,1 -locked*/
    unsigned short b_count; /*user using this block*/
    bool b_dirt;
     bool b_uptodate;
};


//extern int inode_table[];
struct buffer_head{
    char* b_data; //ptr to data //1024 bytes
    unsigned short b_dev; //if ==0 means free
    unsigned short b_blocknr; //block number
    struct status b_status;
    struct buffer_head* prev;
    struct buffer_head* next;
    struct buffer_head* nextfree;
    struct buffer_head* prevfree;
};

struct disk_inode{
    unsigned short i_uid;   //owner identifier
    unsigned short i_gid;   //group identifier
    short filetype;
    unsigned short i_mode;
    unsigned long i_size;
    unsigned long i_mtime;
    unsigned short i_nlinks;
    unsigned short i_disk[9];
    
};

struct inode_status{
    bool i_mount; //whether inode is a mount point 
    bool i_lock;
    bool i_dirt;    
    bool update;
};

//this is the memory i.e in-core inode
struct ii_node{
    unsigned short i_uid;   //owner identifier
    unsigned short i_gid;   //group identifier
    short filetype;
    unsigned short i_mode;
    unsigned long i_size;
    unsigned long i_mtime;
    unsigned short i_nlinks;
    unsigned short i_disk[9];
    
    //extra for in-core manipulation
    unsigned long i_access_time;
    unsigned long i_change_time;
    unsigned short i_dev;
    unsigned short i_num;
    unsigned short i_refcount;
    struct inode_status i_status;
    struct ii_node* next;
    struct ii_node* prev;
    struct ii_node* nextfree;
    struct ii_node* prevfree;

};  


/*
File table entry
*/
struct file{
    unsigned short f_mode;
    unsigned short f_flags;
    unsigned short f_count;
    struct ii_node* f_inode;
    off_t f_pos_read; //read offset
    off_t f_pos_write; //write offset
};

/*
    @brief: I/O parameters used in uarea
*/
struct u_io{
    unsigned short mode;  //READ or WRITE
    int count; //number of bytes to read or write
    int offset; //byte offset in file
    char* address; //target address to copy data, in user memory
};

/*
    @brief: user area of process
*/
struct u_area {
    int fd_arrays[50];
    struct ii_node* curr_dir;
    struct ii_node* root;
    struct u_io io;
}uarea;


struct super{
    /*
        Regarding blocks
    */
    int s_fs_size; //size_of_filesystem
    unsigned short s_nfree_blk; //number of freeblocks
    unsigned short s_free_blk_ls[50]; //list of free disk block numbers
    unsigned short s_next_free_blk; //index of next_free block number
    int* blk_list;
    /*
        Regarding inode
    */
    unsigned short s_nfree_inodes; //number of free inodes
    unsigned short s_free_inodes_ls[TOTAL_INODES]; //list of free inodes
    unsigned short s_next_free_inode; //index of next free inode on free inode list
    short remembered_inode;
    /*
        Others
    */
   unsigned short s_dev; //device number
    bool s_locked;
    bool s_mod; //whether super block is modified

};
/*
    A directory entry consists of 16 bytes
    2 bytes: inode number
    14 bytes= 14 characters= filename

*/
struct dir_entry{
    unsigned short inode;
    char name[NAME_SIZE];
};

/*
    @brief: freelist of buffers
*/
struct freehead{
    struct buffer_head* head;
    struct buffer_head* tail;
};

/*
    @brief: buffer_cache's headers
*/
struct hash_head{
    struct buffer_head* head;
    struct buffer_head* tail;
};


/*
    @brief: buffer cache hashqueue
*/
struct hash_table{
    struct hash_head header[NR_HASH];
    uint8_t size;
};

/*For buffers*/
extern struct hash_table h_table;
extern struct freehead freelist;

/*
    @brief: inode pool headers
*/
struct i_header{
    struct ii_node* head;
    struct ii_node* tail;
};

/*
    @brief: hashqueues for inodes
*/
struct inode_pool{
    struct i_header i_heads[NR_HASH];
    int size;
};

/*
   @brief: Freelist for inodes
*/
struct inode_list{
    struct ii_node* head;
    struct ii_node* tail;
};

/*
    For inodes
*/
extern struct inode_pool i_pool;
extern struct inode_list i_list;



extern struct ii_node inode_table[TOTAL_INODES]; //in-core inode table
extern struct file file_table[FILE_TABLE_SIZE]; //file table
extern struct super super_block;
/*
The ufs has more than one super block 
whereas SV(System V ) has a single super block
If experimenting with may super block fails use one only
*/

extern struct buffer_head* start_buffer;
extern int tot_buffers;

extern int bmap(struct ii_node* inode,int block);
extern int create_block(struct ii_node* inode,int block);
extern struct ii_node* namei(const char* pathname);
extern int open_namei(const char* pathname,int flag,int mode,struct ii_node** res_inode);
extern void iput(struct ii_node* inode);

extern void init_inodes();
extern struct ii_node* iget(int dev,int num);
extern struct ii_node* get_empty_inode(void);
extern struct buffer_head* get_hash_table(int dev,int block);
extern struct buffer_head* getblk(int dev,int block);
extern void ll_rw_block(int rw,struct buffer_head* bh);
extern void brelse(struct buffer_head* buf);
extern struct buffer_head* bread(int dev,int block);
extern void bwrite(struct buffer_head* bh);
extern void init_buffers();

extern struct buffer_head* alloc(int dev);
extern void free_blk(int num);

extern int new_block(int dev);
extern void free_block(int dev,int block);
extern struct ii_node* new_inode(int dev);
extern void ifree(int num);
extern void sync_inodes(void);
extern void mount_root(int dev);
extern struct ii_node* ialloc(int dev);
extern void display_inodes();
extern void display_buffers();
extern void display_super();
/*
extern tells the compiler that the function or variable is defined in another source file or translation unit
inline suggests to the compiler to inline the function to reduce call overhead.
*/
extern struct super* get_super(int dev);



struct filesystem{
    char boot_blk[BLOCK_SIZE]; //1024 bytes
    //struct super sb;  //216 bytes and hence allocating 1024 bytes i.e 1 block to it
    char sb[BLOCK_SIZE]; 
    struct disk_inode dilb[50];  //36*512 = 18,432 bytes
    char d_blk_area[BLOCK_SIZE*96]; //96 blocks each of 1024 bytes
};

extern struct filesystem deb;
extern char logical_blocks[BLOCK_SIZE*TOTAL_BLOCKS];

extern void copy_to_logical_blocks(char* logical_blocks,struct filesystem* fs);

#endif