#define _BSD_SOURCE
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#define builtin_num 3  //已经设置环境变量个数
int lsh_cd(char **p)
{
  if(p[1]==NULL)
  {
    return 2;//直接退出
  }
  else 
  {
    if(chdir(p[1])!=0)
      perror("lsh");
  }
  return 1;

}

int lsh_exit(char **p)
{
  return 0;
}

int lsh_help(char **p)
{
  printf("石海涛的 shell\n");
  printf("本shell只提供如下几种内置指令，更多指令还在完善中^-^\n");
  for(int i=0;i<builtin_num;i++)
  {
    printf("*****%s*******\n",p[i]);
  }
  printf("使用man指令来获取更多信息\n");
}

int lsh_echo(char**p)
{

}

int env_operate(char **env,char ** phrase)   //设置环境变量
{
  int flag=0;
  int i=0;
  while(phrase[0][i])           //设置环境变量有两个条件：只有一个语句 且语句中出现字符：=
  {
    if(phrase[0][i]=='=')
    {
      flag=1;
      break;
    }
    i++;
  }
  if(phrase[1]==NULL && flag)    //设置环境变量的操作
  {
    char*name=strtok(phrase[0],"=");
    char*right=strtok(NULL,"=");
    char result[2048];
    i=0;
    while(right[i])
    {
      if(right[i]=='$')
      {
        flag=2;
        right[i]='\0';
        break;
      }
      i++;
    }

    if(flag==2)
    {
      char* cur=right+i+1;
      char env2[2048];
      char *p;
      i=0;
      while(env[i])
      {
        strcpy(env2,env[i]);
        p=strtok(env2,"=");
        if(strcmp(p,cur)==0)
        {
          p=strtok(NULL,"=");
          strcpy(result,right);
          strcat(result,p);
          break;
        }
        i++;
      }

    }
    else 
    {
      strcpy(result,right);
    }
    setenv(name,result,1);
    return 1;
  }
  return 0;
}


int  built_in_command(char**phrase,int *return_value,char **env)
{

  if(env_operate(env,phrase))
  {
    return 0;
  }
  char*built_in_str[]={"cd","help","exit","echo"};
  int (*built_in_func[])(char**)={
    &lsh_cd,
    &lsh_help,
    &lsh_exit,
    &lsh_echo
  };

  for(int i=0;i<builtin_num;i++)
  {
    if(strcmp(phrase[0],built_in_str[i])==0)
    {
      if(strcmp(built_in_str[i],"help")==0)
        *return_value=(built_in_func[i])(built_in_str);
      else 
        *return_value=(built_in_func[i])(phrase);
      return 0;
    }
  }
  return 1;
}
