        # dyld enters an LC_MAIN executable with main's arguments in registers:
        #   x0 = argc, x1 = argv, x2 = envp
        adrp x11, _rt_argc
        str x0, [x11, :lo12:_rt_argc]
        adrp x11, _rt_argv
        str x1, [x11, :lo12:_rt_argv]
        adrp x11, _rt_envp
        str x2, [x11, :lo12:_rt_envp]

        # the stack dyld hands the LC_MAIN entry is already 16-byte aligned (an
        # arm64 architectural invariant), so no realignment is needed.
        bl _rt_init

        adrp x11, _rt_argc
        ldr x0, [x11, :lo12:_rt_argc]
        adrp x11, _rt_argv
        ldr x1, [x11, :lo12:_rt_argv]
        bl main

        mov x16, 1           # SYS_exit
        svc 0x80
        mov x9, sp            # x9 = initial stack pointer (argc at [x9])
        ldr x0, [x9]          # argc -> x0 (1st argument to main)
        add x1, x9, 8         # argv -> x1 (2nd argument to main)

        # envp = argv + (argc + 1) * 8
        add x10, x0, 1
        lsl x10, x10, 3
        add x10, x1, x10      # x10 = envp

        adrp x11, _rt_argc
        str x0, [x11, :lo12:_rt_argc]
        adrp x11, _rt_argv
        str x1, [x11, :lo12:_rt_argv]
        adrp x11, _rt_envp
        str x10, [x11, :lo12:_rt_envp]

        # sp is 16-byte aligned on kernel entry (AAPCS64 invariant), so no
        # realignment is needed before the calls.
        bl _rt_init

        adrp x11, _rt_argc
        ldr x0, [x11, :lo12:_rt_argc]
        adrp x11, _rt_argv
        ldr x1, [x11, :lo12:_rt_argv]
        bl main

        # exit_group with main's return value (already in x0)
        mov x8, 94           # SYS_exit_group
        svc 0
            ldr x12, {ptr}
            ldar x9, [x12]
            str x9, {result}
            ldr x12, {ptr}
            ldr x9, {v}
            stlr x9, [x12]
            ldr x9, {exp}
            ldr x12, {ptr}
            ldr x13, {desired}
        1:
            ldaxr x10, [x12]
            cmp x10, x9
            b.ne 2f
            stlxr w11, x13, [x12]
            cbnz w11, 1b
        2:
            str x10, {old}
            dmb ish
            ldr x12, {ptr}
            ldr x14, {v}
        1:
            ldaxr x9, [x12]
            add x10, x9, x14
            stlxr w11, x10, [x12]
            cbnz w11, 1b
            str x9, {old}
            dmb ish
            ldr x12, {ptr}
            ldr x14, {v}
        1:
            ldaxr x9, [x12]
            sub x10, x9, x14
            stlxr w11, x10, [x12]
            cbnz w11, 1b
            str x9, {old}
            dmb ish
            ldr x12, {ptr}
            ldr x14, {v}
        1:
            ldaxr x9, [x12]
            stlxr w11, x14, [x12]
            cbnz w11, 1b
            str x9, {old}
            dmb ish
            dmb ish
            yield
            ldr x16, {n}
            svc 0x80
            cset x9, cs
            str x9, {fail}
            str x0, {out}
            ldr x16, {n}
            ldr x0, {a0}
            svc 0x80
            cset x9, cs
            str x9, {fail}
            str x0, {out}
            ldr x16, {n}
            ldr x0, {a0}
            ldr x1, {a1}
            svc 0x80
            cset x9, cs
            str x9, {fail}
            str x0, {out}
            ldr x16, {n}
            ldr x0, {a0}
            ldr x1, {a1}
            ldr x2, {a2}
            svc 0x80
            cset x9, cs
            str x9, {fail}
            str x0, {out}
            ldr x16, {n}
            ldr x0, {a0}
            ldr x1, {a1}
            ldr x2, {a2}
            ldr x3, {a3}
            svc 0x80
            cset x9, cs
            str x9, {fail}
            str x0, {out}
            ldr x16, {n}
            ldr x0, {a0}
            ldr x1, {a1}
            ldr x2, {a2}
            ldr x3, {a3}
            ldr x4, {a4}
            svc 0x80
            cset x9, cs
            str x9, {fail}
            str x0, {out}
            ldr x16, {n}
            ldr x0, {a0}
            ldr x1, {a1}
            ldr x2, {a2}
            ldr x3, {a3}
            ldr x4, {a4}
            ldr x5, {a5}
            svc 0x80
            cset x9, cs
            str x9, {fail}
            str x0, {out}
            ldr x16, {n}
            svc 0x80
            cset x9, cs
            str x9, {fail}
            str x0, {out}
            str x1, {child}
            ldr x16, {n}
            svc 0x80
            cset x9, cs
            str x9, {fail}
            str x0, {out}
            str x1, {child}
        ldr x0, [x29, 16]
        str x0, {fn_addr}
        ldr x0, [x29, 24]
        str x0, {arg_addr}
        ldr x0, [x29, 32]
        str x0, {state_addr}
        ldr x0, [x29, 40]
        str x0, {reclaimable_addr}
        ldr x0, [x29, 48]
        str x0, {owner_base}
        ldr x0, [x29, 56]
        str x0, {owner_size}
        ldr x0, [x29, 64]
        str x0, {stack_base}
        ldr x0, [x29, 72]
        str x0, {stack_size}
        ldr x0, [x29, 80]
        str x0, {name_addr}
        add x0, x29, 88
        str x0, {setup_addr}
        add x0, x29, 96
        str x0, {run_addr}
        add x0, x29, 104
        str x0, {setup_error_addr}
        ldr x19, {stack_base}
        ldr x20, {stack_size}
        ldr x21, {reclaimable_addr}
        ldr x22, {owner_base}
        ldr x23, {owner_size}
        ldr x24, {detached}

        mov x0, x19
        mov x1, x20
        mov x8, 215           # SYS_munmap
        svc 0

        cbnz x24, 1f
        mov x0, 1
        str x0, [x21]
        mov x0, x21
        mov x1, 1             # FUTEX_WAKE
        mov x2, 1
        mov x3, 0
        mov x4, 0
        mov x5, 0
        mov x8, 98            # SYS_futex
        svc 0
        b 2f
    1:
        mov x0, x22
        mov x1, x23
        mov x8, 215           # SYS_munmap
        svc 0
    2:
        mov x0, 0
        mov x8, 93            # SYS_exit
        svc 0
        ldr x0, {flags}
        ldr x1, {stack_ptr}
        mov x2, 0
        mov x3, 0
        mov x4, 0
        mov x8, 220           # SYS_clone
        svc 0
        cbnz x0, 1f           # parent (tid != 0): skip the child dispatch
        b _std_thread_trampoline   # child (x0 == 0): branch to trampoline (no return)
    1:
        str x0, {result}      # parent: tid, or negative errno on failure
            ldr x8, {n}
            svc 0
            str x0, {out}
            ldr x8, {n}
            ldr x0, {a0}
            svc 0
            str x0, {out}
            ldr x8, {n}
            ldr x0, {a0}
            ldr x1, {a1}
            svc 0
            str x0, {out}
            ldr x8, {n}
            ldr x0, {a0}
            ldr x1, {a1}
            ldr x2, {a2}
            svc 0
            str x0, {out}
            ldr x8, {n}
            ldr x0, {a0}
            ldr x1, {a1}
            ldr x2, {a2}
            ldr x3, {a3}
            svc 0
            str x0, {out}
            ldr x8, {n}
            ldr x0, {a0}
            ldr x1, {a1}
            ldr x2, {a2}
            ldr x3, {a3}
            ldr x4, {a4}
            svc 0
            str x0, {out}
            ldr x8, {n}
            ldr x0, {a0}
            ldr x1, {a1}
            ldr x2, {a2}
            ldr x3, {a3}
            ldr x4, {a4}
            ldr x5, {a5}
            svc 0
            str x0, {out}
            ldr x0, [x29, 16]
            str x0, {ctx_addr}
            ldr x0, {flags}
            ldr x1, {stack_ptr}
            mov x2, 0
            mov x3, 0
            mov x4, 0
            mov x8, 220           # SYS_clone
            svc 0
            cbnz x0, 1f           # parent (pid != 0): skip the child dispatch
            b _std_spawn_trampoline    # child (x0 == 0): branch to trampoline (no return)
        1:
            str x0, {result}      # parent: pid, or negative errno on failure
                ldr x0, {data}
                ldr x1, {i}
                add x0, x0, x1
                strb wzr, [x0]
            ldr x0, {data}
            ldr x1, {alignment}
            sub x1, x1, 1
            and x0, x0, x1
            str x0, {remainder}
            ldr x0, {base}
            ldr x1, {alignment}
            ldr x2, {header}
            add x3, x0, x2
            sub x4, x1, 1
            add x3, x3, x4
            sub x4, xzr, x1
            and x3, x3, x4
            sub x4, x3, x2
            str x0, [x4]
            ldr x5, {total}
            str x5, [x4, 8]
            ldr x5, {count}
            str x5, [x4, 16]
            str x3, {out}
            ldr x0, {data}
            ldr x1, {header}
            sub x0, x0, x1
            ldr x1, [x0]
            str x1, {base}
            ldr x1, [x0, 8]
            str x1, {total}
            ldr x1, [x0, 16]
            str x1, {count}
                ldr x8, {syscall_number}
                mov x0, 0
                ldr x1, {size}
                ldr x2, {protection}
                ldr x3, {flags}
                ldr x4, {descriptor}
                mov x5, 0
                svc 0
                cmp x0, 0
                b.ge 1f
                mov x0, 0
                1:
                str x0, {out}
                ldr x8, {syscall_number}
                mov x0, 0
                ldr x1, {size}
                ldr x2, {protection}
                ldr x3, {flags}
                ldr x4, {descriptor}
                mov x5, 0
                svc 0
                cmp x0, 0
                b.ge 1f
                mov x0, 0
                1:
                str x0, {out}
                ldr x8, {syscall_number}
                ldr x0, {data}
                ldr x1, {size}
                svc 0
                str x0, {out}
                ldr x8, {syscall_number}
                ldr x0, {base}
                ldr x1, {size}
                svc 0
                str x0, {out}
                ldr x8, {syscall_number}
                ldr x0, {data}
                ldr x1, {len}
                mov x2, 0
                svc 0
                str x0, {out}
                mov x8, 64    # SYS_write (linux aarch64)
                mov x0, 2     # fd = stderr
                ldr x1, {msg}
                ldr x2, {len}
                svc 0
                mov x8, 94    # SYS_exit_group
                mov x0, 255   # PANIC_EXIT
                svc 0
                brk 0
                mov x16, 4    # SYS_write (darwin aarch64)
                mov x0, 2     # fd = stderr
                ldr x1, {msg}
                ldr x2, {len}
                svc 0x80
                mov x16, 1    # SYS_exit (darwin aarch64)
                mov x0, 255   # PANIC_EXIT
                svc 0x80
                brk 0