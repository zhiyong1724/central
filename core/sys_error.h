#ifndef __SYS_ERROR_H__
#define __SYS_ERROR_H__
enum sys_error_t
{
    SYS_ERROR_OK = 0,                      
    SYS_ERROR_PERM = -1,                    //不允许这样操作
    SYS_ERROR_NOENT = -2,                   //没有这样的文件或目录
    SYS_ERROR_SRCH = -3,                    //没有这样的进程
    SYS_ERROR_INTR = -4,                    //系统调用中断
    SYS_ERROR_IO = -5,                      //IO操作错误
    SYS_ERROR_NXIO = -6,                    //没有这样的设备或地址
    SYS_ERROR_2BIG = -7,                    //参数列表太长
    SYS_ERROR_NOEXEC = -8,                  //执行文件格式错误
    SYS_ERROR_BADF = -9,                    //错误的文件句柄
    SYS_ERROR_CHILD = -10,                  //没有子进程
    SYS_ERROR_AGAIN = -11,                  //重试一次
    SYS_ERROR_NOMEM = -12,                  //内存不足
    SYS_ERROR_ACCES = -13,                  //没有权限操作
    SYS_ERROR_FAULT = -14,                  //错误的地址
    SYS_ERROR_NOTBLK = -15,                 //没有块设备
    SYS_ERROR_BUSY = -16,                   //设备忙
    SYS_ERROR_EXIST = -17,                  //文件已存在
    SYS_ERROR_XDEV = -18,                   //跨设备的链接
    SYS_ERROR_NODEV = -19,                  //没有这样的设备
    SYS_ERROR_NOTDIR = -20,                 //没有这个目录
    SYS_ERROR_ISDIR = -21,                  //是一个目录
    SYS_ERROR_INVAL = -22,                  //无效参数
    SYS_ERROR_NFILE = -23,                  //文件表溢出
    SYS_ERROR_MFILE = -24,                  //打开的文件太多
    SYS_ERROR_NOTTY = -25,                  //没有打印输出
    SYS_ERROR_TXTBSY = -26,                 //文本文件忙
    SYS_ERROR_MFBIG = -27,                  //文件太大
    SYS_ERROR_NOSPC = -28,                  //设备没有剩余空间
    SYS_ERROR_SPIPE = -29,                  //非法seek
    SYS_ERROR_ROFS = -30,                   //只读文件系统
    SYS_ERROR_MLINK = -31,                  //太多链接文件
    SYS_ERROR_PIPE = -32,                   //管道损坏
    SYS_ERROR_NAMETOOLONG = -36,            //文件名太长
    SYS_ERROR_NOTEMPTY = -39,               //文件夹非空
    SYS_ERROR_NODATA = -61,                 //无有效数据
    SYS_ERROR_ILSEQ = -84,                  //非法字节序
};
#endif