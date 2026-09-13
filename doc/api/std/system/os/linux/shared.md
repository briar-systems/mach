# std.system.os.linux.shared

## val SYS_FUTEX

```mach
pub val SYS_FUTEX:             usize = 202
```

## val SYS_FUTEX

```mach
pub val SYS_FUTEX:             usize = 98
```

## val SYS_FUTEX

```mach
pub val SYS_FUTEX:       usize = 98
```

## val FUTEX_WAIT

```mach
pub val FUTEX_WAIT:             usize = 0
```

constants identical across linux architectures (asm-generic matches x86_64
for these), declared once at module scope.

## val FUTEX_WAKE

```mach
pub val FUTEX_WAKE:             usize = 1
```

## val FUTEX_WAIT_BITSET

```mach
pub val FUTEX_WAIT_BITSET:      usize = 9
```

## val FUTEX_BITSET_MATCH_ANY

```mach
pub val FUTEX_BITSET_MATCH_ANY: usize = 0xffffffff
```

## val CLONE_VM

```mach
pub val CLONE_VM:      usize = 0x00000100
```

## val CLONE_FS

```mach
pub val CLONE_FS:      usize = 0x00000200
```

## val CLONE_FILES

```mach
pub val CLONE_FILES:   usize = 0x00000400
```

## val CLONE_SIGHAND

```mach
pub val CLONE_SIGHAND: usize = 0x00000800
```

## val CLONE_THREAD

```mach
pub val CLONE_THREAD:  usize = 0x00010000
```

## val CLONE_SYSVSEM

```mach
pub val CLONE_SYSVSEM: usize = 0x00040000
```

## val PROT_READ

```mach
pub val PROT_READ:  i32 = 1
```

the read-only protection, exported for the static-PIE RELRO re-protection in
std.runtime.linux.reloc.

## rec stat_t

```mach
pub rec stat_t;
```

## rec stat_t

```mach
pub rec stat_t;
```

## rec stat_t

```mach
pub rec stat_t;
```

## rec timespec

```mach
pub rec timespec;
```

time specification with seconds and nanoseconds

## rec DirectoryCursor

```mach
pub rec DirectoryCursor;
```

## val separator

```mach
pub val separator: u8 = '/'
```

path

## val STDIN_FD

```mach
pub val STDIN_FD:  i32 = 0
```

portable constants

## val STDOUT_FD

```mach
pub val STDOUT_FD: i32 = 1
```

## val STDERR_FD

```mach
pub val STDERR_FD: i32 = 2
```

## val O_RDONLY

```mach
pub val O_RDONLY:   i32 = 0
```

## val O_WRONLY

```mach
pub val O_WRONLY:   i32 = 1
```

## val O_RDWR

```mach
pub val O_RDWR:     i32 = 2
```

## val O_CREAT

```mach
pub val O_CREAT:    i32 = 64
```

## val O_EXCL

```mach
pub val O_EXCL:     i32 = 128
```

## val O_TRUNC

```mach
pub val O_TRUNC:    i32 = 512
```

## val O_APPEND

```mach
pub val O_APPEND:   i32 = 1024
```

## val O_NONBLOCK

```mach
pub val O_NONBLOCK: i32 = 0o4000
```

## val O_DIRECTORY

```mach
pub val O_DIRECTORY: i32 = 0o40000
```

## val O_NOFOLLOW

```mach
pub val O_NOFOLLOW:  i32 = 0o100000
```

## val O_DIRECT

```mach
pub val O_DIRECT:    i32 = 0o200000
```

## val O_DIRECTORY

```mach
pub val O_DIRECTORY: i32 = 0o200000
```

## val O_NOFOLLOW

```mach
pub val O_NOFOLLOW:  i32 = 0o400000
```

## val O_DIRECT

```mach
pub val O_DIRECT:    i32 = 0o40000
```

## val AT_FDCWD

```mach
pub val AT_FDCWD:            i32 = -100
```

## val AT_SYMLINK_NOFOLLOW

```mach
pub val AT_SYMLINK_NOFOLLOW: i32 = 0x0100
```

## val AT_REMOVEDIR

```mach
pub val AT_REMOVEDIR:        i32 = 0x0200
```

## val LOCK_SH

```mach
pub val LOCK_SH: i32 = 1
```

advisory whole-file lock operations, passed to lock_fd

## val LOCK_EX

```mach
pub val LOCK_EX: i32 = 2
```

## val LOCK_NB

```mach
pub val LOCK_NB: i32 = 4
```

## val LOCK_UN

```mach
pub val LOCK_UN: i32 = 8
```

## val S_IFMT

```mach
pub val S_IFMT:  u32 = 0o170000
```

## val S_IFDIR

```mach
pub val S_IFDIR: u32 = 0o040000
```

## val S_IFREG

```mach
pub val S_IFREG: u32 = 0o100000
```

## val S_IFLNK

```mach
pub val S_IFLNK: u32 = 0o120000
```

## val WNOHANG

```mach
pub val WNOHANG:    i32 = 1
```

## val WUNTRACED

```mach
pub val WUNTRACED:  i32 = 2
```

## val WCONTINUED

```mach
pub val WCONTINUED: i32 = 8
```

## val CLOCK_REALTIME

```mach
pub val CLOCK_REALTIME:  i32 = 0
```

## val CLOCK_MONOTONIC

```mach
pub val CLOCK_MONOTONIC: i32 = 1
```

## val EPERM

```mach
pub val EPERM:        i64 = -1
```

## val ENOENT

```mach
pub val ENOENT:       i64 = -2
```

## val ESRCH

```mach
pub val ESRCH:        i64 = -3
```

## val EINTR

```mach
pub val EINTR:        i64 = -4
```

## val EIO

```mach
pub val EIO:          i64 = -5
```

## val ENXIO

```mach
pub val ENXIO:        i64 = -6
```

## val E2BIG

```mach
pub val E2BIG:        i64 = -7
```

## val EBADF

```mach
pub val EBADF:        i64 = -9
```

## val ECHILD

```mach
pub val ECHILD:       i64 = -10
```

## val EAGAIN

```mach
pub val EAGAIN:       i64 = -11
```

## val ENOMEM

```mach
pub val ENOMEM:       i64 = -12
```

## val EACCES

```mach
pub val EACCES:       i64 = -13
```

## val EFAULT

```mach
pub val EFAULT:       i64 = -14
```

