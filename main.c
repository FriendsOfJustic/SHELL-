//#pragma once
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<sys/wait.h>
#include"built_in.h"
#include"main_excute.h"
void print_info(char *info)
{
  char dir[100];
  char* cur_dir[200];
  getcwd(dir,100);
  cur_dir[0]=strtok(dir,"/");
  int i=1;
  while(cur_dir[i]=strtok(NULL,"/"))
  {
    i++;
  }

  //strcat(info," ");
  //strcat(info,cur_dir[i-1]);
  printf("%s %s@@@@]$ ",info,cur_dir[i-1]);
}

void line_read(char *line)
{
  fgets(line,1024,stdin);
  line[strlen(line)-1]='\0';
}

char** line_cut(char *line)
{
  char**result;
  char*cur;
  int sz=0;
  cur=strtok(line," ");
  sz=strlen(cur);
  result=(char**)malloc(sizeof(char*));
  result[0]=(char*)malloc((sz+1)*sizeof(char));
  strcpy(result[0],cur);
  int i=1;
  while(cur=strtok(NULL," "))
  {
    sz=strlen(cur);
    result=(char**)realloc(result,(i+1)*sizeof(char*));
    result[i]=(char*)malloc((sz+1)*sizeof(char));
    strcpy(result[i],cur);
    i++;
  }
  result=(char**)realloc(result,(i+1)*sizeof(char*));
  result[i]=NULL;
  return result;
}




void phrase_excute(char **phrase,int *return_value,char**env) 
{
  
  if(built_in_command(phrase,return_value,env))
  {
    int flag=0;  //1 重定向  -1 管道  0 啥都没有
    int i=0;
    while(phrase[i])
    {
        if(strcmp(phrase[i],">")==0 || strcmp(phrase[i],">>")==0)
          flag=1;
        else if(strcmp(phrase[i],"|")==0)
          flag=-1;
        i++;
    }
    int ret=fork();
    if(ret==0)   //child process
    {
      main_excute(phrase,flag);
    }
    else if(ret)  //father process
    {
      waitpid(ret,NULL,0);
    }
  }

}

void lush_loop(char *info,char**env)
{
  char line[1024];
  char **phrase;
  int return_value=1;
  while(return_value)
  {
    print_info(info);
    line_read(line);
    //printf(line);
    if(line[0])
    {
      phrase=line_cut(line);
      phrase_excute(phrase,&return_value,env);
    }
  }
}


int main(int argc,char*argv[],char* env[])
{ 
  char info[100]="[";
  char user[10];
  char hostname[20];
  int i=0;
  while(env[i])
  {
    char *p;
    char env2[2048];

    if(env[i])
      strcpy(env2,env[i]);
    
    p=strtok(env2,"=");
    if(strcmp(p,"USER")==0)
    {
      p=strtok(NULL,"=");
      strcpy(user,p);
      //printf(p);
    }
    else if(strcmp(p,"HOSTNAME")==0)
    {
      p=strtok(NULL,"=");
      strcpy(hostname,p);
      //printf(p);
    }
    i++;
  }
  strcat(info,user);
  strcat(info,"@");
  strcat(info,hostname);
  //printf("%s\n",info);
  // 读取bash的配置 
  lush_loop(info,env);
  //准备推出shell
  return EXIT_SUCCESS;
}
