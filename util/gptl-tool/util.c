#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

size_t file_size(const char *path)
{
    struct stat s;
    if (!stat(path, &s))
        return s.st_size;
    fprintf(stderr, "[%s %d] %s: ", __FILE__, __LINE__, path);
    perror(NULL);
    exit(1);
}

void *load_file(void *buff, const char *path)
{
    size_t size = file_size(path);
    FILE *fp = fopen(path, "rb");
    if (fp == NULL) {
        fprintf(stderr, "[%s %d] %s: ", __FILE__, __LINE__, path);
        perror(NULL);
        exit(1);
    }
    buff = buff ? buff : malloc(size);
    if (buff == NULL) {
        fprintf(stderr, "[%s %d] %s: ", __FILE__, __LINE__, path);
        perror(NULL);
        exit(1);
    }
    fread(buff, size, 1, fp);
    fclose(fp);
    return buff;
}

void save_file(void *buff, size_t size, const char *path)
{
    FILE *fp = fopen(path, "wb");
    if (fp == NULL) {
        fprintf(stderr, "[%s %d] %s: ", __FILE__, __LINE__, path);
        perror(NULL);
        exit(1);
    }
    fwrite(buff, size, 1, fp);
    fclose(fp);
}
