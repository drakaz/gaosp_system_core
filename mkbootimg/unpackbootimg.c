#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <libgen.h>

#include "mincrypt/sha.h"
#include "bootimg.h"

typedef unsigned char byte;

int read_padding(FILE* f, unsigned itemsize, int pagesize)
{
    byte* buf = (byte*)malloc(sizeof(byte) * pagesize);
    unsigned pagemask = pagesize - 1;
    unsigned count;

    if((itemsize & pagemask) == 0) {
        free(buf);
        return 0;
    }

    count = pagesize - (itemsize & pagemask);

    fread(buf, count, 1, f);
    free(buf);
    return count;
}

void write_string_to_file(char* file, char* string)
{
    FILE* f = fopen(file, "w");
    fwrite(string, strlen(string), 1, f);
    fclose(f);
}


int main(int argc, char** argv)
{
    if (argc != 2 && argc != 3)
    {
        printf("Usage:\n");
        printf("\tunpackbootimg <boot.img>\n");
        printf("\tunpackbootimg <boot.img> <output_directory>\n");
        return 0;
    }
    
    char tmp[PATH_MAX];
    char* directory = "./";
    if (argc == 3)
    {
        directory = argv[2];
    }
    //printf("%s\n", directory);
    
    int total_read = 0;
    FILE* f = fopen(argv[1], "rb");
    boot_img_hdr header;

    //printf("Reading header...\n");
    fread(&header, sizeof(header), 1, f);
    printf("BOARD_KERNEL_CMDLINE %s\n", header.cmdline);
    printf("BOARD_KERNEL_BASE %x\n", header.kernel_addr - 0x00008000);
    printf("BOARD_PAGE_SIZE %x\n", header.page_size);
    
    //printf("cmdline...\n");
    sprintf(tmp, "%s/%s", directory, basename(argv[1]));
    strcat(tmp, "-cmdline");
    write_string_to_file(tmp, header.cmdline);
    
    //printf("base...\n");
    sprintf(tmp, "%s/%s", directory, basename(argv[1]));
    strcat(tmp, "-base");
    char basetmp[200];
    sprintf(basetmp, "%x", header.kernel_addr - 0x00008000);
    write_string_to_file(tmp, basetmp);
    
    total_read += sizeof(header);
    //printf("total read: %d\n", total_read);
    total_read += read_padding(f, sizeof(header), header.page_size);

    sprintf(tmp, "%s/%s", directory, basename(argv[1]));
    strcat(tmp, "-zImage");
    FILE *k = fopen(tmp, "wb");
    byte* kernel = (byte*)malloc(header.kernel_size);
    //printf("Reading kernel...\n");
    fread(kernel, header.kernel_size, 1, f);
    total_read += header.kernel_size;
    fwrite(kernel, header.kernel_size, 1, k);
    fclose(k);

    //printf("total read: %d\n", header.kernel_size);
    total_read += read_padding(f, header.kernel_size, header.page_size);

    sprintf(tmp, "%s/%s", directory, basename(argv[1]));
    strcat(tmp, "-ramdisk.gz");
    FILE *r = fopen(tmp, "wb");
    byte* ramdisk = (byte*)malloc(header.ramdisk_size);
    //printf("Reading ramdisk...\n");
    fread(ramdisk, header.ramdisk_size, 1, f);
    total_read += header.ramdisk_size;
    fwrite(ramdisk, header.ramdisk_size, 1, r);
    fclose(r);
    
    fclose(f);
    
    //printf("Total Read: %d\n", total_read);
    return 0;
}