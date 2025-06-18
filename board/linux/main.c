#include "central.h"
#include "sys_task.h"
// #include "sys_port.h"
// #include "ram_block.h"
// #include "ram_block1.h"
// #include "sys_vfs.h"
// #include "lfs_adapter.h"
// #include "fatfs_adapter.h"
// #include "shellio.h"
int main()
{
    sys_init();
    // sys_task_add_cpu_thread 添加CPU
    // ...
    // '
    // '
    // '
    // ...
    // sys_port_init();
    // ram_block_format();
    // ram_block_create();
    // register_lfs();
    // sys_mount("/", "/dev/block");
    // ram_block1_create();
    // ram_block1_format();
    // register_fatfs();
    // sys_mkdir("/sd", VFS_MODE_IRUSR | VFS_MODE_IWUSR);
    // sys_mount("/sd", "/dev/block1");
    // shell_io_init();
    sys_task_start();
    return 0;
}