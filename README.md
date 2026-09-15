<div align="center">

**English** · [Français](README.fr.md)

<img src=".github/banner.svg" alt="Chappe 05, meshcore-chappe05, community LoRa firmware" width="100%">

<br>

[![Status](https://img.shields.io/badge/status-beta-F0A32A)](https://github.com/und3rX1337/meshcore-chappe05)
[![Build repeaters](https://github.com/und3rX1337/meshcore-chappe05/actions/workflows/build-repeater-firmwares.yml/badge.svg)](https://github.com/und3rX1337/meshcore-chappe05/actions/workflows/build-repeater-firmwares.yml)
[![Site](https://img.shields.io/badge/site-chappe05.fr-4BAEDF)](https://chappe05.fr)
[![Fork of](https://img.shields.io/badge/fork%20of-MeshCore-8794D4)](https://github.com/meshcore-dev/MeshCore)

**MeshCore firmware fork for the Chappe 05 community LoRa mesh (Hautes-Alpes, France): per-packet-type filtering, QoS and observability for relays.**

</div>

---

> 🚧 **Beta.** The network is being built and the per-class settings are not yet field-proven. Commands, defaults and builds may change between versions.

## 📡 The project

Chappe 05 is a community LoRa network being built around Gap, in the French Hautes-Alpes, named after Claude Chappe's optical telegraph: a chain of towers on hilltops, each visible from the next, relaying a message from one to the next. That is the network's topology, two centuries later, on the 868 MHz band.

This repository tracks MeshCore upstream and adds a network-preservation layer, meant for a shared mesh where a single mis-configured relay can disrupt everyone. The goal is to feed these functions back into the official firmware, not to maintain a parallel fork.

## 🛡️ What this fork adds

| Feature | Command | Purpose |
|---|---|---|
| Per-type hop caps | `set flood.max.type <type> <hops>` | limit one payload type's reach without touching the others |
| Per-type priority | `set prio.type <type> <penalty>` | make a type yield under queue contention |
| Per-type airtime budget | `set airtime.budget.type <type> <percent>` | cap a type's airtime share, as a percentage of the duty-cycle budget |
| Channel data flooding | `set grp.data.block`, `set grp.data.allow` | drop flooded GRP_DATA, re-open per channel |
| Filter observability | `stats-filter` | count dropped packets, by reason |
| Pinned radio preset | `LORA_CR=8` | align a fresh install on the network preset 869.618 MHz, 62.5 kHz, SF8, CR8 |

The per-neighbour block list (`block.add` and friends) and the passive relay confirmation are the work of Fabrice Crohas; the per-type filtering layer, the QoS and the counters were added by Chappe 05.

## 🧰 Online tools

Everything runs from the browser, nothing to install, over WebSerial.

| Tool | Address | Purpose |
|---|---|---|
| Flasher | ⚡ [chappe05.fr/flasher](https://chappe05.fr/flasher/) | install the firmware and configure a relay by category |
| Test bench | 🧪 [chappe05.fr/banc](https://chappe05.fr/banc/) | forge, transmit and observe packets, measure filtering |
| Relay policy | 📖 [chappe05.fr/politique-relais](https://chappe05.fr/politique-relais/) | relay classes, regions, recommended settings |

## 🚀 Getting started

The simplest path is the [online flasher](https://chappe05.fr/flasher/): it installs a pre-built image and configures the board.

To build it yourself, this repository is a [PlatformIO](https://platformio.org/) project, like MeshCore upstream. The full library documentation, board list and build instructions are maintained upstream: see [meshcore-dev/MeshCore](https://github.com/meshcore-dev/MeshCore). Images are also produced by the workflows in the [Actions](https://github.com/und3rX1337/meshcore-chappe05/actions) tab.

## 🤝 Contributing

Found a bug, or have an observation: open an [issue](https://github.com/und3rX1337/meshcore-chappe05/issues/new). A fix or an improvement: open a pull request. Operator feedback is welcome, the field is what makes the settings evolve.

## 🙏 Upstream and credits

Based on [MeshCore](https://github.com/meshcore-dev/MeshCore) by Scott Powell (`meshcore-dev`), a lightweight C++ multi-hop routing library for LoRa. The upstream code keeps its original license and authors.
