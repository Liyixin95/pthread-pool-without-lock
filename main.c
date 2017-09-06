#include "pool.h"
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>

extern thread_pool *pool;
int Flag = 0;

struct patharg
{
	char Src[512];
	char Dest[512];
};

static void *CopyFile(void *arg)
{
	int buf[1024];
	const struct patharg *path = (struct patharg *)arg;
	#ifdef     DEBUG
		printf("id = %lu ",pthread_self());
		printf("Src = %s,Dest = %s\n",path -> Src,path -> Dest);
	#endif
	FILE *fd_read,*fd_write;
	int ret;

/* 	if(access(path -> Dest,F_OK) == 0 && Flag == 0)
	{
		char p;
		printf("This file already exists,sure to overwrite? y/n and a for all\n ");
		while(1)
		{
			scanf("%c",&p);
			if(p == 'n')
			{
				return NULL;
			}
			else if(p == 'y')
			{
				break;
			}
			else if (p == 'a')
			{
				Flag = 1;
			}
			if(p != 'y' && p != 'n' && p != 'a')
			{
				printf("please enter again\n");
			}
		}
	} */
	
	fd_read = fopen(path -> Src,"r");
	if(fd_read == NULL)
	{
		printf("Open fail\n");
		return NULL;
	}
	
	fd_write = fopen(path -> Dest,"w+");
	do{
		ret = fread(buf,1,1024,fd_read);
		fwrite(buf,1,1024,fd_write);
		memset(buf,0,1024);
	}while(ret >= 1024);
	
	fclose(fd_read);
	fclose(fd_write);
}
static void CreateDir(const char Src[],const char Dest[])
{
	DIR *dirsrc;
	char strsrc[512]={0};
	char strdest[512]={0};
	struct dirent *entry= NULL;
	dirsrc = opendir(Src);
	if(dirsrc == NULL)
	{
		return;
	}
	if(access(Dest,F_OK) != 0)
	{
		mkdir(Dest,S_IRWXU|S_IRGRP|S_IROTH);
	}
/* 	else if(access(Dest,F_OK) == 0 && Flag == 0)
	{
		char p;
		printf("This folder already exists,sure to merge? y/n and a for all\n ");
		while(1)
		{
			scanf("%c",&p);
			if(p == 'n')
			{
				return;
			}
			else if(p == 'y')
			{
				break;
			}
			else if(p == 'a')
			{
				Flag == 1;
			}
			if(p != 'y' && p != 'n' && p != 'a')
			{
				printf("please enter again\n");
			}
		}
		
	} */
	while((entry = readdir(dirsrc)) != NULL)
	{
		if(strcmp(entry->d_name,".") == 0 || strcmp(entry->d_name,"..") == 0)
			continue;
		
		memset(strsrc,0,strlen(strsrc));
		memset(strdest,0,strlen(strdest));
		sprintf(strsrc,"%s/%s",Src,entry->d_name);
		sprintf(strdest,"%s/%s",Dest,entry->d_name);

		if(entry->d_type == DT_DIR)
		{
			CreateDir(strsrc,strdest);
		}
		else
		{
			struct patharg *tmp = malloc(sizeof(struct patharg));      //需另外申请一片内存，否则别的线程再操作此内存会出错			
			strcpy(tmp -> Src,strsrc);
			strcpy(tmp -> Dest,strdest);
			while(add_task(CopyFile,tmp) == false);
		}
	}
	closedir(dirsrc);
}


int main(int argc,char *argv[])
{
	if(argc != 3)
	{
		printf("param error\n");
		return -1;
	}
	
	pool = malloc(sizeof(thread_pool));
	if(init_pool(2) == false)
	{
		printf("init error\n");
		return -1;
	}
	printf("init success\n");
			
	CreateDir(argv[1],argv[2]);

	while(1)
	{
		if(pool -> waiting_tasks == 0)
		{
			printf("works down\n");
			return 0;
		}
	}	
}




















