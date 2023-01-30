#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUFFER_SIZE 1024
#define EXIT_SHELL -100
char *builtin_str[] = {"cd", "help", "exit"}; // 内置指令的名称

int func_cd(const char *args)
{
    int ret = chdir(args);
    if (ret == -1)
    {
        printf("cd is error\n");
        return -1;
    }
    else
        return 0;
}

int func_help(const char *args)
{
    printf("这是 石海涛 的shell\n 我的shell支持重定向、管道、简单内置命令\n更多内容访问我的博客\n ");
    return 0;
}

int func_exit(const char *args)
{
    return EXIT_SHELL;
}

int (*func[])(const char *p) = {&func_cd, &func_help, &func_exit};
void print_info(char **envp)
{
    char **cur = envp;
    char HOST_NAME[100];   // 主机型号和名称
    char USER[100];        // 用户名
    char PWD[100];         // 路径
    char CUR_dic[20];      // 当前目录
    while ((*cur) != NULL) // env最后一个元素为NULL
    {
        char tmp[30]; // 用来存储截取每个用户变量名
        int j = 0;
        for (j = 0; j < strlen(*cur); j++) // 等号前面的一定是变量名
        {
            if ((*cur)[j] == '=')
                break;
            else
                tmp[j] = (*cur)[j];
        }
        tmp[j] = '\0';

        if (strcmp(tmp, "HOSTNAME") == 0)
            strcpy(HOST_NAME, strstr(*cur, "=") + 1);

        if (strcmp(tmp, "USER") == 0)
            strcpy(USER, strstr(*cur, "=") + 1);

        if (strcmp(tmp, "PWD") == 0) // pwd作为路径还需要特殊处理，才能得到当前目录
        {
            strcpy(PWD, strstr(*cur, "=") + 1);
            int i = 0;
            for (i = strlen(PWD); i >= 0; i--)
            {
                if (PWD[i] == '/')
                    break;
            }
            strcpy(CUR_dic, PWD + i + 1);
        }
        cur++;
    }
    if (strcmp(USER, "root") == 0) // 区分 超级用户 和 普通用户
        printf("[%s@%s %s]# ", USER, HOST_NAME, CUR_dic);
    else
        printf("[%s@%s %s]$ ", USER, HOST_NAME, CUR_dic);
}

void read_comand(char **buffer)
{
    // 开辟静态内存
    //  int sz = read(0, *buffer, 200); // 屏幕输入绝对不可能为0 ，至少都有一个回车键
    // fgets(*buffer, 200, stdin);
    //  assert(sz == 0);

    // 动态开辟内存
    char *p = (char *)malloc(BUFFER_SIZE * sizeof(char));
    int buffer_size = BUFFER_SIZE;

    if (p == NULL) // 开辟内存失败
    {
        fprintf(stderr, "read_line error\n");
        exit(0);
    }
    int i = 0;
    while (1)
    {
        char c = getchar();

        if (c == EOF || c == '\n')
        {
            p[i] = '\0';
            break;
        }
        else
        {
            p[i] = c;
            i++;
        }
        if (i >= buffer_size) // 如果空间不够 ， 追加初始一倍 的空间
        {
            buffer_size += BUFFER_SIZE;
            p = realloc(p, buffer_size);
        }
    }
    *buffer = p;
}

char **append_space(char **src, int curize) // 专门为split_line设计的扩容函数
{
    int newsize = curize * 2;

    char **tmp = (char **)malloc(newsize * sizeof(char **));
    memcpy(tmp, src, (curize) * sizeof(char **));
    free(src);
    return tmp;
}