## val EBUSY

```mach
pub val EBUSY:        i64 = -16
```

## val EEXIST

```mach
pub val EEXIST:       i64 = -17
```

## val ENODEV

```mach
pub val ENODEV:       i64 = -19
```

## val ENOTDIR

```mach
pub val ENOTDIR:      i64 = -20
```

## val EISDIR

```mach
pub val EISDIR:       i64 = -21
```

## val EINVAL

```mach
pub val EINVAL:       i64 = -22
```

## val ENFILE

```mach
pub val ENFILE:       i64 = -23
```

## val EMFILE

```mach
pub val EMFILE:       i64 = -24
```

## val ETXTBSY

```mach
pub val ETXTBSY:      i64 = -26
```

## val ENOSPC

```mach
pub val ENOSPC:       i64 = -28
```

## val EROFS

```mach
pub val EROFS:        i64 = -30
```

## val EPIPE

```mach
pub val EPIPE:        i64 = -32
```

## val ENAMETOOLONG

```mach
pub val ENAMETOOLONG: i64 = -36
```

## val ENOTEMPTY

```mach
pub val ENOTEMPTY:    i64 = -39
```

## val ENOTSUP

```mach
pub val ENOTSUP:      i64 = -95
```

## val ERANGE

```mach
pub val ERANGE:       i64 = -34
```

## val EADDRINUSE

```mach
pub val EADDRINUSE:   i64 = -98
```

## val ENETUNREACH

```mach
pub val ENETUNREACH:  i64 = -101
```

## val ECONNABORTED

```mach
pub val ECONNABORTED: i64 = -103
```

## val ECONNRESET

```mach
pub val ECONNRESET:   i64 = -104
```

## val ENOTCONN

```mach
pub val ENOTCONN:     i64 = -107
```

## val ETIMEDOUT

```mach
pub val ETIMEDOUT:    i64 = -110
```

## val ECONNREFUSED

```mach
pub val ECONNREFUSED: i64 = -111
```

## val EHOSTUNREACH

```mach
pub val EHOSTUNREACH: i64 = -113
```

## val ECANCELED

```mach
pub val ECANCELED:    i64 = -125
```

## val EINTR_MAX_RETRIES

```mach
pub val EINTR_MAX_RETRIES: usize = 8
```

## fun syscall0

```mach
pub fun syscall0(n: usize) i64;
```

execute a syscall with no arguments

n: syscall number
ret: syscall result (negative on error)

## fun syscall1

```mach
pub fun syscall1(n: usize, a0: usize) i64;
```

execute a syscall with 1 argument

n: syscall number
a0: first argument
ret: syscall result (negative on error)

## fun syscall2

```mach
pub fun syscall2(n: usize, a0: usize, a1: usize) i64;
```

execute a syscall with 2 arguments

n: syscall number
a0: first argument
a1: second argument
ret: syscall result (negative on error)

## fun syscall3

```mach
pub fun syscall3(n: usize, a0: usize, a1: usize, a2: usize) i64;
```

execute a syscall with 3 arguments

n: syscall number
a0: first argument
a1: second argument
a2: third argument
ret: syscall result (negative on error)

## fun syscall4

```mach
pub fun syscall4(n: usize, a0: usize, a1: usize, a2: usize, a3: usize) i64;
```

execute a syscall with 4 arguments

n: syscall number
a0: first argument
a1: second argument
a2: third argument
a3: fourth argument
ret: syscall result (negative on error)

## fun syscall5

```mach
pub fun syscall5(n: usize, a0: usize, a1: usize, a2: usize, a3: usize, a4: usize) i64;
```

execute a syscall with 5 arguments

n: syscall number
a0: first argument
a1: second argument
a2: third argument
a3: fourth argument
a4: fifth argument
ret: syscall result (negative on error)

## fun syscall6

```mach
pub fun syscall6(n: usize, a0: usize, a1: usize, a2: usize, a3: usize, a4: usize, a5: usize) i64;
```

execute a syscall with 6 arguments

n: syscall number
a0: first argument
a1: second argument
a2: third argument
a3: fourth argument
a4: fifth argument
a5: sixth argument
ret: syscall result (negative on error)

## fun allocate

```mach
pub fun allocate(size: usize) ptr;
```

allocate anonymous read-write memory via mmap

size: number of bytes to allocate; if 0, returns nil
ret: pointer to allocated memory, or nil on failure

## fun deallocate

```mach
pub fun deallocate(p: ptr, size: usize) i64;
```

free memory via munmap

p: pointer to memory; if nil, returns 0
size: allocation size in bytes; if 0, returns 0
ret: 0 on success, or negative errno

## fun thread_stack_allocate

```mach
pub fun thread_stack_allocate(reserve: usize, commit: usize, base: *usize) i64;
```

allocate a writable stack mapping

reserve: complete virtual stack extent
commit: must be 0 until safe stack growth is implemented
base: receives the mapping base on success
ret: 0 on success, or normalized negative error

## fun thread_set_name

```mach
pub fun thread_set_name(name: *u8) i64;
```

set the current thread's native diagnostic name

## fun thread_current_id

```mach
pub fun thread_current_id() i64;
```

## fun thread_current_name

```mach
pub fun thread_current_name(name: *u8, cap: usize) i64;
```

## fun reallocate

```mach
pub fun reallocate(p: ptr, old_size: usize, new_size: usize) ptr;
```

resize a memory region via mremap

p: pointer to memory; if nil, behaves like allocate
old_size: current allocation size in bytes
new_size: desired size; if 0, deallocates and returns nil
ret: pointer to reallocated memory, or nil on failure

## fun heap_region_base

```mach
pub fun heap_region_base() usize;
```

query the current program break via brk(0)

ret: current break address

## fun heap_region_extend

```mach
pub fun heap_region_extend(addr: usize) usize;
```

set the program break to addr via brk

addr: desired new break address
ret: actual break address after the call

## fun protect

```mach
pub fun protect(p: ptr, size: usize, prot: u32) i64;
```

change protection on a memory region via mprotect

p: page-aligned pointer to the region
size: size of the region in bytes
prot: protection flags (PROT_READ, PROT_WRITE, PROT_EXEC, PROT_NONE)
ret: 0 on success, or negative errno

## fun lock

```mach
pub fun lock(p: ptr, size: usize) i64;
```

lock a memory region into physical RAM via mlock

