#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#define BUF_SIZE 4096

int main() {
    const char* file_path = "/path/to/file";
    int fd = open(file_path, O_WRONLY | O_CREAT, 0666);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    // 获取文件系统块大小
    struct statfs sfs;
    if (fstatfs(fd, &sfs) == -1) {
        perror("fstatfs");
        close(fd);
        exit(EXIT_FAILURE);
    }
    const size_t block_size = sfs.f_bsize;

    // 将偏移量按块大小对齐
    off_t offset = lseek(fd, 0, SEEK_END);
    if (offset == -1) {
        perror("lseek");
        close(fd);
        exit(EXIT_FAILURE);
    }
    offset = (offset / block_size) * block_size;

    // 分配缓冲区
    char* buf = (char*) malloc(BUF_SIZE);
    if (buf == NULL) {
        perror("malloc");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // 写入数据
    ssize_t n_written = pwrite(fd, buf, BUF_SIZE, offset);
    if (n_written == -1) {
        perror("pwrite");
        free(buf);
        close(fd);
        exit(EXIT_FAILURE);
    }

    printf("%zd bytes written at offset %lld\n", n_written, (long long)offset);

    free(buf);
    close(fd);
    return 0;
}
