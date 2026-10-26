# mach.provider.git

dependency-specific Git protocol and object primitives

## fwd protocol.Packet

```mach
fwd protocol.Packet
```

forwards [`mach.provider.git.protocol.Packet`](git/protocol.md#rec-packet)

## fwd protocol.Reader

```mach
fwd protocol.Reader
```

forwards [`mach.provider.git.protocol.Reader`](git/protocol.md#rec-reader)

## fwd protocol.next

```mach
fwd protocol.next
```

forwards [`mach.provider.git.protocol.next`](git/protocol.md#fun-next)

## fwd protocol.append_packet

```mach
fwd protocol.append_packet
```

forwards [`mach.provider.git.protocol.append_packet`](git/protocol.md#fun-append_packet)

## fwd protocol.append_flush

```mach
fwd protocol.append_flush
```

forwards [`mach.provider.git.protocol.append_flush`](git/protocol.md#fun-append_flush)

## fwd protocol.apply_delta

```mach
fwd protocol.apply_delta
```

forwards [`mach.provider.git.protocol.apply_delta`](git/protocol.md#fun-apply_delta)

## fwd object.Id

```mach
fwd object.Id
```

forwards [`mach.provider.git.object.Id`](git/object.md#rec-id)

## fwd object.parse_id

```mach
fwd object.parse_id
```

forwards [`mach.provider.git.object.parse_id`](git/object.md#fun-parse_id)

## fwd object.format_id

```mach
fwd object.format_id
```

forwards [`mach.provider.git.object.format_id`](git/object.md#fun-format_id)

## fwd object.object_id

```mach
fwd object.object_id
```

forwards [`mach.provider.git.object.object_id`](git/object.md#fun-object_id)

## fwd pack.Object

```mach
fwd pack.Object
```

forwards [`mach.provider.git.pack.Object`](git/pack.md#rec-object)

## fwd pack.Pack

```mach
fwd pack.Pack
```

forwards [`mach.provider.git.pack.Pack`](git/pack.md#rec-pack)

## fwd pack.decode

```mach
fwd pack.decode
```

forwards [`mach.provider.git.pack.decode`](git/pack.md#fun-decode)

## fwd pack.dnit

```mach
fwd pack.dnit
```

forwards [`mach.provider.git.pack.dnit`](git/pack.md#fun-dnit)

## fwd remote.Advertisement

```mach
fwd remote.Advertisement
```

forwards [`mach.provider.git.remote.Advertisement`](git/remote.md#rec-advertisement)

## fwd remote.discover

```mach
fwd remote.discover
```

forwards [`mach.provider.git.remote.discover`](git/remote.md#fun-discover)

## fwd remote.select

```mach
fwd remote.select
```

forwards [`mach.provider.git.remote.select`](git/remote.md#fun-select)

## fwd remote.request

```mach
fwd remote.request
```

forwards [`mach.provider.git.remote.request`](git/remote.md#fun-request)

## fwd remote.response

```mach
fwd remote.response
```

forwards [`mach.provider.git.remote.response`](git/remote.md#fun-response)

## fwd remote.validate_url

```mach
fwd remote.validate_url
```

forwards [`mach.provider.git.remote.validate_url`](git/remote.md#fun-validate_url)

## fwd https.exchange

```mach
fwd https.exchange
```

forwards [`mach.provider.git.https.exchange`](git/https.md#fun-exchange)

## fwd store.Repository

```mach
fwd store.Repository
```

forwards [`mach.provider.git.store.Repository`](git/store.md#rec-repository)

## fwd store.read_object

```mach
fwd store.read_object
```

forwards [`mach.provider.git.store.read_object`](git/store.md#fun-read_object)

## fwd store.write_object

```mach
fwd store.write_object
```

forwards [`mach.provider.git.store.write_object`](git/store.md#fun-write_object)

## fwd store.write_pack

```mach
fwd store.write_pack
```

forwards [`mach.provider.git.store.write_pack`](git/store.md#fun-write_pack)

## fwd store.TreeEntry

```mach
fwd store.TreeEntry
```

forwards [`mach.provider.git.store.TreeEntry`](git/store.md#rec-treeentry)

## fwd store.tree_next

```mach
fwd store.tree_next
```

forwards [`mach.provider.git.store.tree_next`](git/store.md#fun-tree_next)

## fwd checkout.create

```mach
fwd checkout.create
```

forwards [`mach.provider.git.checkout.create`](git/checkout.md#fun-create)

## rec Fetched

```mach
pub rec Fetched;
```

## fun fetch

```mach
pub fun fetch(a: *A.Allocator, repository: str, selector: str) res[Fetched, str];
```

fetch a selector's full closure and require its exact object to be present

