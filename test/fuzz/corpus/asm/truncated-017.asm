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
