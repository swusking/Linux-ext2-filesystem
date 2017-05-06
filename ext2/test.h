#include <stdio.h>

#define BLOCK_SIZE 1024	/* 块大小 */
#define BLOCK_COUNT 8708	/* 块的数量 1+1+1+1+512+8192*/

#define SUPER_BLOCK_SIZE 64
#define BLOCK_GROUP_DES_SIZE 32
#define INODE_SIZE 64 
#define INODE_TABLE_SIZE 524288

#define SUPER_BLOCK_START 0 // 
#define BLOCK_GROUP_DES_START 1024 //
#define BLOCK_BITMAP_START 2048 // 
#define INODE_BITMAP_START 3072 //
#define INODE_TABLE_START 4096 //1024*4 
#define DATA_BLOCK_START 528384 // 1024*516k

#define VOLUME_NAME "EXT2"

#pragma comment(lib, "WS2_32.lib")
/*
int 4 4294967296  10位数 

*/ 


/*********
大小为 64 bytes 
定义一个超级块
64 - (16+4*6+20) = 4

卷名 
inode和block的总数
inode和block的剩余数
block和inode的大小
建立文件系统的时间 
**********/
struct super_block{
	char sb_volume_name[16];   //卷名 
	unsigned int sb_inodes_count;  //inode总数 = 8*1024
	unsigned int sb_blocks_count;  //block总数 = 8708
	unsigned int sb_free_inodes_count;  //剩余inode数 
	unsigned int sb_free_blocks_count;  //剩余block数 
	unsigned int sb_block_size;  //选择最简单的 1024byte 
	unsigned int sb_inode_size;  // 64byte 
	char sb_wtime[20];  //2016-06-01  24:24:24 一共20bytes  
	char sb_pad[4]; //填充 
	 
};

/********* 
定义一个块组描述表 32byte 

32 - 4*6 = 8
block bitmap从第几块开始
inode bitmap从第几块开始
inode table从第几块开始
inode，block的剩余数 
本块组的目录总数 
**********/

struct block_group_desc{
	unsigned int bgd_block_bitmap_start;  //block bitmap开始块数 
	unsigned int bgd_inode_bitmap_start;  //inode bitmap开始块数 
	unsigned int bgd_inode_table_start;  //inode table 开始块数 
	unsigned int bgd_free_inode_count;  //inode剩余数 
	unsigned int bgd_free_block_count;  //block剩余数 
	unsigned int bgd_used_dirs_count;  //目录总数 
	char bgd_pad[8];  //填充 
}; 

/********
定义一个inode_table   64
64 -  7*4 - 2*8 =  20

文件类型和所属权限 0/1 000 000 000 
该文件或者目录的大小 
访问时间 atime 
创建时间 ctime 
修改时间 mtime
删除时间 dtime
block的数量  
太复杂了：直接叫8个直接索引 所以一个文件最大能够8*1024 = 8K大小 

指向的数据块号 8个：6个直接，一个间接，一个双间接 总的数据块1024*8 = 8192  只需要2个字节记录 
	因为block=1024bytes：
		6个直接：6k
		1个间接：1*512*1K = 512k
		1个双间接：1*512*512*1k = 256M  
*******/
struct inode{
	unsigned int i_mode;  //文件权限 
	unsigned int i_size;  //文件或者目录大小 
	unsigned int i_atime;  //访问时间 
	char i_ctime[20];      //创建时间 
	unsigned int i_mtime;  //修改时间 
	unsigned int i_dtime;  //删除时间 
	unsigned int i_blocks_count; //占用的block的数量 
	unsigned short i_block[8];  //内容指向, 存放block号 
	char i_pad[4];  //填充 
};

/**********
目录体结构  16bytes   一共有1024/16 = 64个目录体 

索引节点号  <8192
目录项长度  
文件名长度
文件类型
文件名 
**********/
struct dir_entry{
	unsigned short inode; //索引节点号
	unsigned short rec_len; //目录项长度
	unsigned short name_len; //文件名长度
	char file_type; //文件类型（1：普通文件，2：代表目录）
	char name[9]; //文件名	
};



//放在下面，因为定义在上面 

static char block_buffer[1024];  
char test[8708*1024];  //测试用 
char inode_table_buffer[522240]; //测试用 
char filebuffer[8192]=""; //文件内容缓存
 
//重点：为什么要指针 ------fwrite 
//使用指针时必须分配地址，所以用数组 
struct super_block super_block_buffer[1];	//超级块缓冲区 
struct block_group_desc block_group_desc_buffer[1];	/* 组描述符缓冲区 */
struct inode inode_buffer[1];  /* 节点缓冲区 */
struct dir_entry dir[64]; //64*16 = 1024 = 1block
 
unsigned char block_bitmap_buffer[1024]=""; //block bitmap缓冲区 
unsigned char inode_bitmap_buffer[1024]=""; //inode bitmap缓冲区 


unsigned int last_inode_bit=1;  //记录最后一次的inode bit 
unsigned int last_block_bit=0;  //记录最后一次的block bit 
unsigned int current_dir_inode; //当前目录的inode 
unsigned int current_dir_length; //当前目录的长度 
char current_path[256]="";    //当前路径名  [root@sking ~]#   16bytes  将13位替换
//char path_first[13]=""; 
char path_last[4]="";
char path_name[241]=""; 
char time_now[32] = "";  //存储当前时间 

FILE *pf;



//函数声明

void read_super_block(void); //从磁盘中读出super_block 到缓冲区
void write_super_block(void); //将缓冲区的super_block 写入磁盘 

void read_block_group_desc(void); // 从磁盘中读出block_group_des 到缓冲区 
void write_block_group_desc(void);  //将缓冲区的block_group_desc 写入磁盘 

void read_block_bitmap(void); //将bit map 读到缓冲区block_buffer
void write_block_bitmap(void); //将缓冲区block_buffer写入磁盘 

void read_inode_bitmap(void); //将bit map 读到缓冲区inode_buffer
void write_inode_bitmap(void); //将缓冲区inode_buffer写入磁盘 

void read_inode(int);  //读取一个inode信息 
void write_inode(int); //写入一个inode信息 

void read_block(int);  //读取一个block信息 
void write_block(int); //写入一个block信息 

void read_dir(int); //读取一个dir信息 
void write_dir(int); //写入一个dir信息 

unsigned int get_free_block(void); //返回一个空闲的block
unsigned int get_free_inode(void); //返回一个空闲的inode 

void remove_block(int); //回收一个block bit
void remove_inode(int); //回收一个inode bit

void current_time(void); //设置当前时间 

void create_fileSystem(); //新建文件系统 

//功能函数声明 
void ls(void); //显示目录下的文件
void mkdir(char temp[9]); //新建一个目录 
void dir_prepare(unsigned int, int); //对新建的目录进行初始化
void cd(char temp[9]); //切换目录
void format(void); //格式化磁盘 
void rmdir(char temp[9]); //删除空目录
void logout(void); //退出登录
void help(void); //显示帮助命令
void dumpe2fs(void); //显示磁盘信息
void vi(char temp[9]); //创建新文件 
void cat(char temp[9]); //查看文件内容 
void rm(char temp[9]); //删除文件 
void ifconfig(void); //显示IP 
void ping(char ip[128]); //ping主机 
void reName(char oldname[128], char newname[128]); //文件重命名 
void data(void); //显示当前时间 
void chmod(char temp[9], int mode); //修改权限 
int search_file(char temp[9], int *i, int *k); //寻找该目录是否有此文件，若有返回1，没有返回0 
