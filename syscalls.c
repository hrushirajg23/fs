#include"fs.h"
#include"asm.h"

int create_file(const char* filename,unsigned short permissions){
     struct ii_node* exist=namei(filename);
     struct ii_node* inode=NULL;
     int fd=-1;
     if(exist!=NULL){
          puts("File already exists\n");
          /*
          look into namei if it has any error
          */
          return fd;
     }
     else{/*File does  not exist already*/
          inode=ialloc(DEVICE_NUM);
          //create_dir_entry in parent/curr directory
               //dir entry=inode_num+filename

     }

     //allocate filetable entry for inode, initialize count
     inode->i_status.i_lock=false;
     return fd;
}