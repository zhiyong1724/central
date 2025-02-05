## 1. 简介
central是一个标准C写成的嵌入式操作系统内核，提供了内存管理，任务管理，消息队列，信号量，互斥量，虚拟文件系统等功能，只需要实现少部分底层接口，就可以移植到所有平台上面运行。git仓库：https://github.com/zhiyong1724/central.git
## 2. 内存管理
central提供了三种内存管理算法，系统使用了其中的两种，基于页分配的内存管理和基于不定长分配的内存管理。基于页分配的管理算法有合并功能，能避免外部碎片的产生，但是大部分情况都是需要分配不定长的内存，按页分配会造成比较大的内存浪费，所以基于按页分配的内存页，系统提供了另一种分配方式用来分配不定长的内存，这样可以避免了内部碎片的产生，但是多次分配和释放可能会导致过多的外部碎片，由于该算法是基于按页分配的内存页，所以只会造成局部的碎片化，不会对全局的堆空间产生影响，当该页所有内存释放完毕，该页会返回全局的堆空间，是一个可逆的过程。系统提供一个额外的内存池算法，如果用户想自己构建自己的内存池，可以选用该方法。
## 3. 任务管理
系统为实时任务提供了实时调度器，为分时任务提供了分时调度器，实时任务的优先级要高于分时任务的优先级，实时任务有64个优先级，范围为0到63,数字越低优先级越高，分时任务提供了40个优先级，范围为0到39，数字越低优先级越高，不管是实时任务还是分时任务都支持抢占，实时任务永远是最高优先级的获得CPU控制权，分时任务的优先级则决定了任务获得运行的时间。
## 4. 消息队列
系统提供了消息队列用于任务间的通信，其中发送消息可以在中断中使用
## 5. 信号量
系统提供了信号量用于任务间的同步，释放信号可以在中断中使用
## 6. 互斥量
系统提供了互斥量来解决任务间的互斥问题，互斥量不能保护中断的临界段
## 7. 虚拟文件系统
虚拟文件系统并不是真正的文件系统，它的作用是统一文件系统操作接口以及通过挂载在同一个目录树的方式去操作多个文件系统以及存储设备。