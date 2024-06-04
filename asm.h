extern inline unsigned char get_fs_byte(const char* addr){
     return *(unsigned char*)addr;
}

extern inline void put_fs_byte(char val,char* addr){
     *addr=val;
}