#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <sys/stat.h>
char buf[512];
char outname[128];

int main(int argc, char **args)
{
    FILE *fp = fopen(args[1], "r");
    if (fp == NULL)
    {
        printf("no such file %s in directory\n", args[1]);
        return -1;
    } else{
        printf("open file %s successfully\n", args[1]);
    }
    struct stat statbuf;
    stat(args[1], &statbuf);
    int fsize = statbuf.st_size;

    memset(buf, 0, 512 * sizeof(char));
    buf[510] = 0x55;
    buf[511] = 0xaa;
    printf("block is %dB\n", fsize);
    fread(buf, sizeof(char), fsize, fp);
    fclose(fp);

    memset(outname, 0, 128 * sizeof(char));
    if (argc > 2)
        memcpy(outname, args[2], strlen(args[2]) * sizeof(char));
    else
        memcpy(outname, args[1], strlen(args[1]) * sizeof(char));

    FILE *fout = fopen(outname, "w");
    if(fout == NULL){return -2;}
    fwrite(buf, sizeof(char), 512, fout);
    fclose(fout);
    return 0;
}