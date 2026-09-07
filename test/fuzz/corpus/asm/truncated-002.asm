        mov x9, sp            # x9 = initial stack pointer (argc at [x9])
        ldr x0, [x9]          # argc -> x0 (1st argument to main)
        add x1, x9, 8         # argv -> x1 (2nd argument to main)
