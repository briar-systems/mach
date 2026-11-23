[mach](../README.md) / [package](README.md) / **scratch**

# mach.package.scratch

scratch: the one place the dependency manager makes temporary storage (a git output capture,
the bare repository release candidates are fetched into). a process has one scratch
directory under the system temporary directory, `mach_scratch_<16 hex digits>`, made at the
first request, and every request is a fresh name inside it. beside it is its lock file, the
same name with `.lock`, made before the directory and locked by the process for the
scratch's whole life; the system releases the lock when the process ends however it ends.

the scratch is released by `end` when the command ends. while it exists the process holds
the interrupt (mach.host.interrupt), so an interrupt or termination is caught, the work in
hand stops, and the scratch is removed before the process ends. a process that is killed
leaves its scratch, and the next process to make one removes every scratch whose lock it
can take: no live process holds that lock, so its owner is gone. a name is reclaimed only
under its lock and only once the lock file the path names is the file locked, so a scratch
being made (whose lock is not yet taken) is never mistaken for an abandoned one, and
nothing that is not a lock file of this exact form, or the directory it names, is touched

Source: [`src/package/scratch.mach`](../../../src/package/scratch.mach)

Uses types from [`mach.base.fail`](../base/fail.md).

| Functions | |
| :--- | :--- |
| [`path`](#path) | a fresh path inside the process's scratch, `<role>_<n>`, made with the scratch on the first request; nothing is created at the path |
| [`end`](#end) | remove the process's scratch and release its lock and its hold on the interrupt; nothing when no scratch was made |

---

## Functions

### path

```mach
pub fun path(a: *std.allocator.Allocator, role: str) res[str, mach.base.fail.Fail]
```

a fresh path inside the process's scratch, `<role>_<n>`, made with the scratch on the
first request; nothing is created at the path

| Parameter | |
| :--- | :--- |
| `a` | owns the returned path |
| `role` | the leading word of the name |

### end

```mach
pub fun end(a: *std.allocator.Allocator) err[mach.base.fail.Fail]
```

remove the process's scratch and release its lock and its hold on the interrupt; nothing
when no scratch was made

| Parameter | |
| :--- | :--- |
| **returns** | the first refusal, once every step has run |

