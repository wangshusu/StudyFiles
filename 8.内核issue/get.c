/*
 * @FilePath: \undefinedd:\桌面\get.c
 * @Brief: 
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#define MAX_PATH 1024

int get_cmdline(char *path, ssize_t length, int pid)
{
    int i;
    ssize_t nread = 0;
    ssize_t file_size = 0;
    FILE* fp = NULL;
    char cmdline[MAX_PATH] = {0};
    char cmd_path[MAX_PATH] = {0};
    
    sprintf(cmd_path, "/proc/%d/cmdline", pid);

    fp = fopen(cmd_path, "r");
    if (fp == NULL)
    {
        return -1;
    }

    while (nread = fread(cmdline, 1, MAX_PATH, fp))
    {
        file_size += nread;
    }
    fclose(fp);

    if (0 == file_size)
    {
        printf("file_size == 0\n");
        return -1;
    }
    if (file_size > length)
    {
        printf("file_size > length file_size : %d\n", file_size);
        return -1;
    }
    
    for (i = 0; i < file_size; i++)
    {
        if (cmdline[i] == '\0')
        {
            cmdline[i] = ' ';
        }
    }

    // 去掉末尾的空格
    strncpy(path, cmdline, file_size - 1);

    return 0;
}

int get_executable_path(int pid, char *path, size_t path_length) {
    int reval = -1;
    char maps_path[MAX_PATH] = {0};
    char cmdline[MAX_PATH] = {0};
    char* line = NULL;
    size_t line_size = 0;
    ssize_t line_len;
    char* exe_path = NULL;
    size_t maps_num = 0;

    // 获取cmdline的内容
    reval = get_cmdline(cmdline, MAX_PATH, pid)
    if (reval < 0)
    {
        return -1;
    }

    // 判断cmdline是否为绝对路径
    if (strstr (cmdline, "./") == NULL)
    {
        strncpy(path, cmdline, path_length);
        return 0;
    }

    // 构造 maps 文件路径
    snprintf(maps_path, sizeof(maps_path), "/proc/%d/maps", pid);

    // 打开 maps 文件
    FILE* fp = fopen(maps_path, "r");
    if (fp == NULL) {
        printf("fopen failed\n");
        return NULL;
    }

    // 解析 maps 文件中的可执行文件路径
    while ((line_len = getline(&line, &line_size, fp)) != -1) 
    {
        char* path_start = strchr(line, '/');
        if (path_start == NULL)
        {
            continue;
        }

        char* path_end = strchr(line, '\n');
        if (path_end == NULL)
        {
            continue;
        }

        *path_end = '\0';
        if (strlen(path_start) < path_length) 
        {
            strncpy(path, path_start, path_length);
            exe_path = realpath(path, NULL);
            maps_num++;
        }
    }
    free(line);
    fclose(fp);

    if (maps_num > 1)
    {
        memset(path, 0, path_length);
        strncpy(path, cmdline, path_length);
        return 0;
    }
    esle if (maps_num == 1)
    {
        return 0;
    }
    strncpy(path, cmdline, path_length);
    return 0;
}


int main(int argc, char *argv[]) {
    
    int pid = atoi(argv[1]);
    char path[MAX_PATH];
    
    get_cmdline(path, MAX_PATH, pid);

    printf("path : |||%s|||\n", path);
}



