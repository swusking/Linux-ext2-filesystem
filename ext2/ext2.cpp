#include <stdio.h>
#include <string.h>
#include <time.h>
#include "test.h"
#include <stdlib.h>
#include <ctype.h> //大小写转换
#include <winsock2.h>
#include <windows.h>



int main(void){
	printf("Welcome to sking's simple ext2\n");
	printf("******************************\n");
	printf("**  Author：@sking        ****\n");
	printf("**  Create：2016-6-8      ****\n");
	printf("**  Blog  ：www.sking.xin ****\n");
	printf("******************************\n\n");
	
	pf = fopen("fileSystem.dat", "r+b");
	if(!pf){		
		printf("The File system does not exist!, it will be creating now!\n");
		create_fileSystem();
	}
	
	pf = fopen("fileSystem.dat", "r+b");
	last_inode_bit=1;  
	last_block_bit=0; 
	read_super_block();
	read_block_group_desc();

	//目录名 
	strcpy(current_path, "[root@sking ");
	strcpy(path_name, "/");
	strcpy(path_last, "]# ");
	
	current_dir_inode=1; 
	current_dir_length=1;
	
	int flag = 1;
//	printf("[root@sking /]# ");
	while(1){
		strcpy(current_path, "[root@sking ");
		strcat(current_path, path_name);
		strcat(current_path, path_last);

		char command[10]="", temp[9]="";  //存储命令和变量 

		fflush(stdin); //清除stdin，就不会出现两个提示符了 
		printf("%s", current_path);
			//如果是回车，则重显 

		if(flag){
			command[0] = getchar();
			if(command[0] == '\n'){
				continue;
			}
		}
		scanf("%s", command+1);
		
		if(!strcmp(command, "ls")){
			ls();

		}else if(!strcmp(command, "mkdir")){
			scanf("%s", temp);
			mkdir(temp);
			
		}else if(!strcmp(command, "cd")){
			scanf("%s", temp);
			cd(temp);
			
		}else if(!strcmp(command, "format")){
			format();
			
		}else if(!strcmp(command, "rmdir")){
			scanf("%s", temp);
			rmdir(temp);
			
		}else if(!strcmp(command, "help")){
			help();
			
		}else if(!strcmp(command, "logout")){
			logout();
			
		}else if(!strcmp(command, "dumpe2fs")){
			dumpe2fs();
			
		}else if(!strcmp(command, "vi")){
			scanf("%s", temp);
			vi(temp);
			
		}else if(!strcmp(command, "cat")){
			scanf("%s", temp);
			cat(temp);
			
		}else if(!strcmp(command, "rm")){
			scanf("%s", temp);
			rm(temp);
			
		}else if(!strcmp(command, "ifconfig")){
			ifconfig();

		}else if(!strcmp(command, "ping")){
			char ip[128];
			scanf("%s", ip);
			ping(ip);

		}else if(!strcmp(command, "rename")){
			char oldname[9];
			char newname[9];
			scanf("%s %s", oldname, newname);
			reName(oldname, newname);

		}else if(!strcmp(command, "data")){
			data();

		}else if(!strcmp(command, "chmod")){
			int mode; 
			scanf("%s", temp);
			scanf("%d", &mode);
			chmod(temp, mode);

		}else{
			printf("Can't find this command！\n");
		}

	}
	
	return 0;

}




void create_fileSystem(void){

	//创建文件系统的存储位置 ，虚拟磁盘
	//inode从1开始，block从0开始 
	last_inode_bit=1;
    last_block_bit=0;
	int i = 0;
	
   	printf("Please wait..\n");
	while(i<20){
		printf(".");
		Sleep(100);
		i++;
	}
	
	i=0;
	//将fileSystem.dat作为模拟磁盘 
	pf = fopen("fileSystem.dat", "w+b");

	if(!pf){
		printf("open file filed!"); //打开文件失败 
		exit(0);
	}
	
	
	//将buffer清空，好对磁盘进行格式化 
	for(i=0; i<BLOCK_SIZE; i++){		
		block_buffer[i] = 0;
		//printf("%d", i); 
	}

	//格式化磁盘
	fseek(pf, 0, SEEK_SET); 
	for(i=0; i < BLOCK_COUNT; i++){		
		fwrite(block_buffer, BLOCK_SIZE, 1, pf);  //向磁盘写入0进行格式化 
	}
	fflush(pf);
	
	 
//	fseek(pf, INODE_TABLE_START, SEEK_SET);
//	fread(inode_table_buffer, INODE_TABLE_SIZE, 1, pf); 
//	for(i=0; i < INODE_TABLE_SIZE; i++){
//		printf("%d", inode_table_buffer[i]);
//	} 
	

	//初始化super_block, 并写入磁盘中的block 
	read_super_block();
	strcpy(super_block_buffer->sb_volume_name, VOLUME_NAME);
	super_block_buffer->sb_inodes_count = 8192;
	super_block_buffer->sb_blocks_count = 8708;
	super_block_buffer->sb_free_inodes_count = 8192-1; //inode号从1开始，所以空闲的少一个 
	super_block_buffer->sb_free_blocks_count = 8708;
	super_block_buffer->sb_block_size = BLOCK_SIZE;
	super_block_buffer->sb_inode_size = 64;
	current_time();
	strcpy(super_block_buffer->sb_wtime, time_now);
	write_super_block();

	

	
	//初始化block_group_desc, 并写入磁盘 
	read_block_group_desc();
	block_group_desc_buffer->bgd_block_bitmap_start = BLOCK_BITMAP_START;
	block_group_desc_buffer->bgd_inode_bitmap_start = INODE_BITMAP_START;
	block_group_desc_buffer->bgd_inode_table_start = INODE_TABLE_START;
	block_group_desc_buffer->bgd_free_inode_count = 1024*8-1;
	block_group_desc_buffer->bgd_free_block_count = 1024*8;
	block_group_desc_buffer->bgd_used_dirs_count = 0;
	write_block_group_desc();
	
	//格式时已初始化block bitmap
	//读入到block_bitmap_buffer缓冲区 
	read_block_bitmap();
	//正常 
//	for(i=0; i<1024; i++){
//		printf("%d", block_bitmap_buffer[i]);
//	}
	//格式时已初始化inode bitmap 
	//读入到inode_bitmap_buffer缓冲区 
	read_inode_bitmap();
	//正常 
//	for(i=0; i<1024; i++){
//		printf("%d", inode_bitmap_buffer[i]);
//	}

	//设置根目录
	read_inode(current_dir_inode);//将第一个inode读入inode_buffer
	
	inode_buffer->i_mode = 755; //rwx r-x r-x
	inode_buffer->i_size = 16*2; //一个block占1024bytes 
	inode_buffer->i_atime = 0;
	current_time();
	strcpy(inode_buffer->i_ctime, time_now);
	inode_buffer->i_mtime = 0;
	inode_buffer->i_dtime = 0;
	inode_buffer->i_blocks_count = 1;
	inode_buffer->i_block[0] = get_free_block(); //分配一个空闲的block bit 
	
	current_dir_inode = get_free_inode(); //分配一个空闲的inode bit 
	current_dir_length = 1; //根分区的长度“/” 


	//printf("%u\n", current_dir_inode);
	write_inode(current_dir_inode);

	
	
	read_dir(0); //写到缓冲区 
	//初始化子目录“..”和“.” 
	dir[0].inode = dir[1].inode = current_dir_inode;
	dir[0].name_len = 1;
	dir[1].name_len = 1;
	dir[0].file_type = dir[1].file_type = 2;
	strcpy(dir[0].name, ".");
	strcpy(dir[1].name, "..");
	for(i=2; i<64; i++){
		dir[i].inode = 0;
	}
	write_dir(inode_buffer->i_block[0]);

	printf("\nthe ext2 file system has been installed!\n\n\n");

	fclose(pf);
}


