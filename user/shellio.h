#ifndef __SHELLIO_H__
#define __SHELLIO_H__
#ifdef __cplusplus
extern "C"
{
#endif
int shell_io_init();
const char *get_shell_path();
void set_shell_path(const char *path);
#ifdef __cplusplus
}
#endif
#endif