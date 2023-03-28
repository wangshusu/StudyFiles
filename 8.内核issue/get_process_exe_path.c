
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <link.h>

char *get_process_exe_path(pid_t pid) {
    char maps_path[64];
    snprintf(maps_path, sizeof(maps_path), "/proc/%d/maps", pid);

    // 打开 /proc/<PID>/maps 文件
    int fd = open(maps_path, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return NULL;
    }

    // 读取 /proc/<PID>/maps 文件的内容
    char buf[4096];
    ssize_t n = read(fd, buf, sizeof(buf) - 1);
    if (n < 0) {
        perror("read");
        close(fd);
        return NULL;
    }
    buf[n] = '\0';

    // 在 /proc/<PID>/maps 文件中查找进程加载的共享库
    char *pos = buf;
    char *exe_path = NULL;
    while ((pos = strstr(pos, " r-xp ")) != NULL) {
        // 解析共享库的路径
        char *end = strchr(pos, '\n');
        if (end == NULL) {
            end = buf + n;
        }
        char path[1024];
        memset(path, 0, sizeof(path));
        memcpy(path, pos + 7, end - pos - 7);
        path[strlen(path) - 1] = '\0';

        // 查找共享库的符号表，并从中解析出可执行文件的路径
        void *handle = dlopen(path, RTLD_NOW);
        if (handle != NULL) {
            Dl_info info;
            if (dladdr((void *)get_process_exe_path, &info) != 0) {
                if (info.dli_fname != NULL) {
                    exe_path = strdup(info.dli_fname);
                }
            }
            dlclose(handle);
        }

        if (exe_path != NULL) {
            break;
        }
        pos = end;
    }

    close(fd);

    return exe_path;
}