//读写 super block 
void read_super_block(void){
	fseek(pf, SUPER_BLOCK_START, SEEK_SET); 
	fread(super_block_buffer, SUPER_BLOCK_SIZE, 1, pf);
}

void write_super_block(void){
	fseek(pf, SUPER_BLOCK_START, SEEK_SET); 
	fwrite(super_block_buffer, SUPER_BLOCK_SIZE, 1, pf);
	fflush(pf);
}


//读写 block_group_desc 
void read_block_group_desc(void){
	fseek(pf, BLOCK_GROUP_DES_START, SEEK_SET); 
	fread(block_group_desc_buffer, BLOCK_GROUP_DES_SIZE, 1, pf);
}

void write_block_group_desc(void){
	fseek(pf, BLOCK_GROUP_DES_START, SEEK_SET); 
	fwrite(block_group_desc_buffer, BLOCK_GROUP_DES_SIZE, 1, pf);
	fflush(pf);
}


//读写block_buffer 
void read_block_bitmap(void){
	fseek(pf, BLOCK_BITMAP_START, SEEK_SET); 
	fread(block_bitmap_buffer, BLOCK_SIZE, 1, pf);
}

void write_block_bitmap(void){
	fseek(pf, BLOCK_BITMAP_START, SEEK_SET); 
	fwrite(block_bitmap_buffer, BLOCK_SIZE, 1, pf);
	fflush(pf);
}



//读写inode_buffer 
void read_inode_bitmap(void){
	fseek(pf, INODE_BITMAP_START, SEEK_SET); 
	fread(inode_bitmap_buffer, BLOCK_SIZE, 1, pf);
}

void write_inode_bitmap(void){
	fseek(pf, INODE_BITMAP_START, SEEK_SET); 
	fwrite(inode_bitmap_buffer, BLOCK_SIZE, 1, pf);
	fflush(pf);
}


//从inode table中读写指定位置的inode 
void read_inode(int inode_num){
	fseek(pf, INODE_TABLE_START+(inode_num-1)*INODE_SIZE, SEEK_SET);
	fread(inode_buffer, INODE_SIZE, 1, pf); 
}

void write_inode(int inode_num){
	fseek(pf, INODE_TABLE_START+(inode_num-1)*INODE_SIZE, SEEK_SET);
	fwrite(inode_buffer, INODE_SIZE, 1, pf); 
	fflush(pf);
}

//从block data中读写数据 
void read_block(int block_num){
	fseek(pf, DATA_BLOCK_START+block_num*BLOCK_SIZE, SEEK_SET);
	fread(block_buffer, BLOCK_SIZE, 1, pf); 
}

void write_block(int block_num){
	fseek(pf, DATA_BLOCK_START+block_num*BLOCK_SIZE, SEEK_SET);
	fwrite(block_buffer, BLOCK_SIZE, 1, pf); 
	fflush(pf);
}

//从data block中读写dir信息 
void read_dir(int block_num){
	fseek(pf, DATA_BLOCK_START+block_num*BLOCK_SIZE, SEEK_SET);
	fread(dir, BLOCK_SIZE, 1, pf); 
} 

void write_dir(int block_num){
	fseek(pf, DATA_BLOCK_START+block_num*BLOCK_SIZE, SEEK_SET);
	fwrite(dir, BLOCK_SIZE, 1, pf);
	fflush(pf);
} 


//返回当前时间 
void current_time(void){
	time_t t = time(0); //获取当前系统的时间 
	strftime(time_now, sizeof(time_now), "%Y-%m-%d %H:%M:%S", localtime(&t));
}

