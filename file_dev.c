#include<stdio.h>
#include"fs.h"
#include"asm.h"
#include"kernel.h"
#include<fcntl.h>


struct ii_node* inode_table[TOTAL_INODES]; //in-core inode table
struct file file_table[FILE_TABLE_SIZE]; //file table
int allocatefd(int f_index){
     
     int i=0;
     while(uarea.fd_arrays[i]!=NULL && i<50){
          i++;
     }
     if(i==50)
          return -1;
     printf("Allocating entry of %d index of ufd array\n",i);
     uarea.fd_arrays[i]=&file_table[f_index];
     return i;
}

int allocatefile(int i_index){
     int i=0;
     while(i<FILE_TABLE_SIZE){
          if(file_table[i].f_inode==NULL)
               file_table[i].f_inode=inode_table[i_index];
               printf("Allocating entry of %d index of filetable\n",i);
               return i;
          i++;
     }
     return INT32_MIN;
}

int allocateitable(struct ii_node* ref){
     int i=0;
     struct ii_node* (*inode)[TOTAL_INODES]=&inode_table;
     /*
          inode_table is an array 28 of struct ii_node*

          inode is an pointer which points to an array 28 of struct ii_node*
     
     */
     while(i<TOTAL_INODES){
          if((*inode)[i]==NULL)
               (*inode)[i]=ref;
               return i;
          i++;
     }
  
  
     if(i==TOTAL_INODES){
          return -1;
     }
     printf("Allocating entry of %d index of ii_table\n",i);
     return i;
     
}

static void display_fdarr(){
     puts("Displaying user area fd array\n");
     for(int i=0;i<50;i++){
          printf("fd index %d\n",i);
          
          printf("points to inode %d\n",uarea.fd_arrays[i]->f_inode->i_num);
     }
}

static void display_file(){
     puts("Displaying file table \n");
     for(int i=0;i<FILE_TABLE_SIZE;i++){
          printf("index is %d\n",i);
          printf("points to inode %d\n",file_table[i].f_inode->i_num);
     }
}

static void display_incoretable(){
     puts("Displaying incore table \n");
     for(int i=0;i<TOTAL_INODES;i++)
     {
          printf("index is %d\n",i);
          printf("inode  is %d\n",inode_table[i]->i_num);
     }
}

/*

     struct ii_node* inode_table[28];

     struct ii_node*  (*inode)  [28]=&


*/