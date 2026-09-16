# Per-type packet filtering (uniform model)

A repeater decides whether to forward a flood packet from the **header alone**, without
ever decrypting the payload. MeshCore carries sixteen payload types, and the repeater can
already shape most of them per type. But one control is asymmetric: only a single type,
`GRP_DATA`, has a real on/off block. Every other type has only a distance cap that never
fully stops it.

This is an accident of history, not a design choice. This document specifies a **uniform
model**: a type is a type, and every type exposes the same set of levers. The special
handling of one type becomes a single default in a general grid.

## Current state

For all sixteen types:

- `flood.max.type <t> <n>` : per-type hop cap (distance).
- `prio.type <t> <n>` : per-type priority under load.
- `airtime.budget.type <t> <n>` : per-type airtime share.

For the single type `GRP_DATA` (6) only:

- `grp.data.block <0|1>` : hard block (drop at zero hops).
- `grp.data.allow <hash>` : per-channel allow-list exception.

Per node (not per type):

- `block.add/remove/clear/list`, `block.lasthop`, `block.alltypes` : the blocklist.

### Why `flood.max.type` is not a block

`isFloodHopLimitExceeded()` in `src/helpers/RoutingPolicy.h` tests
`flood_max_type[t] != 0 && hops >= flood_max_type[t]`. So:

- value `0` means **inherit `flood.max`**, not "blocked";
- the smallest useful cap, `1`, still forwards a fresh packet once (`hops 0 >= 1` is false).

There is therefore no value of `flood.max.type` that fully stops a type. Only a true block
does that, and today only `GRP_DATA` has one.

## The flood pipeline

Order of checks for a flood packet (`isRouteFlood()` true). Direct packets bypass all of it
and follow their recorded path; administration of the node is a separate path, gated by the
ACL (`setperm`).

```
filterRecvFloodPacket():
  1. per-type block        grp.data.block / grp.data.allow   -> drop [grp_data]   (type 6 only, today)
  2. blocklist             ADVERT by public key, GRP_TXT/GRP_DATA (or all, if block.alltypes) by path hop
                                                             -> drop [blocklist]
  3. airtime budget        airtime.budget.type[t]            -> drop [airtime]
allowPacketForward():
  4. hop cap               flood.max / .unscoped / .advert / .type[t] -> drop [hopcap]
  5. region scope          unknown transport code            -> drop
  6. loop detection        loop.detect                       -> drop
  7. send queue            full                              -> drop [qfull]
  -> forwarded
```

## Proposal: generalize block and allow to every type

Two commands, applied to any type index:

| Command | Role |
|---|---|
| `set block.type <0-15> <0\|1>` | hard block for any payload type (drop at zero hops) |
| `set allow.type <0-15> <id>` | allow-list exception, for types that carry a cleartext discriminator |

With `get block.type` and `get allow.type`, matching the existing `get` conventions.

### The allow-list needs a cleartext discriminator

An allow-list can only re-admit a flow if the packet exposes an identifier the repeater can
read **without decrypting**. Not every type carries one. This is the only real constraint,
and it is physical, not arbitrary.

| Type | Cleartext discriminator | `allow.type` meaningful |
|---|---|---|
| `GRP_TXT` (5), `GRP_DATA` (6) | channel hash (`payload[0]`) | yes, per channel |
| `ADVERT` (4) | sender public key | yes, per node (this is the blocklist) |
| `REQ`, `RESPONSE`, `ACK`, `TXT_MSG`, `PATH`, `CONTROL`, ... | none | no, block is all-or-nothing |

The model stays uniform: the allow-list is simply empty where there is nothing to key on,
and the on/off block applies to every type without exception.

### Backward compatibility

- `grp.data.block` becomes an alias of `block.type 6`, `grp.data.allow` an alias of
  `allow.type 6`. Existing scripts and stored preferences keep working unchanged.
- Defaults are unchanged: only `GRP_DATA` (type 6) is blocked by default, every other type
  passes. Upgrading changes no behaviour.

### Guardrail for structural types

Some types keep the mesh alive: `ADVERT` (4) drives discovery, `PATH` (8) and `ACK` (3)
drive routing, `CONTROL` (11) drives coordination. Blocking them breaks the network.
`block.type` must refuse or warn on these types. `flood.max.type` has no such guard today.

## What this does NOT change

The generalization touches only the `GRP_DATA` block, and even then only to subsume it into
the general form with identical behaviour. Everything else stays as is:

- the blocklist (`block.*`) is untouched;
- the group relay confirmation (`grp.relay.*`) is untouched.

## Validation

Nothing here is tuned by guesswork. Each tightening is measured on the bench by reading the
repeater's `stats-filter` drop counters before and after. A filter that drops nothing is
useless; a filter that drops everything cuts the network. The counters decide.

## Upstreaming

The intent is to propose this model to `meshcore-dev`, not to keep a parallel fork.