//返回一个空闲的block 
//block_bitmap_buffer[]   block map缓冲区 
unsigned int get_free_block(void){

	unsigned int temp_block_bit = last_block_bit;  //记录上一次分配的block号 
	unsigned int temp = temp_block_bit/8;  //8bit = 1char 
	char flag = 0;
	unsigned char con = 128;
	read_block_group_desc();
	
	//如果没有剩余block直接跳出 
	if(block_group_desc_buffer->bgd_free_block_count==0){
		printf("there is no free block.\n");
		return -1; 
	}
	
	//将block bitmap读出到  block_bitmap_buffer[] 
	//查看是否哪个字节有bit位空缺，因为一个文件一共有8个索引块 
	read_block_bitmap();
	while(block_bitmap_buffer[temp] == 255){
		if(temp == 1023)
			temp=0;
		else
			temp++;
	}
	
	//用位与的方法，得到空闲的bit位 
	while(block_bitmap_buffer[temp]&con){
		flag++;
		con /= 2;
	}

	//得到bit位之后，直接修改整个char值 
	block_bitmap_buffer[temp] = block_bitmap_buffer[temp]+con; 
	last_block_bit = temp*8+flag;
	write_block_bitmap(); //更新到block bitmap 
	
	//更新到Super Block 
	read_super_block(); 
	super_block_buffer->sb_free_blocks_count--;
	write_super_block();
	
	 //更新到GDT 
	block_group_desc_buffer->bgd_free_block_count--;
	write_block_group_desc();  
	//printf("%u", last_block_bit);  //检验输出 
	return last_block_bit;
}

//返回一个空闲的inode号 
//inode_bitmap_buffer[] 
unsigned int get_free_inode(void){
	unsigned int temp_inode_bit = last_inode_bit; //最开始等于1 
	unsigned int temp = (temp_inode_bit-1)/8;  //8bit = 1char 

	char flag = 0;
	unsigned char con = 128; //1000 0000b
	
	read_block_group_desc();
	if(block_group_desc_buffer->bgd_free_inode_count==0){
		printf("there is no free inode.\n");
		return -1; 
	}

	//将inode bitmap读出到  inode_bitmap_buffer[] 
	read_inode_bitmap();
	while(inode_bitmap_buffer[temp] == 255){
		if(temp == 1023)
			temp=0;
		else
			temp++;
	}
	
//	printf("temp_inode_bit: %d\n", temp_inode_bit);
//	printf("temp: %d\n", temp);
//	printf("inode_bitmap_buffer[temp]: %d\n", inode_bitmap_buffer[temp]);
//	printf("con: %d\n", con);
	//把while写成了if,醉了 
	while(inode_bitmap_buffer[temp]&con){
		flag++;
		con /= 2;
		//printf("con: %d\n", con);
	}

	inode_bitmap_buffer[temp] = inode_bitmap_buffer[temp]+con; 
	//printf("inode_bitmap_buffer[temp]: %d", inode_bitmap_buffer[temp]);
	last_inode_bit = temp*8+flag+1;
	write_inode_bitmap(); //更新到inode bitmap 
	
	//更新到Super Block 
	read_super_block(); 
	super_block_buffer->sb_free_inodes_count--;
	write_super_block();

	block_group_desc_buffer->bgd_free_inode_count--;
	write_block_group_desc(); //更新到块组描述 
	
	//printf("%d\n", flag);
	//printf("%d\n", con);
	//printf("%u\n", last_inode_bit); 
	return last_inode_bit;
} 

//删除 block 
void remove_block(int remove_block_bit){
	unsigned temp = remove_block_bit/8;
	
	read_block_bitmap();
	//找到bit位所属的字节然后和特定的数字相与就可以将它置0 
	switch(remove_block_bit%8){
		case 0: block_bitmap_buffer[temp]&127;break; //0111 1111
		case 1: block_bitmap_buffer[temp]&191;break; //1011 1111
		case 2: block_bitmap_buffer[temp]&223;break; //1101 1111
		case 3: block_bitmap_buffer[temp]&239;break; //1110 1111
		case 4: block_bitmap_buffer[temp]&247;break; //1111 0111
		case 5: block_bitmap_buffer[temp]&251;break; //1111 1011
		case 6: block_bitmap_buffer[temp]&253;break; //1111 1101
		case 7: block_bitmap_buffer[temp]&254;break; //1111 1110
	}
	
	//更新Block Bitmap 
	write_block_bitmap();
	
	//更新GDT 
	read_block_group_desc();
	block_group_desc_buffer->bgd_free_block_count++;
	write_block_group_desc();
	
	//更新Super Block 
	read_super_block(); 
	super_block_buffer->sb_free_blocks_count--;
	write_super_block();
}

//删除inode 
void remove_inode(int remove_inode_bit){
	unsigned temp = (remove_inode_bit-1)/8;
	
	read_inode_bitmap();
	switch((remove_inode_bit-1)%8){
		case 0: inode_bitmap_buffer[temp]&127;break; //0111 1111
		case 1: inode_bitmap_buffer[temp]&191;break; //1011 1111
		case 2: inode_bitmap_buffer[temp]&223;break; //1101 1111
		case 3: inode_bitmap_buffer[temp]&239;break; //1110 1111
		case 4: inode_bitmap_buffer[temp]&247;break; //1111 0111
		case 5: inode_bitmap_buffer[temp]&251;break; //1111 1011
		case 6: inode_bitmap_buffer[temp]&253;break; //1111 1101
		case 7: inode_bitmap_buffer[temp]&254;break; //1111 1110
	}
	
	//更新Inode Bitmap 
	write_inode_bitmap();
	
	//更新GDT 
	read_block_group_desc();
	block_group_desc_buffer->bgd_free_inode_count++;
	write_block_group_desc();
	
	//更新Super Block 
	read_super_block(); 
	super_block_buffer->sb_free_inodes_count++;
	write_super_block();
}

