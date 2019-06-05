#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <getopt.h>
#include <arpa/inet.h>
#include "util.h"
#include "../../includes/gptl.h"

static void usage()
{
    const char *help =
        "Usage: gptl-tool [options]\n"
        "options:\n"
        "  create a package:\n"
        "    --output  <file> | -O <file> output packaged file\n"
        "    --opensbi <file> | -o <file> opensbi binary that needs to be packaged\n"
        "    --kernel  <file> | -k <file> kernel images that needs to be packaged\n"
        "    --initrd  <file> | -r <file> initramfs that needs to be packaged\n"
        "    --fdt     <file> | -f <file> flatten device tree that needs to be packaged\n"
        "    --cmdline <str>  | -c <str>  cmdline of kernel\n\n"
        "  unpack a package:\n"
        "    --extract <file> | -e <file> extract a gptl package\n\n"
        "  dump infomation of a package:\n"
        "    --info    <file> | -i <file> dump infomation of a gptl package\n\n"
        "    --help           | -h        print this message\n\n";
    fprintf(stderr, "%s", help);
    exit(1);
}

static char *optstring = "o:k:r:f:c:O:e:i:h";

static struct option longopts[] = {
    {"opensbi", required_argument, NULL, 'o'},
    {"kernel", required_argument, NULL, 'k'},
    {"initrd", required_argument, NULL, 'r'},
    {"fdt", required_argument, NULL, 'f'},
    {"cmdline", required_argument, NULL, 'c'},
    {"output", required_argument, NULL, 'O'},
    {"extract", required_argument, NULL, 'e'},
    {"info", required_argument, NULL, 'i'},
    {"help", no_argument, NULL, 'h'},
    {0, 0, 0, 0}
};

static char *info_path;
static char *extract_path;
static char *output_path,
    *opensbi_path, *kernel_path, *initrd_path, *fdt_path, *cmdline;

static void parse_argv(int argc, char *argv[])
{
    int c;

    if (argc == 1)
        usage();

    while ((c = getopt_long(argc, argv, optstring, longopts, NULL)) != -1) {
        switch (c) {
        case 'o':
            opensbi_path = optarg;
            break;
        case 'k':
            kernel_path = optarg;
            break;
        case 'r':
            initrd_path = optarg;
            break;
        case 'f':
            fdt_path = optarg;
            break;
        case 'c':
            cmdline = optarg;
            break;
        case 'O':
            output_path = optarg;
            break;
        case 'e':
            extract_path = optarg;
            break;
        case 'i':
            info_path = optarg;
            break;
        case 'h':
            usage();
            break;
        }
    }
}

#define ALIGN_UP(n,a) (((n) + (a) - 1) & ~(a - 1))

static void *create_image(void)
{
    size_t offset = 0;
    struct gptl_header hdr;

    memset(&hdr, 0, sizeof(hdr));
    gptl_header_set_field(&hdr, magic, 0x4750544c);
    gptl_header_set_field(&hdr, version, 0);

    offset += sizeof(struct gptl_header);
    if (cmdline)
        offset += strlen(cmdline) + 1;
    gptl_header_set_field(&hdr, header_size, offset);

    offset = ALIGN_UP(offset, 4096);
    if (opensbi_path) {
        size_t o = offset;
        size_t s = file_size(opensbi_path);
        gptl_header_set_field(&hdr, opensbi_offset, o);
        gptl_header_set_field(&hdr, opensbi_size, s);
        offset += s;
    }

    offset = ALIGN_UP(offset, 4096);
    if (kernel_path) {
        size_t o = offset;
        size_t s = file_size(kernel_path);
        gptl_header_set_field(&hdr, kernel_offset, o);
        gptl_header_set_field(&hdr, kernel_size, s);
        offset += s;
    }

    offset = ALIGN_UP(offset, 4096);
    if (initrd_path) {
        size_t o = offset;
        size_t s = file_size(initrd_path);
        gptl_header_set_field(&hdr, initrd_offset, o);
        gptl_header_set_field(&hdr, initrd_size, s);
        offset += s;
    }

    offset = ALIGN_UP(offset, 4096);
    if (fdt_path) {
        size_t o = offset;
        size_t s = file_size(fdt_path);
        gptl_header_set_field(&hdr, fdt_offset, o);
        gptl_header_set_field(&hdr, fdt_size, s);
        offset += s;
    }

    gptl_header_set_field(&hdr, total_size, offset);

    void *buff = malloc(offset);

    memcpy(buff, &hdr, sizeof(hdr));
	if (cmdline)
		strcpy(buff + sizeof(hdr), cmdline);

    if (opensbi_path)
        load_file(buff + gptl_header_get_field(&hdr, opensbi_offset),
                  opensbi_path);
    if (kernel_path)
        load_file(buff + gptl_header_get_field(&hdr, kernel_offset),
                  kernel_path);
    if (initrd_path)
        load_file(buff + gptl_header_get_field(&hdr, initrd_offset),
                  initrd_path);
    if (fdt_path)
        load_file(buff + gptl_header_get_field(&hdr, fdt_offset),
                  fdt_path);

    return buff;
}

