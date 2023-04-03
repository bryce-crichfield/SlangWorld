#include "SlimPlatform.h"
#include <stdio.h>

int main(int argc, char** argv)
{
    slim_platform_init(argc, argv);
    do {
        slim_platform_update();
    } while (1);
    slim_platform_exit();
    return 0;
}