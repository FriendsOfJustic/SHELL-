#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<sys/wait.h>

void main_excute(char **p,int flag)
{
  if(flag==0) //啥都没有
  {
    if(execvp(p[0],p)==-1)
      perror("execvp");
  }
  else if(flag>0) //重定向
  {
    int i=0;
    while(p[i])
    {
      if(strcmp(p[i],">")==0 || strcmp(p[i],">>")==0)
      {
        if(strcmp(p[i],">>")==0)
          flag=2;
        p[i]=NULL;
        break;
      }
      i++;
    }
    int fd1;
    if(flag==1)
      fd1=open(p[++i],O_CREAT|O_WRONLY,0664);
    else
      fd1=open(p[++i],O_CREAT|O_WRONLY|O_APPEND,0664);

    dup2(fd1,1);

    execvp(p[0],p);

  }
  else if(flag<0)  //管道   双引号未处理
  {
    int fd[2];
    pipe(fd);
    //printf("管道创建成功\n");
    int i=0;
    while(p[i])
    {
      if(strcmp(p[i],"|")==0)
      {
        p[i]=NULL;
        i++;
        break;
      }
      i++;
    }
    int ret=fork();
    if(ret==0)   //子进程向管道里面写入
    {

      close(fd[0]);
      dup2(fd[1],1);
      //printf("子进程开始执行");
      if(execvp(p[0],p)==-1)
        perror("execvp");
    }
    else       //父进程从管道里面读取
    {
      close(fd[1]);    //将写端关闭
      waitpid(ret,NULL,0);
      dup2(fd[0],0);    //将读端从 屏幕读取 修改为从 管道读取
      //printf("父进程开始执行");
      
      if(flag!=-1)
      {
        int pos =-flag;
        p[pos]=NULL;
        int fd1;
        if(flag==1)
          fd1=open(p[pos+1],O_CREAT|O_WRONLY,0664);
        else
          fd1=open(p[pos+1],O_CREAT|O_WRONLY|O_APPEND,0664);

        dup2(fd1,1);        
      }

      if(execvp(p[i],p+i)==-1)
        perror("execvp");
    }

  }

}
