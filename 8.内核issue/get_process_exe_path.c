/*
 * @FilePath: \undefinedd:\桌面\新建文件夹\StudyFiles\8.内核issue\get_process_exe_path.c
 * @Brief: 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PATH_MAX 4096

char *get_executable_path(int pid, char *path, size_t path_length) {
    char maps_path[PATH_MAX];

    // 构造 maps 文件路径
    snprintf(maps_path, sizeof(maps_path), "/proc/%d/maps", pid);

    // 打开 maps 文件
    FILE* fp = fopen(maps_path, "r");
    if (fp == NULL) {
        perror("fopen failed");
        return NULL;
    }

    // 解析 maps 文件中的可执行文件路径
    char* line = NULL;
    size_t line_size = 0;
    ssize_t line_len;
    char* exe_path = NULL;
    while ((line_len = getline(&line, &line_size, fp)) != -1) {
        if (strstr(line, " r-xp ") != NULL) {  // 可执行文件的映射区域
            char* path_start = strchr(line, '/');
            char* path_end = strchr(line, '\n');
            if (path_start != NULL && path_end != NULL) {
                *path_end = '\0';
                if (strlen(path_start) < path_length) {
                    strncpy(path, path_start, path_length);
                    exe_path = realpath(path, NULL);
                } else {
                    fprintf(stderr, "Executable path too long\n");
                }
                break;
            }
        }
    }

    free(line);
    fclose(fp);
    return exe_path;
}
