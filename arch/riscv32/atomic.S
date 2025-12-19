.globl arch_atomic_store
arch_atomic_store:
    fence
    sw a1, 0(a0)
    fence
    ret

.globl arch_atomic_load
arch_atomic_load:
    fence
    lw a0, 0(a0)
    fence
    ret

.globl arch_atomic_exchange
arch_atomic_exchange:
    fence
    amoswap.w a0, a1, 0(a0)
    fence
    ret

.globl arch_atomic_compare_exchange_weak
arch_atomic_compare_exchange_weak:
    fence
    lr.w t0, 0(a0)
    lw t1, 0(a1)
    beq t0, t1, 1f
    sw t0, 0(a1)
    li a0, 0
    fence
    ret
1:
    sc.w t2, a2, 0(a0)
    beqz t2, 2f
    li a0, 0
    fence
    ret
2:
    li a0, 1
    fence
    ret
.globl arch_atomic_fetch_add
arch_atomic_fetch_add:
    fence
    amoadd.w a0, a1, 0(a0)
    fence
    ret

.globl arch_atomic_fetch_and
arch_atomic_fetch_and:
    fence
    amoand.w a0, a1, 0(a0)
    fence
    ret

.globl arch_atomic_fetch_or
arch_atomic_fetch_or:
    fence
    amoor.w a0, a1, 0(a0)
    fence
    ret

.globl arch_atomic_fetch_xor
arch_atomic_fetch_xor:
    fence
    amoxor.w a0, a1, 0(a0)
    fence
    ret