static void extract_image(const char *path)
{
    char wpath[1024];
    struct gptl_header *hdr = load_file(NULL, path);

    if (gptl_header_get_field(hdr, opensbi_size)) {
        size_t offset = gptl_header_get_field(hdr, opensbi_offset);
        size_t size = gptl_header_get_field(hdr, opensbi_size);
        sprintf(wpath, "%s.opensbi", path);
        save_file((void *) hdr + offset, size, wpath);
    }

    if (gptl_header_get_field(hdr, kernel_size)) {
        size_t offset = gptl_header_get_field(hdr, kernel_offset);
        size_t size = gptl_header_get_field(hdr, kernel_size);
        sprintf(wpath, "%s.kernel", path);
        save_file((void *) hdr + offset, size, wpath);
    }

    if (gptl_header_get_field(hdr, initrd_size)) {
        size_t offset = gptl_header_get_field(hdr, initrd_offset);
        size_t size = gptl_header_get_field(hdr, initrd_size);
        sprintf(wpath, "%s.initrd", path);
        save_file((void *) hdr + offset, size, wpath);
    }

    if (gptl_header_get_field(hdr, fdt_size)) {
        size_t offset = gptl_header_get_field(hdr, fdt_offset);
        size_t size = gptl_header_get_field(hdr, fdt_size);
        sprintf(wpath, "%s.fdt", path);
        save_file((void *) hdr + offset, size, wpath);
    }

    if (gptl_header_get_field(hdr, header_size) > sizeof(*hdr)) {
        size_t offset = sizeof(*hdr);
        size_t size = strlen((void *) hdr + offset);
        sprintf(wpath, "%s.cmdline", path);
        save_file((void *) hdr + offset, size, wpath);
    }
}

static void dump_image_info(const char *path)
{
    struct gptl_header *hdr = load_file(NULL, path);

    printf("magic          : %#x\n", gptl_header_get_field(hdr, magic));
    printf("version        : %#x\n", gptl_header_get_field(hdr, version));
    printf("header_size    : %#x\n",
           gptl_header_get_field(hdr, header_size));
    printf("total_size     : %#x\n",
           gptl_header_get_field(hdr, total_size));

    if (gptl_header_get_field(hdr, opensbi_size)) {
        printf("opensbi_offset : %#x\n",
               gptl_header_get_field(hdr, opensbi_offset));
        printf("opensbi_size   : %#x\n",
               gptl_header_get_field(hdr, opensbi_size));
    }
    if (gptl_header_get_field(hdr, kernel_size)) {
        printf("kernel_offset  : %#x\n",
               gptl_header_get_field(hdr, kernel_offset));
        printf("kernel_size    : %#x\n",
               gptl_header_get_field(hdr, kernel_size));
    }

    if (gptl_header_get_field(hdr, initrd_size)) {
        printf("initrd_offset  : %#x\n",
               gptl_header_get_field(hdr, initrd_offset));
        printf("initrd_size    : %#x\n",
               gptl_header_get_field(hdr, initrd_size));
    }

    if (gptl_header_get_field(hdr, fdt_size)) {
        printf("fdt_offset     : %#x\n",
               gptl_header_get_field(hdr, fdt_offset));
        printf("fdt_size       : %#x\n",
               gptl_header_get_field(hdr, fdt_size));
    }
    printf("cmdline        : %s\n", hdr->commandline);
}

int main(int argc, char *argv[])
{
    parse_argv(argc, argv);
    if (info_path)
        dump_image_info(info_path);
    if (extract_path)
        extract_image(extract_path);
    if (output_path) {
        FILE *fp = fopen(output_path, "wb");
        if (fp == NULL) {
            fprintf(stderr, "[%s %d] %s: ", __FILE__, __LINE__,
                    output_path);
            perror(NULL);
            exit(1);
        }
        struct gptl_header *hdr = create_image();
        fwrite(hdr, 1, gptl_header_get_field(hdr, total_size), fp);
        fclose(fp);
    }
    return 0;
}
