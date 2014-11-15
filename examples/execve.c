#include <unistd.h>

int main()
{
    char *cmd[] = { "/bin/sh", (char *)0 };

    execve("/bin/sh", cmd, NULL);

    return 0;
}
