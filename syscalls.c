#include"fs.h"
#include"asm.h"
#include"kernel.h"
#include<stdio.h>
#include<string.h>

int create_file(const char* filename,unsigned short permissions){
    // char* result=strconcat(uarea.pwd,filename);
     struct ii_node* exist=namei(filename);
     struct ii_node* inode=NULL;
     int i=0,file=0;
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
          create_entry(filename,inode->i_num);
               //dir entry=inode_num+filename

     }
     

     //allocate filetable entry for inode, initialize count
      if((i=allocateitable(inode))==-1)
          panic("INODE TABLE FULL\n");
     
     /*
          if file alread exists free blocks()
     */
     if((file=allocatefile(i))==INT32_MIN)
          panic("FILE TABLE FULL\n");
     
     if((fd=allocatefd(file))==-1)
          panic("user file descriptor array FULL\n ");
     
     inode->i_status.i_lock=false;
     inode->i_refcount++;
     return fd;
}

static int find_ref(int inode){
     int i=0;
     while(inode_table[i]->i_num!=inode && i<TOTAL_INODES)
          i++;
     return i;
}
int openfile(const char* filename,int flags,int perm){
     int fd=-1,file=0,i=0;
     struct ii_node* inode=NULL;
     // if(strcmp(filename,)==0){
          
     // }
    // char* result=strconcat(uarea.pwd,filename);
     int i_index=0;
     if(!(inode=namei(filename)))
          return -1;
     if(inode->filetype!=1)
          return -1;
     /*
     
          add permissions part later
          assume all have permissions
     */
     if(inode->i_refcount>0){
          i_index=find_ref(inode->i_num);  
     }
     else{
           if((i_index=allocateitable(inode))==-1)
          panic("INODE TABLE FULL\n");
            //allocate filetable entry for inode, initialize count
     }
    
     
     /*
          if file alread exists free blocks()
     */
     if((file=allocatefile(i_index))==INT32_MIN)
          panic("FILE TABLE FULL\n");
     
     if((fd=allocatefd(file))==-1)
          panic("user file descriptor array FULL\n "); 
     
     inode->i_status.i_lock=false;
     inode->i_refcount++;

     return fd;

}