p: pointer to the region
size: size of the region in bytes
ret: 0 on success, or negative errno

## fun unlock

```mach
pub fun unlock(p: ptr, size: usize) i64;
```

unlock a previously locked memory region via munlock

p: pointer to the region
size: size of the region in bytes
ret: 0 on success, or negative errno

## fun advise

```mach
pub fun advise(p: ptr, size: usize, advice: u32) i64;
```

advise the kernel on expected access patterns via madvise

p: pointer to the region
size: size of the region in bytes
advice: advisory hint (MADV_NORMAL, MADV_RANDOM, etc.)
ret: 0 on success, or negative errno

## fun allocate_huge

```mach
pub fun allocate_huge(size: usize, page_size: usize) ptr;
```

allocate memory backed by huge pages via mmap

size: number of bytes; if 0, returns nil
page_size: huge page size in bytes (must be power of two)
ret: pointer to allocated memory, or nil on failure

## fun numa_node_count

```mach
pub fun numa_node_count() usize;
```

return the number of NUMA nodes via get_mempolicy

ret: number of NUMA nodes (at least 1)

## fun numa_current_node

```mach
pub fun numa_current_node() usize;
```

return the NUMA node of the current CPU via getcpu

ret: NUMA node id (0 on failure)

## fun numa_allocate

```mach
pub fun numa_allocate(size: usize, node: usize) ptr;
```

allocate memory pinned to a specific NUMA node

size: number of bytes; if 0, returns nil
node: target NUMA node id
ret: pointer to allocated memory, or nil on failure

## fun numa_move

```mach
pub fun numa_move(p: ptr, size: usize, node: usize) i64;
```

move a memory region to a different NUMA node via mbind

p: pointer to the region
size: size of the region in bytes
node: target NUMA node id
ret: 0 on success, or negative errno

## fun map_file

```mach
pub fun map_file(fd: i32, offset: usize, size: usize, prot: u32) ptr;
```

map a file descriptor into memory via mmap

fd: file descriptor to map
offset: offset into the file (must be page-aligned)
size: number of bytes to map; if 0, returns nil
prot: protection flags
ret: pointer to mapped region, or nil on failure

## fun sync_file

```mach
pub fun sync_file(p: ptr, size: usize) i64;
```

flush a memory-mapped region to its backing file via msync

p: pointer to the mapped region
size: size of the region in bytes
ret: 0 on success, or negative errno

## fun has_numa

```mach
pub fun has_numa() bool;
```

check if the kernel supports NUMA memory policy

ret: true if get_mempolicy syscall is available

## fun has_huge_pages

```mach
pub fun has_huge_pages() bool;
```

check if the kernel supports huge page allocation

ret: true if MAP_HUGETLB mmap succeeds

## fun read

```mach
pub fun read(fd: i32, buf: *u8, count: usize) i64;
```

read bytes from a file descriptor

retries automatically on EINTR.

fd: file descriptor
buf: destination buffer
count: maximum bytes to read
ret: bytes read, or negative errno

## fun write

```mach
pub fun write(fd: i32, buf: *u8, count: usize) i64;
```

write bytes to a file descriptor

retries automatically on EINTR.

fd: file descriptor
buf: source buffer
count: bytes to write
ret: bytes written, or negative errno

## fun read_at

```mach
pub fun read_at(fd: i32, buf: *u8, count: usize, offset: u64) i64;
```

positioned file access does not mutate the descriptor offset

## fun write_at

```mach
pub fun write_at(fd: i32, buf: *u8, count: usize, offset: u64) i64;
```

## fun open

```mach
pub fun open(dirfd: i32, path: *u8, flags: i32, mode: i32) i64;
```

openat: open a file relative to a directory file descriptor

retries automatically on EINTR.

dirfd: directory fd (or AT_FDCWD)
path: null-terminated path
flags: open flags
mode: permission bits for creation
ret: new fd, or negative errno

## fun close

```mach
pub fun close(fd: i32) i64;
```

close a file descriptor

fd: file descriptor
ret: 0 on success, or negative errno

## fun sync_fd

```mach
pub fun sync_fd(fd: i32) i64;
```

flush an open file's data and metadata to its backing storage

## fun set_mode

```mach
pub fun set_mode(fd: i32, mode: i32) i64;
```

set permission bits through a held descriptor

## fun set_mode_at

```mach
pub fun set_mode_at(dirfd: i32, name: *u8, mode: i32) i64;
```

no following fallback is permitted when the flags-aware syscall is unavailable

## fun stat

```mach
pub fun stat(fd: i32, st: *stat_t) i64;
```

fstat: get file status by file descriptor

fd: file descriptor
st: pointer to stat_t to fill
ret: 0 on success, or negative errno

## fun publication_capabilities

```mach
pub fun publication_capabilities(fd: i32) i64;
```

## fun file_identity

```mach
pub fun file_identity(fd: i32, out: *identities.Identity) i64;
```

observational ids do not retain an object after this call

## fun retain_identity_at

```mach
pub fun retain_identity_at(dirfd: i32, path: *u8, access: identities.Access, out: *identities.Identity) i64;
```

o_path pins the entry without acquiring content-read permission

## fun identity_at

```mach
pub fun identity_at(dirfd: i32, path: *u8, out: *identities.Identity) i64;
```

## fun stat_path

```mach
pub fun stat_path(dirfd: i32, path: *u8, st: *stat_t, flags: i32) i64;
```

newfstatat: get file status relative to a directory fd

dirfd: directory fd (or AT_FDCWD)
path: null-terminated path
st: pointer to stat_t to fill
flags: AT_* flags
ret: 0 on success, or negative errno

## fun stat_mode

```mach
pub fun stat_mode(st: *stat_t) u32;
```

read the type/permission bits from a filled stat buffer as a 32-bit word

