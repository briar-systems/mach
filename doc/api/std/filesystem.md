# std.filesystem

## rec File

```mach
pub rec File;
```

an open file with a native-width opaque handle

## tag FsError

```mach
pub tag FsError: u8 {
    io:      io_error.Error;
    alloc:   A.Error;
    read:    ReadError;
    write:   WriteError;
    removal: removal.Error;
    exhausted;
    published: io_error.Error;
}
```

why a composite filesystem operation refused

io: a native refusal, with its operation and any cleanup failure
alloc: the allocator refused
read: the content read failed; eof is a file shorter than its size said
write: the content write failed; the payload is the persisted prefix
removal: a recursive removal refused, by containment or natively
exhausted: every candidate temporary name was taken
published: the replacement is visible, but the directory flush after it failed

## def Kind

```mach
pub def Kind: u8
```

what a path resolves to

## val KIND_OTHER

```mach
pub val KIND_OTHER:   Kind = 0
```

## val KIND_FILE

```mach
pub val KIND_FILE:    Kind = 1
```

## val KIND_DIR

```mach
pub val KIND_DIR:     Kind = 2
```

## val KIND_SYMLINK

```mach
pub val KIND_SYMLINK: Kind = 3
```

## rec Metadata

```mach
pub rec Metadata;
```

portable file metadata

kind: file, directory, symlink, or other
size: size in bytes
mode: raw permission/type bits (POSIX; synthesized elsewhere)
modified: last modification time
accessed: last access time
created: birth time when the platform provides it, else none

## val STDIN

```mach
pub val STDIN:  File = File;
```

the standard streams, as already-open files

## val STDOUT

```mach
pub val STDOUT: File = File;
```

## val STDERR

```mach
pub val STDERR: File = File;
```

## fun open

```mach
pub fun open(p: Path) res[File, io_error.Error];
```

open a file for reading

p: file path
ret: the open file, or the native refusal

## fun create

```mach
pub fun create(p: Path, mode: i32) res[File, io_error.Error];
```

create or truncate a file for writing

p: file path
mode: permission bits for a newly created file
ret: the open file, or the native refusal

## fun close

```mach
pub fun close(f: *File) err[io_error.Error];
```

close an open file; the handle is invalid afterwards whatever the outcome

f: file to close
ret: ok, or the native refusal (a closed handle is EBADF)

## fun sync

```mach
pub fun sync(f: File) err[io_error.Error];
```

flush an open file's data and metadata to its backing storage

f: file to flush
ret: ok, or the native refusal

## fun read

```mach
pub fun read(f: File, buf: *u8, len: usize) res[usize, io_error.Error];
```

read up to len bytes into buf

f: file to read from
buf: destination buffer
len: maximum bytes to read
ret: bytes read (0 at end of file), or the native refusal

## fun write

```mach
pub fun write(f: File, buf: *u8, len: usize) res[usize, io_error.Error];
```

write up to len bytes from buf

f: file to write to
buf: source buffer
len: bytes to write
ret: bytes written, or the native refusal

## fun seek

```mach
pub fun seek(f: File, offset: i64, whence: i32) res[i64, io_error.Error];
```

reposition the file offset

f: file to seek
offset: byte offset relative to whence
whence: SEEK_SET, SEEK_CUR, or SEEK_END
ret: the resulting absolute offset, or the native refusal

## fun reader

```mach
pub fun reader(f: File) m_reader.Reader;
```

adapt a file to an io.Reader (the file must outlive the reader)

f: file to read from
ret: a Reader drawing from f

## fun writer

```mach
pub fun writer(f: File) m_writer.Writer;
```

adapt a file to an io.Writer (the file must outlive the writer)

f: file to write to
ret: a Writer feeding f

## fun read_bytes

```mach
pub fun read_bytes(a: *A.Allocator, p: Path) res[Vector[u8], FsError];
```

read an entire file into an allocated byte buffer

composes io.read_all over the file's Reader: grows as it reads, so it needs
no size assumption. the caller owns the returned vector; on failure nothing
is owned.

