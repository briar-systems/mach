        mov x9, sp            # x9 = initial stack pointer (argc at [x9])
        ldr x0, [x9]          # argc -> x0 (1st argument to main)
