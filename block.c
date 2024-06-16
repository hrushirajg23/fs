#include<stdio.h>
#include"fs.h"
#include"kernel.h"
#include"asm.h"
#include<string.h>
#define NR_BLK_DEV ((sizeof(rd_blk))/sizeof(rd_blk[0]))


struct filesystem deb;
char logical_blocks[BLOCK_SIZE*100];

int block_write(int dev,long* pos,char* buf,int count){
     int block= *pos/BLOCK_SIZE;
     int offset= *pos % BLOCK_SIZE;
     int chars=0;
     int written=0;
     struct buffer_head* bh=NULL;
     register char* p=NULL;
     while(count > 0){
          bh=bread(dev,block);
          if(!bh){
               if(written!=0){
                    return written;
               }
               else{
                    return -1;
               }
          }
          /*
               if(count < block size chars=count else chars=blksize)
          */
          chars=(count < BLOCK_SIZE) ? count: BLOCK_SIZE;
          p=offset+bh->b_data;
          offset=0;
          block++;
          *pos+=chars;
          written+=chars;
          count-=chars;
          while(chars-->0){
             //  *(p++)=get_fs_byte(buf++);
               
          }
          bh->b_status.b_dirt=1;
          brelse(bh);
     
     }
     return written;
}

int block_read(int dev,long* pos,char* buf,int count){
     int block= (*pos)/BLOCK_SIZE;
     int offset=(*pos)%BLOCK_SIZE;
     int chars=0;   
     int read=0;
     struct buffer_head* bh=NULL;
     register char* p=NULL;

     while(count > 0){
          bh=bread(dev,block);
          if(!bh){
               if(read!=0){
                    return read;
               }
               else{
                    return -1;
               }
          }
          chars=(count < BLOCK_SIZE ) ? count: BLOCK_SIZE;
          p=offset+bh->b_data;
          offset=0;
          block++;
          *pos+= chars;
          read+=chars;
          count-=chars;
          while(chars-->0){
           //    put_fs_byte(*(p++),buf++);
          }
          bh->b_status.b_dirt=1;
          brelse(bh);
     }
     return read;
}

extern void rw_hd(int rw,struct buffer_head* bh);

struct buffer_head* alloc(int dev){
     struct buffer_head* bh=NULL;
     while(super_block.s_locked)
          continue;
     
     int blkno=super_block.s_free_blk_ls->array[super_block.s_next_free_blk--];
     if(!(bh=getblk(dev,blkno)))
          return NULL;
     memset(bh->b_data,'\0',sizeof(bh->b_data));
     super_block.s_mod=true;
     return bh;
}

void free_blk(int num){};
void ll_rw_block(int rw,struct buffer_head* bh){
     int blkno=bh->b_blocknr;
     if(rw==READ){
          printf("\nReading from %p to %p\n",&logical_blocks[blkno*BLOCK_SIZE],&logical_blocks[blkno*2*BLOCK_SIZE]);
          memcpy(bh->b_data,logical_blocks+(blkno*BLOCK_SIZE),BLOCK_SIZE);
     }
     else if(rw==WRITE){
          memcpy(logical_blocks+(blkno*BLOCK_SIZE),bh->b_data,BLOCK_SIZE);
     }
     else{
          panic("GIve either read or write\n");
     }
}