//显示当前目录下文件信息 
void ls(){
	printf("%-15s %-10s %-5s %-13s %-22s %-10s %-10s\n", "name", "type", "user", "group", "create_time", "mode", "size");
	read_inode(current_dir_inode); //读取当前目录的节点信息 
//	printf("current_dir_inode: %d\n", current_dir_inode);
//	printf("i_size: %u\n", inode_buffer->i_size);

	unsigned int i=0, k=0, temp=0, j=0, n=0;
	char mode[9]="";
//	for(i=0; i< 8; i++){
//		printf("%d\n", inode_buffer->i_block[i]);
//	}
	i=0;
	while(i < inode_buffer->i_blocks_count){
		read_dir(inode_buffer->i_block[i]);
		k=0;
		while(k < 64){
			if(dir[k].inode){ //判断是否存在 
				printf("%-15s", dir[k].name); 
				if(dir[k].file_type == 1){
					printf("%-12s", "<File>");
				}else if(dir[k].file_type == 2){
					printf("%-12s", "<Dir>");
				}
				printf("%-5s %-10s", "root", "root");
				read_inode(dir[k].inode);
				printf("%-25s", inode_buffer->i_ctime);
				temp = inode_buffer->i_mode;
				//printf("%d", temp);
				j=0, n=100;
				//1 2 4 
				while(1){
					switch(temp/n){
						case 0:strcpy(mode+j, "---");break;
						case 1:strcpy(mode+j, "r--");break; 
						case 2:strcpy(mode+j, "-w-");break; 
						case 3:strcpy(mode+j, "rw-");break; 
						case 4:strcpy(mode+j, "--x");break; 
						case 5:strcpy(mode+j, "r-x");break; 
						case 6:strcpy(mode+j, "-wx");break; 
						case 7:strcpy(mode+j, "rwx");break; 
					}
					if(n==1){
						break;
					}
					temp %= n;
					n /= 10;
					j += 3;
				}
				printf("%-15s", mode);
				printf("%d\n", inode_buffer->i_size);
				
			} 
			k++; 
		}
		i++;
		read_inode(current_dir_inode);
	}
} 

//新建目录的准备工作 
void dir_prepare(unsigned int dir_inode, int dir_len){
	read_inode(dir_inode);
//	printf("dir_inode: %d\n", dir_inode);
//	printf("current_dir_inode: %d\n", current_dir_inode);
//	printf("dir_len: %d\n", dir_len);
//	printf("current_dir_length: %d\n", current_dir_length);
	inode_buffer->i_mode=755;  
	inode_buffer->i_size=32; 
	inode_buffer->i_blocks_count=1; //占用的block的数量 
	inode_buffer->i_block[0] = get_free_block();
	current_time();
	strcpy(inode_buffer->i_ctime, time_now);
	//printf("inode_buffer->i_block[0]:%d\n", inode_buffer->i_block[0]);
	read_dir(inode_buffer->i_block[0]);
	dir[0].inode = dir_inode;
	dir[1].inode = current_dir_inode; //当前目录的inode 
	dir[0].name_len = dir_len;
	dir[1].name_len = current_dir_length;
	dir[0].file_type = dir[1].file_type = 2;
	//这是很关键的一步，不然一直会显示上次遗留的信息 
	int i=0;
	for(i=2; i<64; i++){
		dir[i].inode=0;
	}
	strcpy(dir[0].name, ".");
	strcpy(dir[1].name, "..");
	write_dir(inode_buffer->i_block[0]);
	write_inode(dir_inode);
	//更新GDT
	read_block_group_desc();
	block_group_desc_buffer->bgd_used_dirs_count++;
	write_block_group_desc();
}

//创建新目录 
void mkdir(char temp[9]){
	read_inode(current_dir_inode); //读取当前节点信息
	int i=0, k=0, dir_inode=0, flag=1;
	//printf("current_dir_inode: %d\n", current_dir_inode);

//	for(i=0; i<9; i++){
//		printf("%c", temp[i]); 
//	} 
	
	//判断是否有同名，若有则退出 
	while(i < inode_buffer->i_blocks_count){
		read_dir(inode_buffer->i_block[i]);
		k=0;
		while(k < 64){
			//如果节点存在并且名字相同 
			//这里是不等于，需要注意一下 
			if(dir[k].inode && !strcmp(dir[k].name, temp)){
				printf("filename has already existed! \n");
				return;
			}
			k++; 
		}
		i++;
	} 
	//k可以用来表示下一个可用的目录点inode， i表示下一个可用的block[i] 
//	printf("k: %d\n", k); 
//	printf("i: %d\n", i); 
	
	if(inode_buffer->i_size == 1024*8){
		printf("Directory has no room to be alloced!\n");
		return; 
	} 
	
	flag=1;
	read_inode(current_dir_inode);
	//判断一个目录项中是否还有空闲 
	if(inode_buffer->i_size != (inode_buffer->i_blocks_count)*1024){
		i=0;
		//printf("i: %d\n", i);
		//把空闲block找到 
		while(flag && i < inode_buffer->i_blocks_count){
			read_dir(inode_buffer->i_block[i]);
//			printf("inode_buffer->i_block[i]: %d\n", inode_buffer->i_block[i]);
//			for(k=0; k<64; k++){
//				printf("%d", dir[k].inode);
//			}
			k=0;
			while(k < 64){
				if(dir[k].inode == 0){
					flag=0; //如果有空间直接跳出两重循环 
					break;
				}
				//printf("\nk: %d\n", k);
				k++; 
			}
			i++;
		} 
		
		//printf("k: %d\n", k);
		dir_inode = get_free_inode();
		dir[k].inode = dir_inode;
		dir[k].name_len = strlen(temp);
		dir[k].file_type=2;
		strcpy(dir[k].name, temp);

		//printf("inode: %d\n", dir_inode);
		write_dir(inode_buffer->i_block[i-1]);
		
	}else{
		inode_buffer->i_block[inode_buffer->i_blocks_count] = get_free_block();
		inode_buffer->i_blocks_count++;
		read_dir(inode_buffer->i_block[inode_buffer->i_blocks_count-1]);
		dir_inode = get_free_inode();
		dir[0].inode = dir_inode;
		dir[0].name_len = strlen(temp);
		dir[0].file_type=2;
		strcpy(dir[0].name, temp);
		for(k=1; k<64; k++){
			dir[k].inode=0;
		}
		
		//printf("%d", dir_inode);
		write_dir(inode_buffer->i_block[i-1]);	
	}
	

	inode_buffer->i_size += 16;
	write_inode(current_dir_inode);
	//为每个新加目录添加额外信息 
//	printf("dir_inode: %d\n", dir_inode);
	dir_prepare(dir_inode, strlen(temp));
}

