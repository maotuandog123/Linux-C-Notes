#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    while (1)
    {
        prompt();

        getline();

        parse();

        if (内部命令)
        {
            ;
        }
        else   // 外部命令
        {
            fork();
            if (< 0)
            {
                ;
            }

            if (0 ==)   // child
            {
                execXX;
                perror();
                exit(1);
            }
            else   // parent
            {
                wait();
            }
        }
    }

    exit(0);
}