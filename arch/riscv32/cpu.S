#include "soc.h"
#define REGBYTES 4
.global get_cpu
get_cpu:
    csrr a0, mhartid
    ret
.global local_irq_save
local_irq_save:
    csrrci t0, mstatus, 0x08
    and a0, t0, 0x08
    ret
.global local_irq_restore
local_irq_restore:
    csrs mstatus, a0
    ret
.global cpu_initialize_stack
cpu_initialize_stack:
    lw t0, 0(a0)
    la t1, sys_task_exit
    sw t1, -1 * REGBYTES(t0)
    sw zero, -2 * REGBYTES(t0)
#    sw gp, -2 * REGBYTES(t0)
#    sw tp, -3 * REGBYTES(t0)
    sw zero, -4 * REGBYTES(t0)
    sw zero, -5 * REGBYTES(t0)
    sw zero, -6 * REGBYTES(t0)
#    sw zero, -7 * REGBYTES(t0)
    sw zero, -8 * REGBYTES(t0)
    sw a2, -9 * REGBYTES(t0)
    sw zero, -10 * REGBYTES(t0)
    sw zero, -11 * REGBYTES(t0)
    sw zero, -12 * REGBYTES(t0)
    sw zero, -13 * REGBYTES(t0)
    sw zero, -14 * REGBYTES(t0)
    sw zero, -15 * REGBYTES(t0)
    sw zero, -16 * REGBYTES(t0)
    sw zero, -17 * REGBYTES(t0)
    sw zero, -18 * REGBYTES(t0)
    sw zero, -19 * REGBYTES(t0)
    sw zero, -20 * REGBYTES(t0)
    sw zero, -21 * REGBYTES(t0)
    sw zero, -22 * REGBYTES(t0)
    sw zero, -23 * REGBYTES(t0)
    sw zero, -24 * REGBYTES(t0)
    sw zero, -25 * REGBYTES(t0)
    sw zero, -26 * REGBYTES(t0)
    sw zero, -27 * REGBYTES(t0)
    sw zero, -28 * REGBYTES(t0)
    sw zero, -29 * REGBYTES(t0)
    sw zero, -30 * REGBYTES(t0)
    sw a1, -31 * REGBYTES(t0)
    csrr t1, mstatus
    ori t1, t1, 0x188
    slli t1, t1, 4
    sw t1, -32 * REGBYTES(t0)
    addi t0, t0, -32 * REGBYTES
    sw t0, 0(a0)
    ret
    
.global cpu_start_thread
cpu_start_thread:
    mv tp, a1                   //tp = &cpu_t 
    sw a0, REGBYTES(tp)         //cpu_t->next = stack_top
#设置定时器比较器
    li t0, MTIMECMP_BASE
    li t1, MTIMECMP
    sw t1, 0(t0)
#开启中断
    li t0, 0x0888
    csrs mie, t0
1:
    j 1b
.global cpu_yield
cpu_yield:
    sw a0, REGBYTES(tp)        //cpu_t->next = stack_top
    li t0, MSIP_BASE           //触发软中断
    li t1, 0x01
    sw t1, 0(t0)
    ret
.global machine_software_interrupt
machine_software_interrupt:
    li t0, MSIP_BASE           //清除标记
    sw zero, 0(t0)
    lw t0, 0(tp)
    lw t1, REGBYTES(tp)
    bne t0, t1, 1f
    ret
1:
    beqz t0, 2f
    sw sp, 0(t0)
2:
    sw t1, 0(tp)
    lw sp, 0(t1)
    sw ra, -1 * REGBYTES(sp)
    addi sp, sp, -1 * REGBYTES
    mv a0, t1
    jal ra, cpu_task_switch_hook
    addi sp, sp, 1 * REGBYTES
    lw ra, -1 * REGBYTES(sp)
    ret
.global machine_timer_interrupt
machine_timer_interrupt:
    li t0, MTIME_BASE
    sw zero, 0(t0)
    lw a0, 2 * REGBYTES(tp)        //a0 = cpu_t->sys_cpu_thread_t
    li a1, SYS_TIMER_CYCLE_NS
    li a2, 0
    sw ra, -1 * REGBYTES(sp)
    addi sp, sp, -1 * REGBYTES
    jal ra, sys_cpu_thread_tick
    addi sp, sp, 1 * REGBYTES
    lw ra, -1 * REGBYTES(sp)
    ret
.global sys_backtrace
sys_backtrace:
    sw ra, -1 * REGBYTES(sp)
    sw fp, -2 * REGBYTES(sp)
    sw a2, -3 * REGBYTES(sp)
    addi sp, sp, -3 * REGBYTES
    addi a2, sp, 3 * REGBYTES
    jal ra, _backtrace
    addi sp, sp, 3 * REGBYTES
    lw ra, -1 * REGBYTES(sp)
    lw fp, -2 * REGBYTES(sp)
    lw a2, -3 * REGBYTES(sp)
    ret

