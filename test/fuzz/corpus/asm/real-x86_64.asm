            mov rax, 15
            syscall
            mov [_rt_argc], rdi    # argc
            mov [_rt_argv], rsi    # argv
            mov [_rt_envp], rdx    # envp

            and rsp, -16      # 16-align rsp for the SysV call boundary
            call _rt_init

            mov rdi, [_rt_argc]
            mov rsi, [_rt_argv]
            call main

            mov rdi, rax      # exit code from main -> rdi (1st syscall arg)
            mov rax, 0x2000001 # SYS_exit (BSD syscall class 2 + exit=1)
            syscall
            mov rax, rsp      # rax = initial stack pointer
            mov rdi, [rax]    # argc -> rdi (1st argument)
            lea rsi, [rax+8]  # argv -> rsi (2nd argument)

            # envp = argv + (argc + 1) * 8
            mov rcx, rdi
            inc rcx
            lea rcx, [rsi + rcx*8]

            mov [_rt_argc], rdi
            mov [_rt_argv], rsi
            mov [_rt_envp], rcx

            and rsp, -16      # 16-align rsp for the SysV call boundary
            call _rt_init

            mov rdi, [_rt_argc]
            mov rsi, [_rt_argv]
            call main

            mov rdi, rax      # exit code from main -> rdi (1st syscall arg)
            mov rax, 0x2000001 # SYS_exit (BSD syscall class 2 + exit=1)
            syscall
        mov rax, rsp      # rax = initial stack pointer
        mov rdi, [rax]    # argc -> rdi (1st argument)
        lea rsi, [rax+8]  # argv -> rsi (2nd argument)

        # envp = argv + (argc + 1) * 8
        mov rcx, rdi
        inc rcx
        lea rcx, [rsi + rcx*8]

        mov [_rt_argc], rdi
        mov [_rt_argv], rsi
        mov [_rt_envp], rcx

        and rsp, -16      # 16-align rsp for the SysV call boundary
        call _rt_init

        mov rdi, [_rt_argc]
        mov rsi, [_rt_argv]
        call main

        mov rdi, rax      # exit code from main -> rdi (1st syscall arg)
        mov rax, 231      # SYS_exit_group
        syscall
        and rsp, -16      # align stack to 16 bytes
        sub rsp, 32       # shadow space

        call GetCommandLineA

        mov rcx, rax      # cmdline -> 1st arg
        call parse_cmdline

        mov rcx, [argc]
        lea rdx, [argv_ptrs]
        call main

        mov ecx, eax      # exit code -> rcx (1st argument, Windows convention)
        call ExitProcess
            mov rcx, {ptr}
            mov rax, [rcx]
            mov {result}, rax
            mov rax, {v}
            mov rcx, {ptr}
            xchg [rcx], rax
            mov rax, {exp}
            mov rdx, {desired}
            mov rcx, {ptr}
            lock cmpxchg [rcx], rdx
            mov {old}, rax
            mov rax, {v}
            mov rcx, {ptr}
            lock xadd [rcx], rax
            mov {old}, rax
            mov rax, {v}
            neg rax
            mov rcx, {ptr}
            lock xadd [rcx], rax
            mov {old}, rax
            mov rax, {v}
            mov rcx, {ptr}
            xchg [rcx], rax
            mov {old}, rax
            mfence
            pause
            mov rax, 0x2000000
            or rax, {n}
            syscall
            mov rcx, 0
            setc cl
            mov {fail}, rcx
            mov {out}, rax
            mov rax, 0x2000000
            or rax, {n}
            mov rdi, {a0}
            syscall
            mov rcx, 0
            setc cl
            mov {fail}, rcx
            mov {out}, rax
            mov rax, 0x2000000
            or rax, {n}
            mov rdi, {a0}
            mov rsi, {a1}
            syscall
            mov rcx, 0
            setc cl
            mov {fail}, rcx
            mov {out}, rax
            mov rax, 0x2000000
            or rax, {n}
            mov rdi, {a0}
            mov rsi, {a1}
            mov rdx, {a2}
            syscall
            mov rcx, 0
            setc cl
            mov {fail}, rcx
            mov {out}, rax
            mov rax, 0x2000000
            or rax, {n}
            mov rdi, {a0}
            mov rsi, {a1}
            mov rdx, {a2}
            mov r10, {a3}
            syscall
            mov rcx, 0
            setc cl
            mov {fail}, rcx
            mov {out}, rax
            mov rax, 0x2000000
            or rax, {n}
            mov rdi, {a0}
            mov rsi, {a1}
            mov rdx, {a2}
            mov r10, {a3}
            mov r8, {a4}
            syscall
            mov rcx, 0
            setc cl
            mov {fail}, rcx
            mov {out}, rax
            mov rax, 0x2000000
            or rax, {n}
            mov rdi, {a0}
            mov rsi, {a1}
            mov rdx, {a2}
            mov r10, {a3}
            mov r8, {a4}
            mov r9, {a5}
            syscall
            mov rcx, 0
            setc cl
            mov {fail}, rcx
            mov {out}, rax
            mov rax, 0x2000000
            or rax, {n}
            syscall
            mov rcx, 0
            setc cl
            mov {fail}, rcx
            mov {out}, rax
            mov {child}, rdx
            mov rax, 0x2000000
            or rax, {n}
            syscall
            mov rcx, 0
            setc cl
            mov {fail}, rcx
            mov {out}, rax
            mov {child}, rdx
            mov rax, {n}
            syscall
            mov {out}, rax
            mov rax, {n}
            mov rdi, {a0}
            syscall
            mov {out}, rax
            mov rax, {n}
            mov rdi, {a0}
            mov rsi, {a1}
            syscall
            mov {out}, rax
            mov rax, {n}
            mov rdi, {a0}
            mov rsi, {a1}
            mov rdx, {a2}
            syscall
            mov {out}, rax
            mov rax, {n}
            mov rdi, {a0}
            mov rsi, {a1}
            mov rdx, {a2}
            mov r10, {a3}
            syscall
            mov {out}, rax
            mov rax, {n}
            mov rdi, {a0}
            mov rsi, {a1}
            mov rdx, {a2}
            mov r10, {a3}
            mov r8, {a4}
            syscall
            mov {out}, rax
            mov rax, {n}
            mov rdi, {a0}
            mov rsi, {a1}
            mov rdx, {a2}
            mov r10, {a3}
            mov r8, {a4}
            mov r9, {a5}
            syscall
            mov {out}, rax
            mov rax, [rbp+8]
            mov {ctx_addr}, rax
            mov rdi, {flags}
            mov rsi, {stack_ptr}
            xor rdx, rdx
            xor r10, r10
            xor r8, r8
            mov rax, 56           # SYS_clone
            syscall
            test rax, rax
            jz _std_spawn_trampoline
            mov {result}, rax     # parent: pid, or negative errno on failure
        mov rax, [rbp+8]
        mov {fn_addr}, rax
        mov rax, [rbp+16]
        mov {arg_addr}, rax
        mov rax, [rbp+24]
        mov {state_addr}, rax
        mov rax, [rbp+32]
        mov {reclaimable_addr}, rax
        mov rax, [rbp+40]
        mov {owner_base}, rax
        mov rax, [rbp+48]
        mov {owner_size}, rax
        mov rax, [rbp+56]
        mov {stack_base}, rax
        mov rax, [rbp+64]
        mov {stack_size}, rax
        mov rax, [rbp+72]
        mov {name_addr}, rax
        lea rax, [rbp+80]
        mov {setup_addr}, rax
        lea rax, [rbp+88]
        mov {run_addr}, rax
        lea rax, [rbp+96]
        mov {setup_error_addr}, rax
        mov r12, {stack_base}
        mov r13, {stack_size}
        mov r14, {reclaimable_addr}
        mov r15, {owner_base}
        mov rbx, {owner_size}
        mov r10, {detached}

        mov rdi, r12
        mov rsi, r13
        mov rax, 11           # SYS_munmap
        syscall

        test r10, r10
        jnz 1f
        mov qword [r14], 1
        mov rdi, r14
        mov rsi, 1            # FUTEX_WAKE
        mov rdx, 1
        xor r10, r10
        xor r8, r8
        xor r9, r9
        mov rax, 202          # SYS_futex
        syscall
        jmp 2f
    1:
        mov rdi, r15
        mov rsi, rbx
        mov rax, 11           # SYS_munmap
        syscall
    2:
        mov rdi, 0
        mov rax, 60           # SYS_exit
        syscall
        mov rdi, {flags}
        mov rsi, {stack_ptr}
        xor rdx, rdx
        xor r10, r10
        xor r8, r8
        mov rax, 56           # SYS_clone
        syscall
        test rax, rax
        jz _std_thread_trampoline
        mov {result}, rax     # parent: tid, or negative errno on failure
                mov rax, {data}
                mov rcx, {i}
                add rax, rcx
                mov byte [rax], 0
            mov rax, {data}
            mov rcx, {alignment}
            dec rcx
            and rax, rcx
            mov {remainder}, rax
            mov rax, {base}
            mov rcx, {alignment}
            mov rdx, {header}
            add rax, rdx
            mov r8, rcx
            dec r8
            add rax, r8
            neg rcx
            and rax, rcx
            mov r8, rax
            sub r8, rdx
            mov r9, {base}
            mov [r8], r9
            mov r9, {total}
            mov [r8 + 8], r9
            mov r9, {count}
            mov [r8 + 16], r9
            mov {out}, rax
            mov rax, {data}
            sub rax, {header}
            mov rcx, [rax]
            mov {base}, rcx
            mov rcx, [rax + 8]
            mov {total}, rcx
            mov rcx, [rax + 16]
            mov {count}, rcx
                mov rax, {syscall_number}
                xor rdi, rdi
                mov rsi, {size}
                mov rdx, {protection}
                mov r10, {flags}
                mov r8, {descriptor}
                xor r9, r9
                syscall
                cmp rax, {error_floor}
                jb 1f
                xor rax, rax
                1:
                mov {out}, rax
                mov rax, {syscall_number}
                xor rdi, rdi
                mov rsi, {size}
                mov rdx, {protection}
                mov r10, {flags}
                mov r8, {descriptor}
                xor r9, r9
                syscall
                cmp rax, {error_floor}
                jb 1f
                xor rax, rax
                1:
                mov {out}, rax
                mov rax, {syscall_number}
                mov rdi, {data}
                mov rsi, {size}
                syscall
                mov {out}, rax
                mov rax, {syscall_number}
                mov rdi, {base}
                mov rsi, {size}
                syscall
                mov {out}, rax
                mov rax, {syscall_number}
                mov rdi, {data}
                mov rsi, {len}
                xor rdx, rdx
                syscall
                mov {out}, rax
                mov eax, 1        # SYS_write
                mov edi, 2        # fd = stderr
                mov rsi, {msg}
                mov rdx, {len}
                syscall
                mov eax, 231      # SYS_exit_group
                mov edi, 255      # PANIC_EXIT
                syscall
                hlt
                mov eax, 0x2000004  # SYS_write (darwin)
                mov edi, 2          # fd = stderr
                mov rsi, {msg}
                mov rdx, {len}
                syscall
                mov eax, 0x2000001  # SYS_exit (darwin)
                mov edi, 255        # PANIC_EXIT
                syscall
                hlt