a: allocator for the result
p: file path
ret: the file contents, or why they could not be read

## fun read_string

```mach
pub fun read_string(a: *A.Allocator, p: Path) res[str, FsError];
```

read an entire file into an allocated, null-terminated string

stats the file for its size and composes io.read_exact into an exact buffer,
allocating once with no growth. for text; binary content with embedded NUL
bytes loses its length to the first NUL when read through str_len. a file
that shrinks between the stat and the read is reported as read{eof} with the
bytes that were delivered, and nothing stays allocated.

a: allocator for the result
p: file path
ret: the file contents, or why they could not be read

## fun write_bytes

```mach
pub fun write_bytes(p: Path, data: *u8, len: usize, mode: i32) err[FsError];
```

write bytes to a file, creating or truncating it

composes io.write_all over the file's Writer. a write failure carries the
persisted prefix; a close failure after a complete write is reported as io,
because the bytes are not known to have reached the file.

p: file path
data: bytes to write
len: number of bytes
mode: permission bits for a newly created file
ret: ok, or why the bytes are not all in the file

## fun replace_bytes_atomic

```mach
pub fun replace_bytes_atomic(a: *A.Allocator, p: Path, data: *u8, len: usize, file_mode: i32, dir_mode: i32) err[FsError];
```

replace a file with complete bytes through a durable sibling temporary

missing parent directories are created before the temporary. the temporary is
created beside the destination with O_EXCL, flushed, and closed before rename,
so readers see either the old file or the complete replacement and a
cross-device rename cannot occur. after rename, the parent directory is
flushed through the platform durability contract. if that final flush
fails, the replacement is already visible: the outcome is `published` with
the flush failure, distinct from every refusal before the rename, after
which the destination is untouched and the temporary is removed.

a: allocator for temporary paths
p: destination path
data: bytes to write
len: number of bytes
file_mode: permission bits for the replacement file
dir_mode: permission bits for newly created parent directories
ret: ok, or why the replacement is not complete and durable

## fun identity_equal

```mach
pub fun identity_equal(a: Identity, b: Identity) bool;
```

## fun identity_of

```mach
pub fun identity_of(f: File) res[Identity, io_error.Error];
```

observations do not retain an object or imply unchanged contents

## fun identity_link

```mach
pub fun identity_link(p: Path) res[Identity, io_error.Error];
```

the final symlink itself is observed and unsupported domains remain explicit

## fun stat_of

```mach
pub fun stat_of(f: File) res[Metadata, io_error.Error];
```

metadata for an open file

## fun metadata

```mach
pub fun metadata(p: Path) res[Metadata, io_error.Error];
```

metadata for a path, following a final symbolic link

p: path to query
ret: the metadata, or the native refusal

## fun metadata_link

```mach
pub fun metadata_link(p: Path) res[Metadata, io_error.Error];
```

metadata for a path without following a final symbolic link

p: path to query
ret: the metadata (a symlink reports itself), or the native refusal

## fun meta_is_dir

```mach
pub fun meta_is_dir(m: *Metadata) bool;
```

check whether metadata describes a directory

## fun meta_is_file

```mach
pub fun meta_is_file(m: *Metadata) bool;
```

check whether metadata describes a regular file

## fun meta_is_symlink

```mach
pub fun meta_is_symlink(m: *Metadata) bool;
```

check whether metadata describes a symbolic link

## fun exists

```mach
pub fun exists(p: Path) res[bool, io_error.Error];
```

check whether a path exists (as any entry, including a broken symlink)

ret: true or false when the question could be answered, or the native
     refusal (permission denied, a symlink loop) when it could not

## fun is_file

```mach
pub fun is_file(p: Path) res[bool, io_error.Error];
```

check whether a path is an existing regular file (following symlinks)

## fun is_dir

```mach
pub fun is_dir(p: Path) res[bool, io_error.Error];
```

check whether a path is an existing directory (following symlinks)

## fun is_symlink

```mach
pub fun is_symlink(p: Path) res[bool, io_error.Error];
```

