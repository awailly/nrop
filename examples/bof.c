#include <string.h>

int boom(char *argv[])
{
    char a[200];

    strcpy(a, argv[1]);

    return 1;
}
int main(int argc, char *argv[])
{
    boom(argv);
    return 0;
}
