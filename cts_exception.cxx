#include <cstdio>
#define W_PREFIX "[!] Warning: "

void CTS_WARNING(const char *str)
{
    printf(W_PREFIX);
    printf("%s", str);
}

void CTS_WARNING(const char *fmt, const char *str)
{
    printf(W_PREFIX);
    printf(fmt, str);
}

