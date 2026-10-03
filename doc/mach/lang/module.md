# mach.lang.module

## rec ModuleId

```mach
pub rec ModuleId;
```

a module's index in the session's module table

## val MODULE_NIL

```mach
pub val MODULE_NIL: ModuleId = ModuleId;
```

## rec StableModuleId

```mach
pub rec StableModuleId;
```

a module's identity across sessions, keyed by its fully qualified name

## val STABLE_MODULE_NIL

```mach
pub val STABLE_MODULE_NIL: StableModuleId = StableModuleId;
```

## fun id

```mach
pub fun id(index: u32) ModuleId;
```

## fun index

```mach
pub fun index(m: ModuleId) u32;
```

## fun same

```mach
pub fun same(left: ModuleId, right: ModuleId) bool;
```

## fun is_nil

```mach
pub fun is_nil(m: ModuleId) bool;
```

## fun stable_id

```mach
pub fun stable_id(index: u32) StableModuleId;
```

## fun stable_index

```mach
pub fun stable_index(m: StableModuleId) u32;
```

## fun stable_same

```mach
pub fun stable_same(left: StableModuleId, right: StableModuleId) bool;
```

## fun stable_is_nil

```mach
pub fun stable_is_nil(m: StableModuleId) bool;
```

