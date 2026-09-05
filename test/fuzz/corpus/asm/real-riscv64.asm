        mv t0, sp            # t0 = initial stack pointer (argc at [t0])
        ld a0, 0(t0)         # argc -> a0 (1st argument to main)
        addi a1, t0, 8       # argv -> a1 (2nd argument to main)

        # envp = argv + (argc + 1) * 8
        addi t1, a0, 1
        slli t1, t1, 3
        add t1, a1, t1       # t1 = envp

        la t2, _rt_argc
        sd a0, 0(t2)
        la t2, _rt_argv
        sd a1, 0(t2)
        la t2, _rt_envp
        sd t1, 0(t2)

        call _rt_init

        la t2, _rt_argc
        ld a0, 0(t2)
        la t2, _rt_argv
        ld a1, 0(t2)
        call main

        # exit_group with main's return value (already in a0)
        li a7, 94            # SYS_exit_group
        ecall
            ld a0, {ptr}
            fence
            ld a1, 0(a0)
            fence
            sd a1, {result}
            ld a0, {ptr}
            ld a1, {v}
            amoswap.d.aqrl zero, a1, (a0)
            ld a0, {ptr}
            ld a1, {exp}
            ld a2, {desired}
        1:
            lr.d.aqrl a3, (a0)
            bne a3, a1, 2f
            sc.d.aqrl a4, a2, (a0)
            bnez a4, 1b
        2:
            sd a3, {old}
            ld a0, {ptr}
            ld a1, {v}
            amoadd.d.aqrl a2, a1, (a0)
            sd a2, {old}
            ld a0, {ptr}
            ld a1, {v}
            neg a1, a1
            amoadd.d.aqrl a2, a1, (a0)
            sd a2, {old}
            ld a0, {ptr}
            ld a1, {v}
            amoswap.d.aqrl a2, a1, (a0)
            sd a2, {old}
            fence
            pause
        ld a0, 0(s0)
        sd a0, {fn_addr}
        ld a0, 8(s0)
        sd a0, {arg_addr}
        ld a0, 16(s0)
        sd a0, {state_addr}
        ld a0, 24(s0)
        sd a0, {reclaimable_addr}
        ld a0, 32(s0)
        sd a0, {owner_base}
        ld a0, 40(s0)
        sd a0, {owner_size}
        ld a0, 48(s0)
        sd a0, {stack_base}
        ld a0, 56(s0)
        sd a0, {stack_size}
        ld a0, 64(s0)
        sd a0, {name_addr}
        addi a0, s0, 72
        sd a0, {setup_addr}
        addi a0, s0, 80
        sd a0, {run_addr}
        addi a0, s0, 88
        sd a0, {setup_error_addr}
        ld s1, {stack_base}
        ld s2, {stack_size}
        ld s3, {reclaimable_addr}
        ld s4, {owner_base}
        ld s5, {owner_size}
        ld s6, {detached}

        mv a0, s1
        mv a1, s2
        li a7, 215           # SYS_munmap
        ecall

        bnez s6, 1f
        li a0, 1
        sd a0, 0(s3)
        mv a0, s3
        li a1, 1            # FUTEX_WAKE
        li a2, 1
        li a3, 0
        li a4, 0
        li a5, 0
        li a7, 98           # SYS_futex
        ecall
        j 2f
    1:
        mv a0, s4
        mv a1, s5
        li a7, 215          # SYS_munmap
        ecall
    2:
        li a0, 0
        li a7, 93            # SYS_exit
        ecall
        ld a0, {flags}
        ld a1, {stack_ptr}
        li a2, 0
        li a3, 0
        li a4, 0
        li a7, 220           # SYS_clone
        ecall
        bnez a0, 1f          # parent (tid != 0): skip the child dispatch
        tail _std_thread_trampoline   # child (a0 == 0): jump to trampoline (no return)
    1:
        sd a0, {result}      # parent: tid, or negative errno on failure
            ld a7, {n}
            ecall
            sd a0, {out}
            ld a7, {n}
            ld a0, {a0}
            ecall
            sd a0, {out}
            ld a7, {n}
            ld a0, {a0}
            ld a1, {a1}
            ecall
            sd a0, {out}
            ld a7, {n}
            ld a0, {a0}
            ld a1, {a1}
            ld a2, {a2}
            ecall
            sd a0, {out}
            ld a7, {n}
            ld a0, {a0}
            ld a1, {a1}
            ld a2, {a2}
            ld a3, {a3}
            ecall
            sd a0, {out}
            ld a7, {n}
            ld a0, {a0}
            ld a1, {a1}
            ld a2, {a2}
            ld a3, {a3}
            ld a4, {a4}
            ecall
            sd a0, {out}
            ld a7, {n}
            ld a0, {a0}
            ld a1, {a1}
            ld a2, {a2}
            ld a3, {a3}
            ld a4, {a4}
            ld a5, {a5}
            ecall
            sd a0, {out}
            ld a0, 0(s0)
            sd a0, {ctx_addr}
            ld a0, {flags}
            ld a1, {stack_ptr}
            li a2, 0
            li a3, 0
            li a4, 0
            li a7, 220           # SYS_clone
            ecall
            bnez a0, 1f          # parent (pid != 0): skip the child dispatch
            tail _std_spawn_trampoline    # child (a0 == 0): jump to trampoline (no return)
        1:
            sd a0, {result}      # parent: pid, or negative errno on failure
                ld t0, {data}
                ld t1, {i}
                add t0, t0, t1
                sb zero, 0(t0)
            ld t0, {data}
            ld t1, {alignment}
            addi t1, t1, -1
            and t0, t0, t1
            sd t0, {remainder}
            ld t0, {base}
            ld t1, {alignment}
            ld t2, {header}
            add t3, t0, t2
            addi t4, t1, -1
            add t3, t3, t4
            sub t4, zero, t1
            and t3, t3, t4
            sub t4, t3, t2
            sd t0, 0(t4)
            ld t5, {total}
            sd t5, 8(t4)
            ld t5, {count}
            sd t5, 16(t4)
            sd t3, {out}
            ld t0, {data}
            ld t1, {header}
            sub t0, t0, t1
            ld t1, 0(t0)
            sd t1, {base}
            ld t1, 8(t0)
            sd t1, {total}
            ld t1, 16(t0)
            sd t1, {count}
                ld a7, {syscall_number}
                li a0, 0
                ld a1, {size}
                ld a2, {protection}
                ld a3, {flags}
                ld a4, {descriptor}
                li a5, 0
                ecall
                bgez a0, 1f
                li a0, 0
                1:
                sd a0, {out}
                ld a7, {syscall_number}
                li a0, 0
                ld a1, {size}
                ld a2, {protection}
                ld a3, {flags}
                ld a4, {descriptor}
                li a5, 0
                ecall
                bgez a0, 1f
                li a0, 0
                1:
                sd a0, {out}
                ld a7, {syscall_number}
                ld a0, {data}
                ld a1, {size}
                ecall
                sd a0, {out}
                ld a7, {syscall_number}
                ld a0, {base}
                ld a1, {size}
                ecall
                sd a0, {out}
                ld a7, {syscall_number}
                ld a0, {data}
                ld a1, {len}
                li a2, 0
                ecall
                sd a0, {out}
                li a7, 64     # SYS_write (linux riscv64)
                li a0, 2      # fd = stderr
                ld a1, {msg}
                ld a2, {len}
                ecall
                li a7, 94     # SYS_exit_group
                li a0, 255    # PANIC_EXIT
                ecall
                ebreak