check whether a path is a symbolic link (not following it)

## fun create_dir

```mach
pub fun create_dir(p: Path, mode: i32) err[io_error.Error];
```

create a single directory

p: directory path
mode: permission bits
ret: ok, or the native refusal

## fun remove_file

```mach
pub fun remove_file(p: Path) err[io_error.Error];
```

remove a single file or symbolic link

p: path to remove
ret: ok, or the native refusal

## fun remove_dir

```mach
pub fun remove_dir(p: Path) err[io_error.Error];
```

remove an empty directory

p: directory path to remove
ret: ok, or the native refusal

## fun rename

```mach
pub fun rename(from: Path, to: Path) err[io_error.Error];
```

rename or move a file or directory

An existing destination file is replaced, as POSIX rename does. Replacing an
existing *directory* is not portable: POSIX allows it when the destination is
an empty directory and windows refuses it outright, so callers should not rely
on it.
windows retains open destination handles on supporting local filesystems,
while smb refuses replacement until destination handles close.

from: source path
to: destination path
ret: ok, or the native refusal

## fun symlink

```mach
pub fun symlink(target: Path, linkpath: Path) err[io_error.Error];
```

create a symbolic link at linkpath pointing to target

target: path the link points to (stored verbatim)
linkpath: path of the link to create
ret: ok, or the native refusal

## fun read_dir

```mach
pub fun read_dir(a: *A.Allocator, p: Path) res[Vector[str], FsError];
```

list the entries of a directory, excluding "." and ".."

each name and the vector storage are owned by the caller. free each name
with std.text.string.str_free before vector.dnit, or release the whole arena.
names are returned in directory order. a failure releases every partial
result and reports the first refusal, with a cursor or descriptor cleanup
failure kept beside it.

a: allocator for the vector and each name
p: directory path
ret: the child names, or why they could not be listed

## fun create_dir_all

```mach
pub fun create_dir_all(a: *A.Allocator, p: Path, mode: i32) err[FsError];
```

create a directory and every missing parent (mkdir -p)

a directory that already exists is success. when the final creation fails
and the path is still not a directory, the creation's refusal is reported.

a: allocator for transient parent paths
p: directory path
mode: permission bits for created directories
ret: ok, or why the directory does not exist

## fun remove_all

```mach
pub fun remove_all(a: *A.Allocator, p: Path) err[FsError];
```

recursively remove a file, directory tree, or symbolic link (rm -rf)

directories are removed depth-first. symbolic links are removed as links and
never followed, so a link pointing outside the tree leaves its target intact.
a missing path succeeds, making the operation idempotent.
windows force deletion preserves alias attributes and reports unsupported
for read-only entries on filesystems without native posix deletion.
roots and dot entries are refused before mutation, as a removal containment
refusal.

a: allocator for the owned input and parent paths
p: path naming the child entry to remove
ret: ok, or why the entry may still exist

## rec TempFile

```mach
pub rec TempFile;
```

a created temporary file and its path

file: the open handle
path: the allocated path (freed with the caller's allocator)

## fun temp_create

```mach
pub fun temp_create(a: *A.Allocator, prefix: str) res[TempFile, FsError];
```

create a uniquely named temporary file under the OS temp directory

the file is opened O_CREAT|O_EXCL, so the returned path is exclusively owned.

a: allocator for the path
prefix: name prefix (e.g. "build" -> "<tmp>/build_<hex>")
ret: the created temporary file, or why none could be created

## fun temp_path

```mach
pub fun temp_path(tf: *TempFile) Path;
```

the path of a temporary file

## fun temp_close

```mach
pub fun temp_close(tf: *TempFile) err[io_error.Error];
```

close a temporary file without removing it

## fun temp_remove

```mach
pub fun temp_remove(tf: *TempFile) err[io_error.Error];
```

remove a temporary file from disk

## fun temp_close_and_remove

```mach
pub fun temp_close_and_remove(tf: *TempFile) err[io_error.Error];
```

close and remove a temporary file; both effects run, and a close failure is
reported first with the removal's refusal beside it