//切换目录
void cd(char temp[9]){
	int i=0, k=0;
//	printf("%u", current_dir_inode); 
//	fseek(pf, INODE_TABLE_START, SEEK_SET);
//	fread(inode_table_buffer, INODE_TABLE_SIZE, 1, pf); 
//	for(i=0; i < INODE_TABLE_SIZE; i++){
//		printf("%d", inode_table_buffer[i]);
//	} 
//	temp[strlen(temp)] = '\0';
//	printf("%d", strlen(temp));
	if(!strcmp(temp, ".")){
		//printf("test");//啥都不做 
		return;
	}else if(!strcmp(temp, "..")){
		read_inode(current_dir_inode); //读取当前目录的节点信息 
		
		while(i < inode_buffer->i_blocks_count){
			read_dir(inode_buffer->i_block[i]); //读取每一个block指针所对应的dir 
			k=0;
			while(k < 64){
				if(!strcmp(dir[k].name, "..")){
					current_dir_inode = dir[k].inode;
					path_name[strlen(path_name)-dir[k-1].name_len-1] = '\0';
					current_dir_length = dir[k].name_len;
					//printf("%d", current_dir_inode);
					//  / sking/123/123/ 
					return;
				}
				k++;
				
			}
			i++;
		}
	}

	
	read_inode(current_dir_inode); //读取当前目录的节点信息 
	while(i < inode_buffer->i_blocks_count){
		read_dir(inode_buffer->i_block[i]); //读取每一个block指针所对应的dir 
		k=0;
		while(k < 64){
			if(!strcmp(dir[k].name, temp) && dir[k].inode && dir[k].file_type==2){
				current_dir_inode = dir[k].inode;
				current_dir_length = dir[k].name_len;
				//printf("%d", current_dir_inode); 
				
				strcat(path_name, temp);
				strcat(path_name, "/");
				return;
			}
			k++;
			
		}
		i++;
	}
	printf("Can't find this Directory! \n");


}

//格式化磁盘 
void format(){
	char answer = 'Y';
	while(1){

		scanf("%c", &answer);
		if(toupper(answer) == 'Y'){
			printf("Ready to format......\n"); 
			create_fileSystem();
			pf = fopen("fileSystem.dat", "r+b");
			last_inode_bit=1;  
			last_block_bit=0; 
			read_super_block();
			read_block_group_desc();
		
			//目录名 
			strcpy(current_path, "[root@sking ");
			strcpy(path_name, "/");
			strcpy(path_last, "]# ");
			
			current_dir_inode=1; 
			current_dir_length=1;
			return;
		}else if(toupper(answer) == 'N'){
			return;
		}else{
			fflush(stdin);
		}
		printf("Are you sure you want to format the disk ?[y / n]: ");

	}
	
	
}

//删除空目录 
void rmdir(char temp[9]){
	int i=0, k=0, flag=0;
	
	if(!strcmp(temp, "..") || !strcmp(temp, ".")) {
		printf("This directory is not allowed to be deleted!\n");
		return;
	}
	
	read_inode(current_dir_inode);

	while(!flag && i < inode_buffer->i_blocks_count){
		read_dir(inode_buffer->i_block[i]);
		k=0;
		while(k < 64){
			if(dir[k].inode && !strcmp(dir[k].name, temp) && dir[k].file_type==2){
				flag = 1; //找到此目录了 
				break;
			}
			k++; 
		}
		i++;
	} 
	
	if(!flag){
		printf("Please enter the correct directory name!\n");
		return;
	}else{
		//加载需要删除的节点信息 
		read_inode(dir[k].inode);
		//判断是否为空目录 
		//只有 .. 和 . 目录 
//			unsigned int i_mode;  //文件类型和权限 
//			unsigned int i_size;  //文件或者目录大小 
//			unsigned int i_blocks_count; //占用的block的数量 
//			unsigned short i_block[8];  //内容指向, 存放block号 
		if(inode_buffer->i_size == 32){
			inode_buffer->i_mode = 0;
			inode_buffer->i_size = 0;
			inode_buffer->i_blocks_count = 0;
			inode_buffer->i_size = 0;
			//将子目录下的两个文件删除 
			read_dir(inode_buffer->i_block[0]);
			dir[0].inode=0;
			dir[1].inode=0;
			write_dir(inode_buffer->i_block[0]);
			//删除block号 
			remove_block(inode_buffer->i_block[0]);
			//删除本目录下的这个目录 
			read_inode(current_dir_inode); 
			read_dir(inode_buffer->i_block[i-1]);
			dir[k].inode=0;
			write_dir(inode_buffer->i_block[i-1]);
			inode_buffer->i_size -= 16;
			
			//把哪些整个block都是空的这种块去掉
			 i=1;
			 flag=0;
			 while(flag<64 && i<inode_buffer->i_blocks_count){
			 	k=0;
			 	read_dir(inode_buffer->i_block[i]);
			 	while(k<64){
			 		if(!dir[k].inode){
			 			flag++;
			 		}
			 		k++;
			 	}
			 	if(flag==64){
			 		remove_block(inode_buffer->i_block[i]);
			 		inode_buffer->i_blocks_count--;
			 		while(i<inode_buffer->i_blocks_count){
			 			inode_buffer->i_block[i] = inode_buffer->i_block[i+1];
						i++;
			 		}
			 	}
			 	
			 }
			 write_inode(current_dir_inode);
			 
			
			
			
		}else{
			printf("Can't delete, directory is not null! ");
			return;
		}
	}

}

