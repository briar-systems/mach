# mach.lang.package.pull

pull: `mach dep pull`, which realizes a project's dependency closure and notes every
package the closure pins at more than one commit

## fun project_closure

```mach
pub fun project_closure(s: *session.Session, a: *A.Allocator, root: str, quiet: bool, rep: *report.Report) err[fail.Fail];
```

realize the dependency closure of a project: acquire every declared dependency
transitively, check each realized manifest's project id against its declared name, and
report realized directories under dep/ that are no longer in the closure and packages
pinned at different commits within it

s: the session; a failure may borrow from it, so the caller shows it before ending it
a: the storage the operation works in
root: the project root directory
quiet: report no progress lines
rep: receives progress, notes, and the effects a failure left
ret: err naming the failure

