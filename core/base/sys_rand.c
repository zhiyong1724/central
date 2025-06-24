#include "sys_rand.h"
static unsigned int s_seed = 2082931198;
static const unsigned long long a = 127ll;
static const unsigned long long c = 524287ll;
void sys_srand(unsigned int seed)
{
    s_seed = seed;
}

unsigned int sys_rand()
{
    unsigned long long n = (a * s_seed + c) % (SYS_RAND_MAX + 1);
    s_seed = (unsigned int)n;
    return s_seed;
}