//退出登录 
void logout(void){
	char answer = 'Y';
	while(1){
		scanf("%c", &answer);
		if(toupper(answer) == 'Y'){
			printf("\nbye bye ~~\n");
			exit(0);
		}else if(toupper(answer) == 'N'){
			return;
		}else{
			fflush(stdin);
		}
		printf("Are you sure you want to quit ?[y / n]: ");
	}
}

//显示帮助命令 
void help(){
    printf("      *****************************************************************************************\n");
    printf("      *                                    command help                                       *\n");
    printf("      *                                                                                       *\n");
    printf("      * 01.command help  : help                  09.format disk       : format                *\n");
    printf("      * 02.create dir    : mkdir + dir_name      10.delete empty dir  : rmdir + dir_name      *\n");
    printf("      * 03.list dir      : ls                    11.chang dir         : cd + dir_name         *\n");
    printf("      * 04.edit file     : vi + file_name        12.read file         : cat + file_name       *\n");
    printf("      * 05.remove file   : rm + file_name        13.ping IP/Address   : ping + ip/host        *\n");
    printf("      * 06.list local IP : ifconfig              14.traceroute        : traceroute + ip/host  *\n");   
    printf("      * 07.show data now : data                  15.modify mode       : chmod + mode          *\n");
    printf("      * 08.logout        : logout                16.display disk info : dumpe2fs              *\n");
    printf("      *                                                                                       *\n");
    printf("      *****************************************************************************************\n\n");
} 

//显示磁盘信息 
void dumpe2fs(){
	read_super_block();
	printf("volume name            : %s\n", super_block_buffer->sb_volume_name);
	printf("inodes counts          : %d\n", super_block_buffer->sb_inodes_count);
	printf("blocks counts          : %d\n", super_block_buffer->sb_blocks_count);
	printf("free inodes counts     : %d\n", super_block_buffer->sb_free_inodes_count);
	printf("free blocks counts     : %d\n", super_block_buffer->sb_free_blocks_count);
	printf("inode size             : %d(kb)\n", super_block_buffer->sb_inode_size);
	printf("block size             : %d(kb)\n", super_block_buffer->sb_block_size);
	printf("create time            : %s\n\n", super_block_buffer->sb_wtime);

}

void vi(char temp[9]){ 
	read_inode(current_dir_inode); //读取当前节点信息
	int i=0, k=0, flag=1, m=0, file_inode=0;
	int local=0, file_length=0, file_block_count=0;
	
	//寻找文件是否存在，如果是目录，则不能进行编辑
	//如果是文件，则存在进行编辑，不存在进行创建 
	while(flag && i < inode_buffer->i_blocks_count){
		read_dir(inode_buffer->i_block[i]);
		k=0;
		while(k < 64){
			if(dir[k].inode && !strcmp(dir[k].name, temp)){
				if(dir[k].file_type == 2){
					printf("Directory can't be edit! \n");
					return;
				}else{
					flag = 0; //存在文件 
					break;
				}
			}
			k++; 
		}
		i++;
	} 
	
	//如果文件不存在, 将创建新文件 
	if(flag){
		printf("Can't find the file name, will create a new file!\n");
		printf("Please input '\\q' to quit! \n\n");
		read_inode(current_dir_inode); //读取当前节点信息
		i=0, k=0, file_inode=0, flag=1;
		
		//是否还有block 
		if(inode_buffer->i_size == 1024*8){
			printf("Directory has no room to be alloced!\n");
			return; 
		} 

		m=1; //用来终止循环 
		//判断一个目录项中是否还有空闲 
		if(inode_buffer->i_size != (inode_buffer->i_blocks_count)*1024){
			i=0;
			while(m && i < inode_buffer->i_blocks_count){
				read_dir(inode_buffer->i_block[i]);
				k=0;
				while(k < 64){
					if(dir[k].inode == 0){
						m=0; //如果有空间直接跳出两重循环 
						break;
					}
					//printf("\nk: %d\n", k);
					k++; 
				}
				i++;
			} 
			
			//printf("k: %d\n", k);
			file_inode = get_free_inode();
			dir[k].inode = file_inode;
			dir[k].name_len = strlen(temp);
			dir[k].file_type=1;
			strcpy(dir[k].name, temp);
	
			//printf("inode: %d\n", file_inode);
			write_dir(inode_buffer->i_block[i-1]);
			
		}else{
			inode_buffer->i_block[inode_buffer->i_blocks_count] = get_free_block();
			inode_buffer->i_blocks_count++;
			read_dir(inode_buffer->i_block[inode_buffer->i_blocks_count-1]);
			file_inode = get_free_inode();
			dir[0].inode = file_inode;
			dir[0].name_len = strlen(temp);
			dir[0].file_type=1;
			strcpy(dir[0].name, temp);
			for(k=1; k<64; k++){
				dir[k].inode=0;
			}
		
			//printf("%d", file_inode);
			write_dir(inode_buffer->i_block[i-1]);	
		}
		
		inode_buffer->i_size += 16;
		write_inode(current_dir_inode);//写入当前目录 
		
		//写入文件初始化信息 
		read_inode(file_inode);
		inode_buffer->i_mode = 777;
		inode_buffer->i_size = 0;
		inode_buffer->i_blocks_count = 0;
		current_time();
		strcpy(inode_buffer->i_ctime, time_now);

		
		//开始写入文件缓存filebuffer；
		while(1){
			filebuffer[local] = getchar();
			if(filebuffer[local] == 'q' && filebuffer[local-1]=='\\'){
				filebuffer[local-1] = '\0';
				break;
			}

			if(local>=8191){
				printf("Sorry,the max size of a file is 4KB!\n"); 
				break;
			}
			
			local++;
		}
		
//		for(i=0; i<local-1; i++){
//			printf("%c", filebuffer[i]);
//		}
//		printf("\n");

		file_length = strlen(filebuffer); //文件内容的长度
		file_block_count = file_length/1024;
		if(file_length%1024){
			file_block_count++;
		} 
//		printf("file_block_count: %d\n", file_block_count);
		//直接覆盖写入
		for(i=0; i<file_block_count; i++){
			inode_buffer->i_blocks_count++;
			inode_buffer->i_block[i] = get_free_block();
			read_block(inode_buffer->i_block[i]); //将数据读入缓冲区
			if(i==file_block_count-1)
				memcpy(block_buffer, filebuffer+i*BLOCK_SIZE, file_length-i*BLOCK_SIZE);
			else
				memcpy(block_buffer, filebuffer+i*BLOCK_SIZE, BLOCK_SIZE);
			write_block(inode_buffer->i_block[i]);
		}
		
		inode_buffer->i_size=file_length;
//		printf("file_inode: %d", file_inode);
//		printf("inode_buffer->i_blocks_count: %d\n", inode_buffer->i_blocks_count);
		write_inode(file_inode);
		printf("\nSave as ");
		for(i=0; i<strlen(temp); i++){
			printf("%c", temp[i]);
		} 
		printf("!\n");
	//文件存在时 
	}else{
		fflush(stdin);
		printf("The file is exist!!\n");
		printf("Please input '\\q' to quit! \n\n");
		read_inode(dir[k].inode); //读取当前节点信息,存入inode_buffer
//		printf("dir[k].inode: %d\n", dir[k].inode);
//		printf("inode_buffer->i_blocks_count: %d\n", inode_buffer->i_blocks_count);

		for(i=0; i<inode_buffer->i_blocks_count; i++){
			read_block(inode_buffer->i_block[i]); //存入block_buffer
			if(i == inode_buffer->i_blocks_count-1){
				memcpy(filebuffer+i*BLOCK_SIZE, block_buffer, inode_buffer->i_size-i*BLOCK_SIZE);	
			}else{
				memcpy(filebuffer+i*BLOCK_SIZE, block_buffer, i*BLOCK_SIZE);	
			}
			remove_block(inode_buffer->i_block[i]);	
		} 
		
		for(i=0; i<inode_buffer->i_size; i++){
			printf("%c", filebuffer[i]);
		}
		
		inode_buffer->i_blocks_count=0;
		local = inode_buffer->i_size;
		while(1){
			if(local>=8191){
				printf("Sorry,the max size of a file is 4KB!\n"); 
				break;
			}
			
			filebuffer[local] = getchar();
			if(filebuffer[local] == 'q' && filebuffer[local-1]=='\\'){
				filebuffer[local-1] = '\0';
				break;
			}
			
			local++;
		}
		

		file_length = strlen(filebuffer); //文件内容的长度
		file_block_count = file_length/1024;
		if(file_length%1024){
			file_block_count++;
		}

		for(i=0; i<file_block_count; i++){
			inode_buffer->i_blocks_count++;
			inode_buffer->i_block[i] = get_free_block();
			read_block(inode_buffer->i_block[i]); //将数据读入缓冲区
			if(i==file_block_count-1)
				memcpy(block_buffer, filebuffer+i*BLOCK_SIZE, file_length-i*BLOCK_SIZE);
			else
				memcpy(block_buffer, filebuffer+i*BLOCK_SIZE, BLOCK_SIZE);
			write_block(inode_buffer->i_block[i]);
		}
		
		inode_buffer->i_size=file_length;
		//更新时间 
		current_time();
		strcpy(inode_buffer->i_ctime, time_now);
//		printf("file_inode: %d", file_inode);
//		printf("inode_buffer->i_blocks_count: %d\n", inode_buffer->i_blocks_count);
		write_inode(dir[k].inode);
		
		printf("\nSave!\n");
	} 

} 