st_mode is 32-bit in the linux stat ABI; the accessor exists so portable
callers read the mode uniformly across platforms (darwin's mode is 16-bit).

st: pointer to a filled stat_t
ret: the raw mode bits, widened to 32 bits

## fun unlink

```mach
pub fun unlink(dirfd: i32, path: *u8, flags: i32) i64;
```

unlinkat: remove a file or directory relative to a directory fd

dirfd: directory fd (or AT_FDCWD)
path: null-terminated path
flags: AT_REMOVEDIR to remove a directory
ret: 0 on success, or negative errno

## fun rename

```mach
pub fun rename(olddir: i32, oldpath: *u8, newdir: i32, newpath: *u8) i64;
```

renameat: rename a file relative to directory fds

olddir: source directory fd
oldpath: source path
newdir: destination directory fd
newpath: destination path
ret: 0 on success, or negative errno

## fun lock_fd

```mach
pub fun lock_fd(fd: i32, op: i32) i64;
```

flock: take or release an advisory whole-file lock on an open descriptor

the lock lives on the open file DESCRIPTION, not on the directory entry, so
the kernel drops it when the last descriptor referring to that description is
closed -- including the implicit close every exiting process performs, however
it exits. a holder that crashes therefore leaves no lock behind, which is what
makes a live holder distinguishable from a dead one's residue.

a directory descriptor is lockable on linux, but windows cannot lock one, so
portable callers that want to exclude writers from a directory lock a regular
file inside it rather than the directory itself.

fd: open descriptor to lock, which may refer to a directory
op: LOCK_SH, LOCK_EX or LOCK_UN, optionally combined with LOCK_NB
ret: 0 on success, or negative errno; EAGAIN when LOCK_NB was set and
     another holder has the lock

## fun symlink

```mach
pub fun symlink(target: *u8, linkpath: *u8) i64;
```

create a symbolic link at linkpath pointing to target

target is stored verbatim: a relative target stays relative, so the link
keeps resolving correctly after the containing tree is moved.

target: path the link points to (stored as given, not resolved)
linkpath: path of the link to create
ret: 0 on success, or negative errno

## fun make_dir

```mach
pub fun make_dir(dirfd: i32, path: *u8, mode: i32) i64;
```

mkdirat: create a directory relative to a directory fd

dirfd: directory fd (or AT_FDCWD)
path: null-terminated path
mode: permission bits
ret: 0 on success, or negative errno

## fun directory_init

```mach
pub fun directory_init(out: *DirectoryCursor, fd: i32) DirectoryInitResult;
```

fd and its aliases lend exclusive enumeration and seek access until close

## fun directory_next

```mach
pub fun directory_next(cursor: *DirectoryCursor, out: *DirectoryEntry) i64;
```

## fun directory_close

```mach
pub fun directory_close(cursor: *DirectoryCursor) i64;
```

consumes the owner, including when native release reports failure

## fun access

```mach
pub fun access(dirfd: i32, path: *u8, mode: i32, flags: i32) i64;
```

faccessat: check file accessibility relative to a directory fd

dirfd: directory fd (or AT_FDCWD)
path: null-terminated path
mode: access mode flags
flags: AT_* flags
ret: 0 if accessible, or negative errno

## fun seek

```mach
pub fun seek(fd: i32, offset: i64, whence: i32) i64;
```

lseek: reposition read/write offset of a file descriptor

fd: file descriptor
offset: byte offset
whence: SEEK_SET, SEEK_CUR, or SEEK_END
ret: resulting offset, or negative errno

## var _envp

```mach
pub var _envp: usize = 0
```

environment (set by runtime at program start)

## fun environ

```mach
pub fun environ() **u8;
```

pointer to the environment captured by the runtime at program
start (null-terminated array of "NAME=value" strings). nil before
runtime init.

## var _pagesz

```mach
pub var _pagesz: usize = 0
```

the runtime page size in bytes, read from the auxiliary vector
(AT_PAGESZ) at program start. 0 until the runtime publishes it; AT_PAGESZ is
mandatory on linux, so a 0 still seen at a page_size() call means the auxv
lacked it (broken/nonstandard environment) or a caller ran before _rt_init
published, page_size() panics on either rather than fabricating a value. this
is the single consumer-facing page-size source (briar-systems/mach-std#336):
std.runtime.linux.reloc keeps its own private pre-relocation read of AT_PAGESZ
for the RELRO mprotect (it runs before any global is safe to touch), but every
page_size() caller reads this global.

## fun capture_pagesz

```mach
pub fun capture_pagesz();
```

publish the auxv page size into `_pagesz`

called once by the entrypoint at startup, after `_envp` is set and (in a --pie
build) after `_rt_relocate` has applied the relocations, so it runs as ordinary
post-relocation code that may touch globals. `_pagesz` stays 0 only if the auxv
lacks the mandatory AT_PAGESZ, which page_size() treats as a fatal invariant
breach rather than fabricating a default.

## fun terminate

```mach
pub fun terminate(code: i64);
```

exit the process with the given code

uses exit_group so all threads in the process are terminated; plain
exit would only end the calling thread, leaving the process alive.

code: exit code

## fun abort

```mach
pub fun abort();
```

abort the process immediately with exit code 255

## fun fork

```mach
pub fun fork() i64;
```

create a new child process

ret: in parent: child PID (>0), in child: 0, on error: negative errno

## fun vfork

```mach
pub fun vfork() i64;
```

create child process sharing memory with parent

ret: in parent: child PID (>0), in child: 0, on error: negative errno

## fun exec

```mach
pub fun exec(pathname: *u8, argv: **u8, envp: **u8) i64;
```

execve: replace the current process image with a new program

pathname: path to executable
argv: null-terminated argument array
envp: null-terminated environment array
ret: does not return on success; negative errno on error

## fun wait

```mach
pub fun wait(pid: i64, status: *os_shared.ProcessStatus, options: i32, rusage: *u8) os_shared.WaitResult;
```

failures and no-event results leave the caller's status unchanged

## fun wait_pid

```mach
pub fun wait_pid(pid: i64, status: *os_shared.ProcessStatus, options: i32) os_shared.WaitResult;
```

## fun terminate_child

```mach
pub fun terminate_child(pid: i64) i64;
```

forcefully stop a spawned child without reaping it

waitid with WNOWAIT first asks the kernel to verify that `pid` is still a
child of this process. this rejects unknown and already-reaped PIDs before
kill can affect them, while preserving an exited child's wait status.
callers must not concurrently reap this child through wait or wait_pid: a
reaper could release the numeric PID between the ownership check and kill.

pid: child process ID returned by spawn
ret: 0 on success, or negative errno

## fun setpgid

```mach
pub fun setpgid(pid: i64, pgid: i64) i64;
```

setpgid: set (or join) the process group ID of a process

pid: target process ID (0 = caller)
pgid: desired process group ID (0 = pid becomes the leader of a new group)
ret: 0 on success, or negative errno

## fun getpgid

```mach
pub fun getpgid(pid: i64) i64;
```

getpgid: read the process group ID of a process

pid: target process ID (0 = caller)
ret: process group ID, or negative errno

## fun terminate_group

```mach
pub fun terminate_group(pgid: i64) i64;
```

signal every process in a group without reaping any of them

unlike terminate_child, there is no waitid ownership probe first: a process
group is not tracked as a child of this process, so kill(-pgid, SIGTERM) is
the only available primitive. surviving members stay individually waitable
through wait/wait_any exactly as before.

pgid: process group ID (must be > 0; a negated pgid targets the whole group)
ret: 0 on success, or negative errno

## fun spawn

```mach
pub fun spawn(pathname: *u8, argv: **u8, envp: **u8) i64;
```

create a child process running the given program

closes inherited file descriptors >= 3 in the child before exec to
prevent leaking parent FDs (pipes, sockets, etc.) to the child.

pathname: path to executable
argv: null-terminated argument array
envp: null-terminated environment array
ret: child PID on success, or negative errno

## fun spawn_grouped

```mach
pub fun spawn_grouped(pathname: *u8, argv: **u8, envp: **u8) i64;
```

create a child process running the given program, placed as the leader of
its own new process group (see spawn_redirected_in_grouped)

pathname: path to executable
argv: null-terminated argument array
envp: null-terminated environment array
ret: child PID on success, or negative errno; the child's pgid equals its pid

## fun spawn_in

```mach
pub fun spawn_in(pathname: *u8, argv: **u8, envp: **u8, cwd: *u8) i64;
```

create a child process running the given program from `cwd`

pathname: path to executable
argv: null-terminated argument array
envp: null-terminated environment array, or nil to inherit
cwd: working directory for the child, or nil to inherit
ret: child PID on success, or negative errno

## fun spawn_redirected

```mach
pub fun spawn_redirected(pathname: *u8, argv: **u8, envp: **u8, stdin_fd: i32, stdout_fd: i32, stderr_fd: i32) i64;
```

spawn a child with its standard streams bound to
caller-supplied descriptors.

forks, duplicates each non-negative descriptor onto the matching
standard stream (-1 inherits the parent's stream), closes inherited
descriptors >= 3 (including both ends of a capture pipe), and execs.
the parent keeps ownership of the passed descriptors; pairing one with
a pipe end lets the parent stream the child's stdio. the child exits
126 when a redirect fails and 127 when exec fails.

pathname: path to executable
argv: null-terminated argument array
envp: null-terminated environment array
stdin_fd: descriptor to install as the child's stdin, or -1 to inherit
stdout_fd: descriptor to install as the child's stdout, or -1 to inherit
stderr_fd: descriptor to install as the child's stderr, or -1 to inherit
ret: child PID on success, or negative errno

## fun spawn_redirected_grouped

```mach
pub fun spawn_redirected_grouped(pathname: *u8, argv: **u8, envp: **u8, stdin_fd: i32, stdout_fd: i32, stderr_fd: i32) i64;
```

spawn a child with its standard streams bound to caller-supplied
descriptors, placed as the leader of its own new process group (see
spawn_redirected_in_grouped)

pathname: path to executable
argv: null-terminated argument array
envp: null-terminated environment array
stdin_fd: descriptor to install as the child's stdin, or -1 to inherit
stdout_fd: descriptor to install as the child's stdout, or -1 to inherit
stderr_fd: descriptor to install as the child's stderr, or -1 to inherit
ret: child PID on success, or negative errno; the child's pgid equals its pid

## fun spawn_redirected_in

```mach
pub fun spawn_redirected_in(pathname: *u8, argv: **u8, envp: **u8, cwd: *u8, stdin_fd: i32, stdout_fd: i32, stderr_fd: i32) i64;
```

spawn a child with redirected streams and an explicit working directory

`cwd` is the directory the child starts in, or nil to inherit the parent's.
the chdir happens in the child, after the fork/clone, so the parent's own
working directory is untouched. passing it here rather than prefixing a `cd`
onto a shell command keeps the path out of the command line, where it would
have to survive a quoting convention the target program may not share
(mach#2587).

pathname: path to executable
argv: null-terminated argument array
envp: null-terminated environment array, or nil to inherit
cwd: working directory for the child, or nil to inherit
stdin_fd: descriptor to install as the child's stdin, or -1 to inherit
stdout_fd: descriptor to install as the child's stdout, or -1 to inherit
stderr_fd: descriptor to install as the child's stderr, or -1 to inherit
ret: child PID on success, or negative errno

## fun spawn_redirected_in_grouped

```mach
pub fun spawn_redirected_in_grouped(pathname: *u8, argv: **u8, envp: **u8, cwd: *u8, stdin_fd: i32, stdout_fd: i32, stderr_fd: i32) i64;
```

spawn a child with redirected streams and an explicit working directory,
placed as the leader of its own new process group

identical to spawn_redirected_in except the child calls setpgid(0, 0)
between the clone and the exec (see spawn_trampoline), and the parent ALSO
calls setpgid(pid, pid) once clone returns, ignoring its result (mach#535).
on a real kernel CLONE_VFORK suspends the parent until the child execs or
exits, so the parent-side call is redundant there (it either finds the
child already exec'd, where POSIX's own rule against changing pgid post-exec
makes it a harmless no-op, or - vanishingly rarely - still pre-exec, where it
just repeats what the child was about to do anyway). it is NOT redundant
everywhere this runs: qemu-user (the riscv64 CI lane) silently downgrades
CLONE_VFORK to a plain fork, so the parent truly can run concurrently with
the child there, and a caller's getpgid(pid) right after this call returning
could otherwise observe the parent's own group for a still-ungrouped child.
the same double-setpgid mitigation the darwin backend already needs for its
real fork() closes that window here too.

pathname: path to executable
argv: null-terminated argument array
envp: null-terminated environment array, or nil to inherit
cwd: working directory for the child, or nil to inherit
stdin_fd: descriptor to install as the child's stdin, or -1 to inherit
stdout_fd: descriptor to install as the child's stdout, or -1 to inherit
stderr_fd: descriptor to install as the child's stderr, or -1 to inherit
ret: child PID on success, or negative errno; the child's pgid equals its pid

## fun spawn_shell

```mach
pub fun spawn_shell(command: *u8, envp: **u8, cwd: *u8) i64;
```

spawn `command` through the host command interpreter, from `cwd`

posix hands argv straight to execve, so the command is one argument and there
is no command-line encoding step to get wrong - the counterpart to the windows
backend, where the interpreter's quoting rules have to be honoured explicitly.

command: the command line to hand the interpreter
envp: null-terminated environment array, or nil to inherit
cwd: working directory for the child, or nil to inherit
ret: child PID on success, or negative errno

## fun running_under_user_emulation

```mach
pub fun running_under_user_emulation() bool;
```

detect a user-mode emulator such as qemu-user

the kernel names a process after the image it executed (/proc/self/comm, at most
fifteen bytes), while an emulator presents the guest's own command line through
/proc/self/cmdline. natively the two name the same executable; under emulation the
comm names the emulator. tests use this to decline what an emulator's syscall
translation cannot reproduce, as running_under_wine does on windows.

ret: true when the process image is not the executable the command line names

## fun getpid

```mach
pub fun getpid() i64;
```

return the current process ID

ret: process ID

## fun pipe

```mach
pub fun pipe(fds: *i32) i64;
```

create a unidirectional pipe

fds: pointer to two i32s; fds[0] = read end, fds[1] = write end
ret: 0 on success, or negative errno

## fun ignore_sigpipe

```mach
pub fun ignore_sigpipe() i64;
```

ignore SIGPIPE for this process

rt_sigaction's first two words are the handler and flags on every supported
ISA. x86_64 and aarch64 then carry a restorer word before the signal mask;
riscv64 puts the mask in the third word. a zeroed four-word buffer satisfies
both layouts because this disposition has no flags, restorer, or masked
signals. the kernel reads only its own layout and the trailing word is inert.

the disposition is process-wide: after success, writes from every thread
return EPIPE when their pipe has no reader instead of terminating the process.

ret: 0 on success, or negative errno

## fun sleep

```mach
pub fun sleep(nsec: i64);
```

suspend execution for the given number of nanoseconds

nsec: nanoseconds to sleep

## fun cpu_count

```mach
pub fun cpu_count() i64;
```

the number of CPUs available to this process

popcounts the sched_getaffinity mask, so taskset/cgroup cpuset restrictions
are honoured; the kernel reports the mask size it wrote in the return value.
never returns less than 1.

ret: usable CPU count (>= 1)

## fun getcwd

```mach
pub fun getcwd(buf: *u8, size: usize) i64;
```

get the current working directory

the raw syscall returns the length INCLUDING the terminator; this layer
reports the path length the way darwin and windows do, so "length of path"
means one thing across the three backends (#432).

buf: destination buffer
size: buffer capacity in bytes, including room for the terminator
ret: length of the path, excluding the terminator, or negative errno

## fun getenv

```mach
pub fun getenv(name: *u8, buf: *u8, cap: usize) i64;
```

look up an environment variable by name

when the value fits (ret < cap) it is copied and null-terminated.
always returns the full value length, so ret >= cap signals truncation
(buffer contents unspecified) and ret + 1 is the capacity to retry with.

name: null-terminated variable name
buf: destination buffer for the value
cap: buffer capacity in bytes
ret: full length of the value, or NOT_FOUND

## fun thread_wait

```mach
pub fun thread_wait(addr: *i64, expected: i64) i64;
```

block until the value at addr changes from expected

wraps the futex FUTEX_WAIT operation. returns immediately if *addr
does not equal expected. spurious wakeups are possible.

addr: pointer to the futex word
expected: value to compare against
ret: 0 on wake, or negative errno

## fun thread_wait_until

```mach
pub fun thread_wait_until(addr: *i64, expected: i64, deadline: *os_shared.Timespec) i64;
```

block until the word changes or an absolute monotonic deadline is reached

## fun thread_wake

```mach
pub fun thread_wake(addr: *i64) i64;
```

wake one thread blocked on a futex wait at addr

addr: pointer to the futex word
ret: number of threads woken, or negative errno

## val IO_EVENT_READ

```mach
pub val IO_EVENT_READ:   u32 = 0x01
```

## val IO_EVENT_WRITE

```mach
pub val IO_EVENT_WRITE:  u32 = 0x02
```

## val IO_EVENT_ERROR

```mach
pub val IO_EVENT_ERROR:  u32 = 0x04
```

## val IO_EVENT_HANGUP

```mach
pub val IO_EVENT_HANGUP: u32 = 0x08
```

## val IO_EVENT_WAKE

```mach
pub val IO_EVENT_WAKE:   u32 = 0x10
```

## rec IoCompletion

```mach
pub rec IoCompletion;
```

context is opaque to the OS and zero is reserved for the runtime wake source

## fun io_queue_create

```mach
pub fun io_queue_create(queue: *os_shared.IoQueue) i64;
```

## fun io_queue_watch

```mach
pub fun io_queue_watch(queue: *os_shared.IoQueue, resource: usize, context: usize, events: u32) i64;
```

## fun io_queue_unwatch

```mach
pub fun io_queue_unwatch(queue: *os_shared.IoQueue, resource: usize) i64;
```

## fun io_queue_poll

```mach
pub fun io_queue_poll(queue: *os_shared.IoQueue, completions: *IoCompletion, capacity: usize, timeout_ms: i32) i64;
```

completions is also the kernel scratch buffer. normalization runs backwards
so expanding packed x86-64 epoll records cannot overwrite unread records.

## fun io_queue_wait

```mach
pub fun io_queue_wait(queue: *os_shared.IoQueue, timeout_ms: i32) i64;
```

## fun io_queue_wake

```mach
pub fun io_queue_wake(queue: *os_shared.IoQueue) i64;
```

nil is invalid use and a closed queue is EBADF on every platform

## fun io_queue_close

```mach
pub fun io_queue_close(queue: *os_shared.IoQueue) i64;
```

## val AF_INET

```mach
pub val AF_INET:             i32 = 2
```

## val AF_INET6

```mach
pub val AF_INET6:            i32 = 10
```

## val SOCK_STREAM

```mach
pub val SOCK_STREAM:         i32 = 1
```

## val SOCK_DGRAM

```mach
pub val SOCK_DGRAM:          i32 = 2
```

## val SOCK_NONBLOCK

```mach
pub val SOCK_NONBLOCK:       i32 = O_NONBLOCK
```

## val SOCK_CLOEXEC

```mach
pub val SOCK_CLOEXEC:        i32 = 0x80000
```

## val SOL_SOCKET

```mach
pub val SOL_SOCKET:          i32 = 1
```

## val SO_UNSUPPORTED

```mach
pub val SO_UNSUPPORTED:      i32 = -2147483648
```

## val SO_REUSEADDR

```mach
pub val SO_REUSEADDR:        i32 = 2
```

## val SO_REUSEPORT

```mach
pub val SO_REUSEPORT:        i32 = 15
```

## val SO_EXCLUSIVEADDRUSE

```mach
pub val SO_EXCLUSIVEADDRUSE: i32 = SO_UNSUPPORTED
```

## val SO_KEEPALIVE

```mach
pub val SO_KEEPALIVE:        i32 = 9
```

## val SO_SNDBUF

```mach
pub val SO_SNDBUF:           i32 = 7
```

## val SO_RCVBUF

```mach
pub val SO_RCVBUF:           i32 = 8
```

## val SO_LINGER

```mach
pub val SO_LINGER:           i32 = 13
```

## val SO_RCVTIMEO

```mach
pub val SO_RCVTIMEO:         i32 = 20
```

## val IPPROTO_IP

```mach
pub val IPPROTO_IP:          i32 = 0
```

## val IPPROTO_TCP

```mach
pub val IPPROTO_TCP:         i32 = 6
```

## val IPPROTO_IPV6

```mach
pub val IPPROTO_IPV6:        i32 = 41
```

## val IP_TOS

```mach
pub val IP_TOS:              i32 = 1
```

## val IPV6_V6ONLY

```mach
pub val IPV6_V6ONLY:         i32 = 26
```

## val IPV6_TCLASS

```mach
pub val IPV6_TCLASS:         i32 = 67
```

## val TCP_NODELAY

```mach
pub val TCP_NODELAY:         i32 = 1
```

## val TCP_KEEPIDLE

```mach
pub val TCP_KEEPIDLE:        i32 = 4
```

## val TCP_KEEPINTVL

```mach
pub val TCP_KEEPINTVL:       i32 = 5
```

## val TCP_KEEPCNT

```mach
pub val TCP_KEEPCNT:         i32 = 6
```

## val TCP_FASTOPEN

```mach
pub val TCP_FASTOPEN:        i32 = 23
```

## val SHUT_RD

```mach
pub val SHUT_RD:             i32 = 0
```

## val SHUT_WR

```mach
pub val SHUT_WR:             i32 = 1
```

## val SHUT_RDWR

```mach
pub val SHUT_RDWR:           i32 = 2
```

## val SOCKADDR_IN_SIZE

```mach
pub val SOCKADDR_IN_SIZE:      usize = 16
```

## val SOCKADDR_IN6_SIZE

```mach
pub val SOCKADDR_IN6_SIZE:     usize = 28
```

## val SOCKADDR_STORAGE_SIZE

```mach
pub val SOCKADDR_STORAGE_SIZE: usize = 128
```

## fun sock_addr_family

```mach
pub fun sock_addr_family(sa: *u8) i32;
```

## fun sock_addr_init

```mach
pub fun sock_addr_init(sa: *u8, port: u16, addr: *u8);
```

fill a sockaddr_in buffer with the given port and IPv4 address

sa: pointer to a buffer of at least SOCKADDR_IN_SIZE bytes
port: port in host byte order
addr: pointer to 4 IPv4 address bytes in network order

## fun sock_addr_read

```mach
pub fun sock_addr_read(sa: *u8, port: *u16, addr: *u8);
```

read port and IPv4 address from a sockaddr_in buffer

sa: pointer to the sockaddr_in buffer
port: pointer to store port in host byte order
addr: pointer to store 4 IPv4 address bytes

## fun sock_addr6_init

```mach
pub fun sock_addr6_init(sa: *u8, port: u16, addr: *u8, scope: u32);
```

## fun sock_addr6_read

```mach
pub fun sock_addr6_read(sa: *u8, port: *u16, addr: *u8, scope: *u32);
```

## fun sock_send

```mach
pub fun sock_send(fd: usize, buf: *u8, len: usize) i64;
```

send data on a connected socket

fd: socket fd
buf: source buffer
len: number of bytes to send
ret: bytes sent, or negative errno

## fun sock_recv

```mach
pub fun sock_recv(fd: usize, buf: *u8, len: usize) i64;
```

receive data from a connected socket

fd: socket fd
buf: destination buffer
len: maximum bytes to receive
ret: bytes received, or negative errno

## rec IoVector

```mach
pub rec IoVector;
```

## fun sock_write_vector

```mach
pub fun sock_write_vector(fd: usize, vectors: *IoVector, count: usize) i64;
```

## fun sock_send_message

```mach
pub fun sock_send_message(fd: usize, vectors: *IoVector, vector_count: usize, address: *u8, address_length: u32, control: *u8, control_length: usize) i64;
```

## fun sock_receive_message

```mach
pub fun sock_receive_message(fd: usize, vectors: *IoVector, vector_count: usize, address: *u8, address_length: *u32, control: *u8, control_length: *usize, flags: *i32) i64;
```

## val MSG_CTRUNC

```mach
pub val MSG_CTRUNC:       i32 = 0x8
```

## val MSG_CMSG_CLOEXEC

```mach
pub val MSG_CMSG_CLOEXEC: i32 = 0x40000000
```

## val SCM_RIGHTS

```mach
pub val SCM_RIGHTS:       i32 = 1
```

## val SCM_CREDENTIALS

```mach
pub val SCM_CREDENTIALS:  i32 = 2
```

## val SCM_PIDFD

```mach
pub val SCM_PIDFD:        i32 = 4
```

## val SCM_MAX_FD

```mach
pub val SCM_MAX_FD:          usize = 253
```

net/scm.h SCM_MAX_FD: sendmsg refuses a larger descriptor set, and a stream receive
returns at most one set (unix_stream_read_generic stops after the record that carries it)

## val LOCAL_CONTROL_CAPACITY

```mach
pub val LOCAL_CONTROL_CAPACITY: usize = 1088
```

CMSG_SPACE(253 descriptors) + CMSG_SPACE(ucred) + CMSG_SPACE(pidfd): the largest descriptor
set together with the fixed-size records SO_PASSCRED and SO_PASSPIDFD prepend to it

## fun sock_recv_close_rights

```mach
pub fun sock_recv_close_rights(fd: usize, buffer: *u8, length: usize, address: *u8, address_length: *u32, control: *u8, capacity: usize) i64;
```

receives bytes and closes every descriptor the kernel installed for them. installation is
close-on-exec, the kernel installs no descriptor it does not report (scm_detach_fds writes
the slot before fd_install and drops what does not fit under MSG_CTRUNC), and close always
releases a descriptor on linux, so the byte count is the outcome and the walk is complete.
address, when given, receives the sender and address_length its extent.

## fun local_stream_recv

```mach
pub fun local_stream_recv(fd: usize, buffer: *u8, length: usize) i64;
```

byte reception for local streams: rights a peer attaches are observed and closed

## fun sock_connect_status

```mach
pub fun sock_connect_status(fd: usize) i64;
```

## fun sock_close

```mach
pub fun sock_close(fd: usize) i64;
```

close a socket file descriptor

fd: socket fd
ret: 0 on success, or negative errno

## fun sock_create

```mach
pub fun sock_create(domain: i32, typ: i32, protocol: i32, out: *usize) i64;
```

create a socket file descriptor

domain: address family (AF_INET)
typ: socket type (SOCK_STREAM, SOCK_DGRAM)
protocol: protocol (0 for default)
out: receives the native-width handle
ret: 0 on success, or negative errno

## fun sock_create_flags

```mach
pub fun sock_create_flags(domain: i32, typ: i32, protocol: i32, nonblocking: bool, non_inheritable: bool, out: *usize) i64;
```

## fun sock_bind

```mach
pub fun sock_bind(fd: usize, addr: *u8, addrlen: usize) i64;
```

bind a socket to an address

fd: socket fd
addr: pointer to sockaddr structure
addrlen: size of the sockaddr structure
ret: 0 on success, or negative errno

## fun sock_listen

```mach
pub fun sock_listen(fd: usize, backlog: i32) i64;
```

mark a socket as a passive listener

fd: socket fd
backlog: maximum pending connection queue length
ret: 0 on success, or negative errno

## fun sock_accept

```mach
pub fun sock_accept(fd: usize, addr: *u8, addrlen: *u32, out: *usize) i64;
```

accept a connection on a listening socket

fd: socket fd
addr: pointer to sockaddr to fill with peer address
addrlen: pointer to sockaddr length (in/out)
out: receives the native-width accepted handle
ret: 0 on success, or negative errno

## fun sock_accept_flags

```mach
pub fun sock_accept_flags(fd: usize, addr: *u8, addrlen: *u32, nonblocking: bool, non_inheritable: bool, out: *usize) i64;
```

## fun sock_connect

```mach
pub fun sock_connect(fd: usize, addr: *u8, addrlen: usize) i64;
```

connect a socket to a remote address

fd: socket fd
addr: pointer to sockaddr structure
addrlen: size of the sockaddr structure
ret: 0 on success, or negative errno

## fun sock_sendto

```mach
pub fun sock_sendto(fd: usize, buf: *u8, len: usize, flags: i32, addr: *u8, addrlen: usize) i64;
```

send data to a specific address

fd: socket fd
buf: source buffer
len: number of bytes to send
flags: send flags
addr: pointer to destination sockaddr
addrlen: size of the sockaddr structure
ret: bytes sent on success, or negative errno

## fun sock_recvfrom

```mach
pub fun sock_recvfrom(fd: usize, buf: *u8, len: usize, flags: i32, addr: *u8, addrlen: *u32) i64;
```

receive data and source address

fd: socket fd
buf: destination buffer
len: maximum bytes to receive
flags: recv flags
addr: pointer to sockaddr to fill with source address
addrlen: pointer to sockaddr length (in/out)
ret: bytes received on success, or negative errno

## fun sock_shutdown

```mach
pub fun sock_shutdown(fd: usize, how: i32) i64;
```

shut down part or all of a socket connection

fd: socket fd
how: SHUT_RD (0), SHUT_WR (1), or SHUT_RDWR (2)
ret: 0 on success, or negative errno

## fun sock_setopt

```mach
pub fun sock_setopt(fd: usize, level: i32, opt: i32, value: *u8, vallen: usize) i64;
```

set a socket option

fd: socket fd
level: option level (SOL_SOCKET, etc.)
opt: option name (SO_REUSEADDR, etc.)
value: pointer to option value
vallen: size of option value
ret: 0 on success, or negative errno

## fun sock_getopt

```mach
pub fun sock_getopt(fd: usize, level: i32, opt: i32, value: *u8, vallen: *u32) i64;
```

## fun sock_local_addr

```mach
pub fun sock_local_addr(fd: usize, addr: *u8, addrlen: *u32) i64;
```

## fun sock_remote_addr

```mach
pub fun sock_remote_addr(fd: usize, addr: *u8, addrlen: *u32) i64;
```

## fun sock_inheritable

```mach
pub fun sock_inheritable(fd: usize, out: *bool) i64;
```

## fun sock_set_rcvtimeo

```mach
pub fun sock_set_rcvtimeo(fd: usize, ms: u64) i64;
```

set the receive timeout on a socket

writes a struct timeval (tv_sec, tv_usec) derived from ms as the SO_RCVTIMEO
option value. ms == 0 clears the timeout, restoring indefinite blocking. once
set, a recv with no data pending fails with EAGAIN after the interval elapses.

fd: socket fd
ms: timeout in milliseconds, or 0 for no timeout
ret: 0 on success, or negative errno

## val DNS_HOSTS_PATH

```mach
pub val DNS_HOSTS_PATH: str = "/etc/hosts"
```

## fun random_fill

```mach
pub fun random_fill(buf: *u8, len: usize) i64;
```

fill a buffer with cryptographic random bytes via getrandom

buf: destination buffer
len: number of bytes to fill
ret: 0 on success, or negative errno

## fun clock_gettime

```mach
pub fun clock_gettime(clock_id: i32, ts: *timespec) i64;
```

get time from a clock source

clock_id: clock to query (CLOCK_REALTIME, CLOCK_MONOTONIC)
ts: pointer to timespec to fill
ret: 0 on success, negative errno on failure

## fun error_message

```mach
pub fun error_message(code: i64) str;
```

return a human-readable message for a negated errno code

code: negated errno value (e.g., -2 for ENOENT)
ret: descriptive error message string

