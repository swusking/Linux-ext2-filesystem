## 用C语言写的一个仿Linux ext2的文件系统，我的文档还是比较系统化
### 话不多说，文档附上（#课程设计，请勿吐槽）
+  doucument：流程图
+  ext2：程序代码  

### 目录树

1.  概述  
  1.1 什么是文件系统   
  1.2 常见的文件系统分类  
  1.3 EXT2的简单介绍    
  1.4 EXT2和FAT的区别  
  1.5 EXT2的结构  
  1.6 EXT2的目录和文件    
2. 数据结构  
  2.1 用户和组  
  2.2 inode和block的大小  
  2.3 Super Block  
  2.4 Group Description Table  
  2.5 Block Bitmap  
  2.6 Inode Bitmap  
  2.7 Inode Table  
  2.8 Data Block  
  2.9 目录和文件  
  2.10 宏定义  
  2.11 全局变量  
3. 函数  
  3.1	系统函数   
  &emsp;  3.1.1 read_super_block()    
  &emsp;  3.1.2 write_super_block()  
  &emsp;  3.1.3 read_block_group_desc()  
  &emsp;  3.1.4 write_block_group_desc()  
  &emsp;  3.1.5 read_block_bitmap()  
  &emsp;  3.1.6 write_block_bitmap()  
  &emsp;  3.1.7 read_inode_bitmap()  
  &emsp;  3.1.8 write_inode_bitmap()  
  &emsp;  3.1.9 read_inode()  
  &emsp;  3.1.10 write_inode()  
  &emsp;  3.1.11 read_block()  
  &emsp;  3.1.12 write_block()
  &emsp;  3.1.13 read_dir()  
  &emsp;  3.1.14 write_dir()  
  &emsp;  3.1.15 get_free_block()  
  &emsp;  3.1.16 get_free_inode()  
  &emsp;  3.1.17 remove_block()  
  &emsp;  3.1.18 remove_inode()  
  &emsp;  3.1.19 current_time()  
  &emsp;  3.1.20 create_fileSystem()  
  3.2 功能函数  
  &emsp;  3.2.1 ls()    
  &emsp;  3.2.2 mkdir()   
  &emsp;  3.2.3	dir_prepare()  
  &emsp;  3.2.4	cd()  
  &emsp;  3.2.5	format()  
  &emsp;  3.2.6	void rmdir(char temp[9])  
  &emsp;  3.2.7	logout()  
  &emsp;  3.2.8	help()  
  &emsp;  3.2.9	dumpe2fs()  
  &emsp;  3.2.10 vi()  
  &emsp;  3.2.11 cat()  
  &emsp;  3.2.12 rm()  
  &emsp;  3.2.13 ifconfig()  
  &emsp;  3.2.14 ping()  
  &emsp;  3.2.15 reName()  
  &emsp;  3.2.16 data()  
  &emsp;  3.2.17 chmod()  
  &emsp;  3.2.18 search_file()  
  3.3	主函数main  
  3.4 EXT2相关命令对应的功能函数  