void cat(char temp[9]){
	read_inode(current_dir_inode); //读取当前节点信息
	int i=0, k=0, flag=1;
	
	//寻找文件是否存在，如果是目录，则不能进行查看 
	while(flag && i < inode_buffer->i_blocks_count){
		read_dir(inode_buffer->i_block[i]);
		k=0;
		while(k < 64){
			if(dir[k].inode && !strcmp(dir[k].name, temp)){
				if(dir[k].file_type == 2){
					printf("That is a directory! \n");
					return;
				}else{
					flag = 0; //存在文件 
					break;
				}
			}
			k++; 
		}
		i++;
	} 
	if(!flag){
		read_inode(dir[k].inode); //读取当前节点信息,存入inode_buffer
		for(i=0; i<inode_buffer->i_blocks_count; i++){
			read_block(inode_buffer->i_block[i]); //存入block_buffer
			if(i == inode_buffer->i_blocks_count-1){
				memcpy(filebuffer+i*BLOCK_SIZE, block_buffer, inode_buffer->i_size-i*BLOCK_SIZE);	
			}else{
				memcpy(filebuffer+i*BLOCK_SIZE, block_buffer, i*BLOCK_SIZE);	
			}
			remove_block(inode_buffer->i_block[i]);	
		} 
		
		for(i=0; i<inode_buffer->i_size; i++){
			printf("%c", filebuffer[i]);
		}
		printf("\n");
	}else{
		printf("Can't find the filename!\n");
		return;
	}

} 

