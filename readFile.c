#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

char *read_line(FILE *file)
{
    char *ln = NULL;
    size_t n = 0;
    if (getline(&ln, &n, file) == -1)
    {
        return NULL;
    }
    return ln;
}
int main()
{
    FILE *fd = fopen("input.txt", "r");
    char *ln;
    while ((ln = read_line(fd)) != NULL)
    {
        printf("%s", ln);
    }
    printf("\n");

    return 0;
}