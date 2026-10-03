# mach.lang.alloc

the compiler's allocation refusal, rendered once. an allocation refusal is not
something the compiler recovers from at the site that met it; it is reported
through the failure domains (fail.refused, outcome.refused) and the build
stops. the text is presentation only and no site branches on it.

## fun text

```mach
pub fun text(e: A.Error) str;
```

the refusal as text: `exhausted` is the message the compiler has always
reported, the other cases name the contract fault they are

