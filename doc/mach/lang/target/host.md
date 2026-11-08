# mach.lang.target.host

the host a target runs on

a target's host is mink's row for its system, under the environment the
target's manifest states and none where it states none, for its architecture,
held under the traits its calling convention states, as riscv's float abi
picks one of linux's riscv64 rows. what a host says of the target, the
capabilities it offers, the instruction set floor every machine it runs has,
its default convention and how it names libraries, is read from that row and
stated nowhere in mach

## fun refusal

```mach
pub fun refusal(a: *A.Allocator, f: mink_fail.Fail) fail.Fail;
```

## fun refused

```mach
pub fun refused(a: *A.Allocator, kind: diagnostic_kind.Kind, r: *mink_target.Refusal) fail.Fail;
```

mink's refusal of a part of a target as a user failure of `kind`, in mink's words

## fun of

```mach
pub fun of(a: *A.Allocator, system: *mink_os.System, env: opt[*mink_os.Env], arch: *mink_arch.Arch, abi: str) res[*mink_os.Host, fail.Fail];
```

mink's host of `system` running `arch` under `env`, held under the traits the
convention `abi` states, "" for a convention not yet chosen. refused where no
host of the system holds

## fun default_abi

```mach
pub fun default_abi(h: *mink_os.Host) opt[str];
```

the calling convention the host calls by default, none where the architecture's
family names none or the host derives it from an ISA string

## fun floor

```mach
pub fun floor(h: *mink_os.Host) opt[str];
```

the instruction set floor of the host, in masc's selection spelling, none where
the system bounds the machine not at all, as a freestanding one

