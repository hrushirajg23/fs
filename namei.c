#include"fs.h"
#include"asm.h"
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include"kernel.h"
#include<stdbool.h>

char* strconcat(const char* str1,const char* str2);

/*
     @brief: creates entry of file/directory in current directory
*/
void create_entry(const char* filename,unsigned short i_num){
     struct buffer_head* bh=NULL;
     int dir_num=uarea.curr_dir->i_num;
	int i=0;
     struct dir_entry *entry=(struct dir_entry*)malloc(sizeof(struct dir_entry));

     entry->inode=i_num;
     strncpy(entry->name,filename,sizeof(entry->name)-1);
	entry->name[sizeof(entry->name)-1]='\0';

	printf("now entry inode= %d\n",entry->inode);
	printf("now entry name= %s\n",entry->name);
	
	// if((bh=alloc(DEVICE_NUM))){
	// 	panic("Couldn't allocate block for buffer\n");
	// }

	if(  ((uarea.curr_dir->i_size/BLOCK_SIZE)!=((uarea.curr_dir->i_size+16)/BLOCK_SIZE)) || (uarea.curr_dir->i_size==0)  ){
		printf("%d\n",uarea.curr_dir->i_size);
		printf("Entered the condition of filesize==0\n");
		bh=alloc(DEVICE_NUM);
		i=((uarea.curr_dir->i_size+16)/BLOCK_SIZE);
		uarea.curr_dir->i_disk[i]=bh->b_blocknr;
		brelse(bh);
	}

     int blkno=bmap(uarea.curr_dir,uarea.curr_dir->i_size);
     printf("In entry function bmapped to block number %d\n",blkno);
	if(!(bh=bread(DEVICE_NUM,blkno))){
          panic("couldn't create entry due to failed buffer allocation\n");
     }
	puts("bread successfull\n");
     int index=((uarea.curr_dir->i_size/16)%DIR_ENTRIES_PER_BLOCK);
	int offset=index*sizeof(struct dir_entry);
	printf("got index %d\n",index);
	//((struct dir_entry*)bh->b_data)[index]= *(entry);
	memcpy(bh->b_data+offset,&(entry->inode),sizeof(unsigned short));
	char* name_pos=bh->b_data+offset+sizeof(unsigned short);
	strcpy(name_pos,entry->name);
	struct dir_entry* copied=(struct dir_entry*)(bh->b_data+offset);
	
	puts("After copying entry to buffer\n");
	
	printf("%u\n",copied->inode);
	printf("%s\n",copied->name);
     //bh->b_status.b_dirt=true;
     //ll_rw_block(WRITE,bh);
     bwrite(bh);
	uarea.curr_dir->i_size+=sizeof(struct dir_entry);
     brelse(bh);
	entry=NULL;
	free(entry);
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
	printf("file name is : %s",filename);
	//struct ii_node* inode=NULL;
	int c=0,index=0;
	int i_found=-1;
	bool bFlag=false;
	struct buffer_head* bh=NULL;
	struct dir_entry* entry=NULL;
	int blkno=0;
	//if(!(inode=iget(DEVICE_NUM,inum)))
	//	return -1;
	if(inode->filetype==1){ //means to search in a directory it must be a directory
		return -2;
	}
	entry=(struct dir_entry*)malloc(sizeof(struct dir_entry));
	struct dir_entry* trial=(struct dir_entry*)malloc(sizeof(struct dir_entry));
	printf("In search dir size of directory is %d\n",inode->i_size);
	while(c!=inode->i_size){
		if(c%1024==0)
		{	if(bh!=NULL){
				brelse(bh);
			}
			blkno=bmap(inode,c);
			if(!(bh=bread(inode->i_dev,blkno)))
				panic("Couldn't read buffer\n");
			
		}
		printf("before comparing i_found is %d\n",i_found);
		memcpy(&entry->inode,&(bh->b_data[c%1024]),sizeof(unsigned short));
		memcpy(&entry->name,&(bh->b_data[(c%1024)+sizeof(unsigned short)]),sizeof(entry->name));
		//entry=((struct dir_entry*)bh->b_data+(c%1024));
		printf("%d\n",entry->inode);
		printf("%s\n",entry->name);
		if(strcmp(filename,entry->name)==0){
			puts("file entry matched");
			printf("%s\n",filename);
			printf("%s\n",entry->name);
			i_found=entry->inode;
			bFlag=true;
			break;
		}
		c+=(sizeof(struct dir_entry));;
	}

	brelse(bh);
	printf("bFlag value is %d\n",bFlag);
	printf("\nReturning ifound %d\n",i_found);
	if(bFlag==false){
		return -1;
	}
	else{
		return i_found;
	}
} 


static inline bool check_curr(const char* name){
	const char* p=name;

	while(*p!='\0'){
		if((*p)=='/')
			return true;
		p++;
	}
	return false;
}

/*
     @name: namei
     @brief: converts path name to inode
     @return: locked inode

*/

struct ii_node* namei(const char* pathname){
     
	puts("Entered namei\n");
	printf("pathname is %s\n",pathname);
     struct ii_node* working=NULL;
	char* str=NULL;
	char root='/';
	if(!(check_curr(pathname)))
	{
		puts("Entered if\n");	
		str=strcat(&root,pathname);
	}
     else{
		str=(char*)malloc(sizeof(char)*(sizeof(pathname)+1));
		puts("Entered else\n");
		memset(str,'\0',sizeof(pathname)+1);
		strcpy(str,pathname);
     }
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
	 	strncpy(comp,str+index+1,end-index+1);
         	inode=search_dir(comp,working);
	  	if(inode==-1){
			puts("inode == -1 NOt found in directory ");

			return NULL;
		}	
	  }
	  iput(working);
	  working=iget(DEVICE_NUM,inode);
	  index++;
	}
	
     
     return working; //working is locked inode due to iget() algo is called internally by it 
}

/*
	unix.txt
	12345678
*/

/*
	@name: mkdir

*/

void mkdir(const char* filename,unsigned short imode){
	struct ii_node* curr=uarea.curr_dir;
	
}


char* strconcat(const char* str1,const char* str2){

	size_t len1=strlen(str1);
	size_t len2=strlen(str2);

	char* result=(char*)malloc(len1+len2+1);

	if(result==NULL){
		fprintf(stderr,"Dynamic memory allocation failed\n");
		exit(1);
	}

	strcpy(result,str1);
	strcat(result,str2);

	return result;
}