void rm(char temp[9]){
	read_inode(current_dir_inode); //读取当前节点信息
	int i=0, k=0, flag=1;
	int m=0, n=0;
	
	//判断文件是否存在，若文件存在才可删除 
	while(flag && i < inode_buffer->i_blocks_count){
		read_dir(inode_buffer->i_block[i]);
		k=0;
		while(k < 64){
			if(dir[k].inode && !strcmp(dir[k].name, temp)){
				if(dir[k].file_type == 2){
					printf("'rm' only delete file! \n");
					return;
				}else{
					flag = 0; //存在文件 
					break;
				}
			}
			k++; 
		}
		i++;
	} 
	
	//flag = 1 文件不存在 
	if(flag){
		printf("Can't find this filename!\n"); 
		return;
	}else{
		read_dir(inode_buffer->i_block[i]);
		read_inode(dir[k].inode); //加载inode
		//把当前文件的inode信息删除 
		for(i=0; i<inode_buffer->i_blocks_count; i++){
			remove_block(inode_buffer->i_block[i]);
		}
		inode_buffer->i_mode = 0;
		inode_buffer->i_size = 0;
		inode_buffer->i_blocks_count=0;
		
		//删除本目录下此文件的信息
		read_inode(current_dir_inode);
		inode_buffer->i_size -= 16;
		read_dir(inode_buffer->i_block[i]);
		dir[k].inode = 0;
		write_dir(inode_buffer->i_block[i]);
		
		m=1;
		while(m < inode_buffer->i_blocks_count){
			read_dir(inode_buffer->i_block[m]);
			flag=n=0;
			while(n<64){
				if(!dir[n].inode){
					flag++;
				}
				n++;
			}
			
			if(flag == 64){
				remove_block(inode_buffer->i_block[m]);
				inode_buffer->i_blocks_count--;
				while(m < inode_buffer->i_blocks_count){
					inode_buffer->i_block[m] = inode_buffer->i_block[m+1];
					m++;
				}
			}
			m++;
			
		}
		write_inode(current_dir_inode);
		printf("The file has been deleted!\n");
		
	}
} 

void ifconfig(){
     char host_name[256]; 
     int WSA_return, i;
     WSADATA WSAData;
     HOSTENT *host_entry; 
     WORD wVersionRequested;

     wVersionRequested = MAKEWORD(2, 0);
     WSA_return = WSAStartup(wVersionRequested, &WSAData); 

     if (WSA_return == 0){
         gethostname(host_name, sizeof(host_name));
         host_entry = gethostbyname(host_name);
		 printf("\t%-15s\t  %s\n", "IP", "hostname");
         for(i = 0; host_entry != NULL && host_entry->h_addr_list[i] != NULL; ++i){
             const char *pszAddr = inet_ntoa (*(struct in_addr *)host_entry->h_addr_list[i]);
             printf("   %-24s%s\n", pszAddr, host_name);
         }
     }
     else{
         printf("Please check network!\n");
     }
     WSACleanup();
}

void ping(char ip[128]){
	char address[150];
	strcpy(address, "ping -n 4 ");
	strcat(address, ip);
	char buffer[128];
	
//	for(int i=0; i<128; i++){
//		printf("%c", ip[i]);
//	}
	FILE *pipe = _popen(address, "r");
	if(!pipe){
		printf("cmd failed\n");
	}
	
	while(!feof(pipe)){
		fgets(buffer, 128, pipe);
		printf("%s", buffer);
	} 
	_pclose(pipe);
	printf("\n");
} 

//对文件重命名 
void reName(char oldname[128], char newname[128]){ 
	read_inode(current_dir_inode); //读取当前节点信息
	int i=0, k=0, flag=1;
	int m=0, n=0;
	//判断文件是否存在，若文件存在才可修改 
	while(flag && i < inode_buffer->i_blocks_count){
		read_dir(inode_buffer->i_block[i]);
		k=0;
		while(k < 64){
			if(dir[k].inode && !strcmp(dir[k].name, oldname)){
				if(dir[k].file_type == 2){
					printf("The directory name can't be modified!\n");
					return;
				}else{
					flag = 0; //存在
					break;
				}
			}
			k++; 
		}
		i++;
	}
	
	//文件不存在flag=1 
	if(flag){
		printf("Can't find the filename!\n");
		return; 
	}else{
		flag=1;
		//判断newname是否重名 
		while(flag && m < inode_buffer->i_blocks_count){
			read_dir(inode_buffer->i_block[m]);
			n=0;
			while(n < 64){
				if(dir[n].inode && !strcmp(dir[n].name, newname)){
						flag = 0; //存在
						break;
				}
				n++; 
			}
			m++;
		}
		if(!flag){
			printf("The filename \"%s\" is existed\n", newname);
			return;
		}else{
			read_inode(current_dir_inode); //读取当前节点信息
			read_dir(inode_buffer->i_block[i]);
			strcpy(dir[k].name, newname);
			dir[k].name_len=strlen(newname);
			write_dir(inode_buffer->i_block[i]);
		}
	}
} 
//显示时间 
void data(){
	char data_time[128];
	time_t t = time(0); //获取当前系统的时间 
	strftime(data_time, sizeof(data_time), "%Y-%m-%d %H:%M:%S %A", localtime(&t));
	printf("%s\n", data_time);
}

//修改权限 
void chmod(char temp[9], int mode){
	if(mode>777 || mode < 0){
		printf("mode <000-777>!\n");
		return;
	}
	read_inode(current_dir_inode); //读取当前节点信息
	int i=0, k=0, flag=1;

	//判断文件是否存在，若文件存在才可修改 
	flag = search_file(temp, &i, &k);
	
	//flag = 1 文件不存在 
	if(flag){
		printf("Can't find this filename!\n"); 
		return;
	}else{
		read_dir(inode_buffer->i_block[i]);
		read_inode(dir[k].inode);
		inode_buffer->i_mode = mode;
		current_time();
		strcpy(inode_buffer->i_ctime, time_now);
		write_inode(dir[k].inode);
		printf("The file's mode has been modify!\n");
		
	}
}

//返回1为没找到，0为找到了 
int search_file(char temp[9], int *i, int *k){
	read_inode(current_dir_inode); //读取当前节点信息
	int flag=1, i_temp=0, k_temp=0; //设置临时变量 
	
	while(flag && i_temp < inode_buffer->i_blocks_count){
		read_dir(inode_buffer->i_block[i_temp]);
		k_temp=0;
		while(k_temp < 64){
			if(dir[k_temp].inode && !strcmp(dir[k_temp].name, temp)){
					flag = 0; //存在
					*i=i_temp;
					*k=k_temp;
					return 0;
			}
			k_temp++; 
		}
		i_temp++;
	}
	return 1;
} 
