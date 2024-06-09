#include"fs.h"
#include"asm.h"
#include<stdio.h>
#include<string.h>
#include<stdlib.h>

/*
     @brief: creates entry of file/directory in current directory
*/
void create_entry(const char* filename,int i_num){
     struct buffer_head* bh=NULL;
     int dir_num=uarea.curr_dir->i_num;
     struct dir_entry *entry=(struct dir_entry*)malloc(sizeof(struct dir_entry));

     entry->inode=i_num;
     strcpy(entry->name,filename);

     int blkno=bmap(uarea.curr_dir,uarea.curr_dir->i_size);
     if(!(bh=bread(DEVICE_NUM,blkno))){
          panic("couldn't create entry due to failed buffer allocation\n");
     }

     int index=((uarea.curr_dir->i_size/16)%DIR_ENTRIES_PER_BLOCK);
     ((struct dir_entry*)bh->b_data)[index]= *(entry);
     //bh->b_status.b_dirt=true;
     //ll_rw_block(WRITE,bh);
     bwrite(bh);
     brelse(bh);
}

/*
     @brief: returns index of the component where string end or next char=/ 
*/
static inline int next_comp(const char* str,int start,int len){
     int end=start;
     while(end < len && str[end]!= '/'  ){    
          end++;
     }
     if(end==len || str[end]=='/' )
          end-=1;
     return end;
}

/* 
 	@name: seach_dir
	@brief: searches for an entry in the given dir
	@return: returns inode number of the entry if found else -1 
*/

static int search_dir(const char* filename,struct ii_node* inode){
	//struct ii_node* inode=NULL;
	int c=0,index=0;
	unsigned short i_found=-1;
	struct buffer_head* bh=NULL;
	struct dir_entry* entry=NULL;
	//if(!(inode=iget(DEVICE_NUM,inum)))
	//	return -1;
	if(inode->filetype==1){ //means to search in a directory it must be a directory
		return -2;
	}
	
	while(c!=inode->i_size){
		if(c%1024==0)
		{	if(bh!=NULL){
				brelse(bh);
			}
			blkno=bmap(inode,c);
			if(!(bh=bread(inode->i_dev,blkno))
				panic("Couldn't read buffer\n");
			//index++;
			
		}
		*entry=((struct dir_entry*)bh->b_data[c%1024]);
		if(strcmp(filename,entry->name)){
			i_found=entry->inode;
			break;
		}
		c+=(sizeof(dir_entry));;
	}
	return i_found;
} 


/*
     @name: namei
     @brief: converts path name to inode
     @return: locked inode

*/
struct ii_node* namei(const char* pathname){
     
     struct ii_node* working=NULL;
     char* str=pathname;
     int index=0;
     int len=0,inode=0;
     if((len=strlen(pathname))<=0)
          return NULL;
     if(pathname[0]=='/')
          working=iget(DEVICE_NUM,0);
     else
          working=uarea.curr_dir;
     while(index<len){

	if(str[index]=='/' ){
		int end=next_comp(pathname,index+1,len);  
	  	char* comp=(char*)malloc(sizeof(char)*(end-index+1));
	 	strncpy(comp,str+index,end-index+1);
         	inode=search_dir(comp,working);
	  	if(inode==-1){
			return NULL;
		}	
	  }
	  iput(working);
	  working=iget(DEVICE_NUM,inode);
	}
	index++;
     }
     return working; //working is locked inode due to iget() algo is called internally by it 

}




