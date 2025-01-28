#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>

#define IN_FILE_NAME "./war-and-peace.txt"
#define OUT_FILE_NAME "./war-and-peace-copy.txt"

int main()
{
    int in_fd, out_fd;
    struct stat sb;
    char *src, *dest;

    printf("Copying from %s to %s using memory-mapped files\n", IN_FILE_NAME, OUT_FILE_NAME);

    // Open the input file
    in_fd = open(IN_FILE_NAME, O_RDONLY);
    if (in_fd == -1)
    {
        perror("Error opening input file");
        exit(1);
    }

    // Get the size of the input file
    if (fstat(in_fd, &sb) == -1)
    {
        perror("Error getting input file size");
        close(in_fd);
        exit(1);
    }

    // Memory-map the input file
    src = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, in_fd, 0);
    if (src == MAP_FAILED)
    {
        perror("Error memory-mapping input file");
        close(in_fd);
        exit(1);
    }

    // Open the output file
    out_fd = open(OUT_FILE_NAME, O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (out_fd == -1)
    {
        perror("Error opening output file");
        munmap(src, sb.st_size);
        close(in_fd);
        exit(1);
    }

    // Ensure the output file is the same size as the input file
    if (ftruncate(out_fd, sb.st_size) == -1)
    {
        perror("Error setting output file size");
        munmap(src, sb.st_size);
        close(in_fd);
        close(out_fd);
        exit(1);
    }

    // Memory-map the output file
    dest = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, out_fd, 0);
    if (dest == MAP_FAILED)
    {
        perror("Error memory-mapping output file");
        munmap(src, sb.st_size);
        close(in_fd);
        close(out_fd);
        exit(1);
    }

    // Copy the input file to the output file using memory
    memcpy(dest, src, sb.st_size);

    // Clean up
    munmap(src, sb.st_size);
    munmap(dest, sb.st_size);
    close(in_fd);
    close(out_fd);

    printf("Done - copied %ld bytes\n", sb.st_size);
    return 0;
}
