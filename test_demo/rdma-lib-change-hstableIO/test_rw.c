#include<stdio.h>
#include<unistd.h>
#include<error.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<string.h>
#include<fcntl.h>

int main(int args,char *argv[])
{
	char readbuf[128]={0};           //缓冲区
	char *str="hello world!";
	int fd;
	int n_read;
	int n_write;
	
	if((fd=open("/home/klx/rpc/new_rdma-rpc/rdma-rpc/test_demo/rdma-lib-change-hstableIO/test",O_RDWR|O_CREAT))==-1)  //文件的打开
	{
		perror("open");
		exit(-1);
	}
	if((n_read=read(fd,readbuf,sizeof(readbuf)))==-1)   //文件的读取
	{
		perror("read");
		exit(-1);
	}
	/*
	if((n_write=write(fd,readbuf,strlen(readbuf)))==-1)//文件的写入

	{
		perror("write");
		exit(-1);
	}
	*/
	printf("返回的文件描述符 fd=%d\n",fd);
	printf("读取的大小       n_read=%d\n",n_read);
	printf("写入的大小       n_write=%d\n",n_write);
	close(fd);	//文件的关闭
	return 0;
}