char **split_line(char *buffer, int *flag) //  1 重定向 -1 管道  0 啥都不是
{
    char **p = (char **)malloc(6 * sizeof(char **));
    int size = 6;

    if (strstr(buffer, ">")) // 重定向
    {
        // 将buffer按照重定向符切成两个字符串
        *flag = 1;
        char *p2 = strstr(buffer, ">"); // 重定向符 后面的字符串
        *p2 = '\0';
        p2 += 1;

        int i = 0;
        p[i++] = strtok(buffer, " ");
        while ((p[i] = strtok(NULL, " ")) != NULL)
        {
            i++;
            if (i >= size)
            {
                p = append_space(p, i);
            }
        }

        i++;           // 两个 切成块的 字符串 用 NULL来分割
        if (i >= size) // 检查是否要扩容
        {
            p = append_space(p, i);
        }

        p[i++] = strtok(p2, " ");
        while ((p[i] = strtok(NULL, " ")) != NULL)
        {
            i++;
            if (i >= size) // 检查是否要扩容
            {
                p = append_space(p, i);
            }
        }
        return p;
    }
    else if (strstr(buffer, "|"))
    {
        // 将buffer按照重定向符切成两个字符串
        *flag = -1;
        char *p2 = strstr(buffer, "|"); // 重定向符 后面的字符串
        *p2 = '\0';
        p2 += 1;

        int i = 0;
        p[i++] = strtok(buffer, " ");
        while ((p[i] = strtok(NULL, " ")) != NULL)
        {
            i++;
            if (i >= size) // 检查是否要扩容
            {
                p = append_space(p, i);
            }
        }

        i++;           // 两个 切成块的 字符串 用 NULL来分割
        if (i >= size) // 检查是否要扩容
        {
            p = append_space(p, i);
        }

        p[i++] = strtok(p2, " ");
        while ((p[i] = strtok(NULL, " ")) != NULL)
        {
            i++;
            if (i >= size) // 检查是否要扩容
            {
                p = append_space(p, i);
            }
        }
        return p;
    }
    else // 没有管道 也没有重定向
    {
        *flag = 0;
        int i = 0;
        p[i++] = strtok(buffer, " ");
        while ((p[i] = strtok(NULL, " ")) != NULL)
        {
            i++;
            if (i >= size) // 检查是否要扩容
            {
                p = append_space(p, i);
            }
        }
        return p;
    }
}

int excute_command(char **args) // 执行单个指令
{
    for (int i = 0; i < sizeof(builtin_str) / sizeof(char *); i++) // 检查是否是内置命令
    {
        if (strcmp(args[0], builtin_str[i]) == 0)
        {
            return func[i](args[1]);
        }
    }
    // 不是内置命令就要开始进程替换了
    __pid_t i = fork();
    if (i == 0)
    {
        execvp(args[0], args);
    }

    int status = 0;
    waitpid(i, &status, 0);

    return status >> 8;
}

int excute_line(char **buffer, int flag) // 执行指令的函数
{

    if (flag == 0)
    {
        int ret = excute_command(buffer);
        return ret;
    }
    else if (flag == 1) // 重定向
    {

        char **file_name = buffer;
        while (*file_name != NULL)
        {
            file_name++;
        }

        file_name++; // 找到文件名

        pid_t i = fork();
        if (i == 0)
        {
            int fd = open(*file_name, O_CREAT | O_RDWR, 0644);
            dup2(fd, 1);
            execvp(buffer[0], buffer);
        }
        int status;
        waitpid(i, &status, 0);
        return status >> 8;
    }
    else // 管道
    {

        char **command2 = buffer;
        while (*command2 != NULL)
        {
            command2++;
        }

        command2++; // 找到指令2的起始位置
        // printf("%s\n", *command2);

        pid_t i = fork();
        if (i == 0)
        {
            int fd[2];
            if (pipe(fd) == -1)
                fprintf(stderr, "create pipe fail\n");
            pid_t son = fork();
            if (son == 0)
            {
                close(fd[0]);
                dup2(fd[1], 1);
                printf("1\n");
                execvp(buffer[0], buffer);
            }
            // 父进程

            waitpid(son, NULL, 0);
            close(fd[1]);
            dup2(fd[0], 0);
            // printf("2\n");
            execvp(command2[0], command2);
        }
        int status;
        waitpid(i, &status, 0);
        return status >> 8;
    }
}

int main(int argc, char **argv, char **envp)
{
    while (1)
    {
        char *buffer = NULL; // 存储 输入的命令行

        print_info(envp); // 输出命令行前面的主机信息

        read_comand(&buffer); // 读取 屏幕上输入的指令

        int flag = 0;

        char **p = split_line(buffer, &flag); // 将字符串裂项成 字符串数组

        int ret = excute_line(p, flag); // 执行 指令

        free(buffer);
        free(p); // 释放内存

        if (ret == EXIT_SHELL) // 接受返回值并判断是否要退出SHELL
            break;
    }
}