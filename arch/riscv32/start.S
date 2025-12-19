#include "soc.h"
#define RISCV_PGSHIFT 16
#define RISCV_PGSIZE (1 << RISCV_PGSHIFT)
#define REGBYTES 4
trap_exception_table:
    .dc.a instruction_address_misaligned               //0
    .dc.a instruction_access_fault                     //1
    .dc.a illegal_instruction                          //2
    .dc.a breakpoint                                   //3
    .dc.a load_address_misaligned                      //4
    .dc.a load_access_fault                            //5
    .dc.a store_address_misaligned                     //6
    .dc.a store_access_fault                           //7
    .dc.a env_call_from_umode                          //8
    .dc.a env_call_from_smode                          //9
    .dc.a bad_trap                                     //10
    .dc.a env_call_from_mmode                          //11
    .dc.a illegal_page_fault                           //12
    .dc.a load_page_fault                              //13
    .dc.a bad_trap                                     //14
    .dc.a store_page_fault                             //15
trap_interrupt_table:
    .dc.a bad_trap                                     //0
    .dc.a supervisor_software_interrupt                //1
    .dc.a bad_trap                                     //2
    .dc.a machine_software_interrupt                   //3
    .dc.a bad_trap                                     //4
    .dc.a supervisor_timer_interrupt                   //5
    .dc.a bad_trap                                     //6
    .dc.a machine_timer_interrupt                      //7
    .dc.a bad_trap                                     //8
    .dc.a supervisor_external_interrupt                //9
    .dc.a bad_trap                                     //10
    .dc.a machine_external_interrupt                   //11
.globl do_reset
do_reset:
#初始化通用寄存器
    li x1, 0
    li x2, 0
    li x3, 0
    li x4, 0
    li x5, 0
    li x6, 0
    li x7, 0
    li x8, 0
    li x9, 0
    li x10, 0
    li x11, 0
    li x12, 0
    li x13, 0
    li x14, 0
    li x15, 0
    li x16, 0
    li x17, 0
    li x18, 0
    li x19, 0
    li x20, 0
    li x21, 0
    li x22, 0
    li x23, 0
    li x24, 0
    li x25, 0
    li x26, 0
    li x27, 0
    li x28, 0
    li x29, 0
    li x30, 0
    li x31, 0
    csrw mscratch, zero
#设置mtvec寄存器
    la t0, trap_vector
    csrw mtvec, t0
    csrr t1, mtvec
1:
    bne t0, t1, 1b
#设置堆栈指针
    la sp, _fuser_stack + RISCV_PGSIZE
    csrr t0, mhartid
    slli t1, t0, RISCV_PGSHIFT
    add sp, sp, t1
#清空bss段
    la t0, _fbss
    la t1, _ebss
    beq t0, t1, 2f
1:
    sw zero, 0(t0)
    addi t0, t0, REGBYTES
    bne t0, t1, 1b
2:
#开启全局中断
    li t0, 0x08
    csrs mstatus, t0
    jal ra, soc_init
    j _start
#恢复寄存器   
restore_regs:
    addi sp, sp, 32 * REGBYTES
    lw t0, -31 * REGBYTES(sp)
    csrw mepc, t0
    lw t0, -32 * REGBYTES(sp)
    csrw mstatus, t0
    lw ra, -1 * REGBYTES(sp)
    lw fp, -2 * REGBYTES(sp)
#    lw gp, -2 * REGBYTES(sp)
#    lw tp, -3 * REGBYTES(sp)
    lw t0, -4 * REGBYTES(sp)
    lw t1, -5 * REGBYTES(sp)
    lw t2, -6 * REGBYTES(sp)
#    lw fp, -7 * REGBYTES(sp)
    lw s1, -8 * REGBYTES(sp)
    lw a0, -9 * REGBYTES(sp)
    lw a1, -10 * REGBYTES(sp)
    lw a2, -11 * REGBYTES(sp)
    lw a3, -12 * REGBYTES(sp)
    lw a4, -13 * REGBYTES(sp)
    lw a5, -14 * REGBYTES(sp)
    lw a6, -15 * REGBYTES(sp)
    lw a7, -16 * REGBYTES(sp)
    lw s2, -17 * REGBYTES(sp)
    lw s3, -18 * REGBYTES(sp)
    lw s4, -19 * REGBYTES(sp)
    lw s5, -20 * REGBYTES(sp)
    lw s6, -21 * REGBYTES(sp)
    lw s7, -22 * REGBYTES(sp)
    lw s8, -23 * REGBYTES(sp)
    lw s9, -24 * REGBYTES(sp)
    lw s10, -25 * REGBYTES(sp)
    lw s11, -26 * REGBYTES(sp)
    lw t3, -27 * REGBYTES(sp)
    lw t4, -28 * REGBYTES(sp)
    lw t5, -29 * REGBYTES(sp)
    lw t6, -30 * REGBYTES(sp)
    mret
.align 4
trap_vector:
#保存寄存器
    sw ra, -1 * REGBYTES(sp)
    sw fp, -2 * REGBYTES(sp)
    mv fp, sp
#    sw gp, -2 * REGBYTES(sp)
#    sw tp, -3 * REGBYTES(sp)
    sw t0, -4 * REGBYTES(sp)
    sw t1, -5 * REGBYTES(sp)
    sw t2, -6 * REGBYTES(sp)
#    sw fp, -7 * REGBYTES(sp)
    sw s1, -8 * REGBYTES(sp)
    sw a0, -9 * REGBYTES(sp)
    sw a1, -10 * REGBYTES(sp)
    sw a2, -11 * REGBYTES(sp)
    sw a3, -12 * REGBYTES(sp)
    sw a4, -13 * REGBYTES(sp)
    sw a5, -14 * REGBYTES(sp)
    sw a6, -15 * REGBYTES(sp)
    sw a7, -16 * REGBYTES(sp)
    sw s2, -17 * REGBYTES(sp)
    sw s3, -18 * REGBYTES(sp)
    sw s4, -19 * REGBYTES(sp)
    sw s5, -20 * REGBYTES(sp)
    sw s6, -21 * REGBYTES(sp)
    sw s7, -22 * REGBYTES(sp)
    sw s8, -23 * REGBYTES(sp)
    sw s9, -24 * REGBYTES(sp)
    sw s10, -25 * REGBYTES(sp)
    sw s11, -26 * REGBYTES(sp)
    sw t3, -27 * REGBYTES(sp)
    sw t4, -28 * REGBYTES(sp)
    sw t5, -29 * REGBYTES(sp)
    sw t6, -30 * REGBYTES(sp)
    csrr t0, mepc
    sw t0, -31 * REGBYTES(sp)
    csrr t0, mstatus
    sw t0, -32 * REGBYTES(sp)
    addi sp, sp, -32 * REGBYTES
#判断是异常还是中断
    csrr t0, mcause
    li t1, 0x80000000
    and t1, t0, t1
    beqz t1, 1f
#中断
    la t1, trap_interrupt_table
    slli t0, t0, 2
    add t1, t1, t0
    lw t1, 0(t1)
    j 2f
1:
#异常
    la t1, trap_exception_table
    slli t0, t0, 2
    add t1, t1, t0
    lw t1, 0(t1)
2:
    jalr ra, t1
    j restore_regs
