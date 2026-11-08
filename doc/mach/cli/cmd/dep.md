# mach.cli.cmd.dep

## val LIST

```mach
pub val LIST: std_args.Command = std_args.Command;
```

`mach dep list`

## val ADD

```mach
pub val ADD: std_args.Command = std_args.Command;
```

`mach dep add`

## val REMOVE

```mach
pub val REMOVE: std_args.Command = std_args.Command;
```

`mach dep remove`

## val UPDATE

```mach
pub val UPDATE: std_args.Command = std_args.Command;
```

`mach dep update`

## val PULL

```mach
pub val PULL: std_args.Command = std_args.Command;
```

`mach dep pull`

## val OUTDATED

```mach
pub val OUTDATED: std_args.Command = std_args.Command;
```

`mach dep outdated`

## val VERIFY

```mach
pub val VERIFY: std_args.Command = std_args.Command;
```

`mach dep verify`

## val ADD_POLICY

```mach
pub val ADD_POLICY: args.Policy = args.Policy;
```

what mach holds `mach dep add` to beyond the parse

## val UPDATE_POLICY

```mach
pub val UPDATE_POLICY: args.Policy = args.Policy;
```

what mach holds `mach dep update` to beyond the parse

## val VERIFY_POLICY

```mach
pub val VERIFY_POLICY: args.Policy = args.Policy;
```

what mach holds `mach dep verify` to beyond the parse

## val POLICY

```mach
pub val POLICY: args.Policy = args.Policy;
```

what mach holds `mach dep` and its other actions to beyond the parse

## val COMMAND

```mach
pub val COMMAND: std_args.Command = std_args.Command;
```

`mach dep`

## fun run

```mach
pub fun run(cx: *args.Call) res[i64, fail.Fail];
```

`mach dep`: route to one action: list, add, remove, update, pull, verify or outdated, each a
command over the dependency manager (mach.lang.package) that renders what it reports

cx: the call, its action read and its operands counted
ret: exit.OK, the code `exit.of` maps the action's rendered failure to, or the failure that
     kept the action from starting

