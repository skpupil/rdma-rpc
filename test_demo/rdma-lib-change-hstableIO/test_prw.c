#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#define BLOCK_SIZE 4096

int main(int argc, char *argv[])
{
    int fd, ret;
    size_t file_size;
    char *buf;
    off_t offset;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <file_path> <write_string>\n", argv[0]);
        return EXIT_FAILURE;
    }

    fd = open(argv[1], O_WRONLY | O_DIRECT);
    if (fd < 0) {
        fprintf(stderr, "Failed to open file %s: %s\n", argv[1], strerror(errno));
        return EXIT_FAILURE;
    }

    file_size = lseek(fd, 0, SEEK_END);
    if (file_size == (off_t)-1) {
        fprintf(stderr, "Failed to get file size: %s\n", strerror(errno));
        close(fd);
        return EXIT_FAILURE;
    }

    buf = (char*)aligned_alloc(BLOCK_SIZE, BLOCK_SIZE);
    if (buf == NULL) {
        fprintf(stderr, "Failed to allocate memory: %s\n", strerror(errno));
        close(fd);
        return EXIT_FAILURE;
    }

    memset(buf, 0, BLOCK_SIZE);
    memcpy(buf, argv[2], strlen(argv[2]));

    offset = file_size - (file_size % BLOCK_SIZE);
    printf("offset: %ld\n",offset);
    ret = pwrite(fd, buf, BLOCK_SIZE, offset);
    if (ret != BLOCK_SIZE) {
        fprintf(stderr, "Failed to write data: %s\n", strerror(errno));
        free(buf);
        close(fd);
        return EXIT_FAILURE;
    }

    free(buf);
    close(fd);
    return EXIT_SUCCESS;
}

