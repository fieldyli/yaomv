# yaomv

**Unofficial Yandex Disk status integration for OpenMediaVault 8**

[![License: GPL-3.0](https://img.shields.io/badge/License-GPL--3.0-blue.svg)](LICENSE)
![Status: experimental](https://img.shields.io/badge/status-experimental-orange)
![Version](https://img.shields.io/badge/version-0.1.0--alpha9-blue)

**Latest verified build:** `openmediavault-yaomv 0.1.0~alpha9` — experimental, manually packaged, read-only.

> **Independent community project.** yaomv is **not affiliated with, endorsed by, sponsored by, or supported by Yandex or the OpenMediaVault project, their developers, or their respective teams**. Yandex Disk, Yandex, and OpenMediaVault are trademarks of their respective owners. This plugin is maintained independently, without official support from either project.

> **Disclaimer — use at your own risk.** The software is provided **"AS IS", WITHOUT WARRANTY OF ANY KIND**, to the extent permitted by applicable law. The author and contributors accept **no liability for loss of data, service interruption, security issues, hardware damage, or other direct or indirect losses arising from its use**, to the extent permitted by law. See [GNU GPL v3, sections 15–17](LICENSE). Back up important data and configurations before installing experimental software.

## What it does

yaomv integrates the **existing Linux `yandex-disk` command-line client** into the OpenMediaVault 8 (Synchrony) web interface.

- **Services → Yandex Disk:** read-only service state, synchronization state, cloud usage, and conditional error details; auto-refreshes every 30 seconds.
- **Dashboard → Yandex Disk:** compact, English-language sync status, used/total cloud quota, and a native horizontal capacity bar.
- **Capacity warning:** bar turns red at 90% usage; textual `Warning` appears at 70–89% and `Critical` at 90% or above.
- **Security:** RPC methods require an OMV administrator; the backend executes only fixed read-only commands with timeouts.

The bar displays **cloud storage capacity used**, **not overall synchronization progress**. Yandex Disk's per-operation percentage is not a reliable overall completion indicator.

## Requirements

- **OpenMediaVault 8 / Debian 13**, with the standard Workbench and RPC engine.
- An **already installed, authenticated, and configured** `yandex-disk` Linux command-line client.
- A systemd unit named **`yandex-disk.service`**, which must be configured and managed independently of this plugin.
- Root access to build/install the Debian package.

The plugin does **not** install or authorize the Yandex Disk account, configure the sync directory, or create the systemd service. It is not an official OMV plugin repository package.

## Installation

There is currently **no published apt repository or prebuilt official package**. Build the experimental `.deb` from the repository and inspect the APT plan before installation.

```sh
# On your OMV 8 system, as root:
command -v yandex-disk
systemctl status yandex-disk.service --no-pager

cd /tmp
curl -fsSL https://github.com/fieldyli/yaomv/archive/refs/heads/main.tar.gz -o yaomv.tar.gz
mkdir -p yaomv-build
tar -xzf yaomv.tar.gz -C yaomv-build
cd yaomv-build/yaomv-main

php -l engined/rpc/yaomv.inc
sh packaging/build-deb.sh
dpkg-deb --info dist/openmediavault-yaomv_0.1.0~alpha9_all.deb
apt -s install ./dist/openmediavault-yaomv_0.1.0~alpha9_all.deb
```

Only proceed if APT proposes to install/upgrade **`openmediavault-yaomv`** without unexpected package removals or system upgrades:

```sh
apt install ./dist/openmediavault-yaomv_0.1.0~alpha9_all.deb

dpkg-query -W openmediavault-yaomv
omv-rpc -u admin Yaomv getOverview '{}'
omv-rpc -u admin Yaomv getDashboardUsage '{}'
systemctl is-active openmediavault-engined
systemctl is-active yandex-disk.service
```

Then open **Services → Yandex Disk** and enable the **Yandex Disk** widget in Dashboard settings as needed. A private/incognito browser window can help when OMV Workbench caches the previous UI.

**Note:** installation/upgrade/removal requests OMV's `update-workbench` and `restart-engined` dpkg triggers. OMV rebuilds its Workbench configuration and **restarts `openmediavault-engined`**. The plugin does not itself restart the Yandex Disk service. Avoid updating during an OMV maintenance-sensitive window.

See [INSTALL.md](docs/INSTALL.md) for prerequisites, safe upgrade, rollback, troubleshooting, and uninstall instructions.

## Tested environment

The **alpha9** package, RPC methods and both Workbench views were manually verified on the following **single** NAS environment:

| Component | Tested configuration |
| --- | --- |
| OpenMediaVault | **8.6.0-1 (Synchrony)** |
| Distribution | **Debian 13 (Trixie)** |
| Kernel | **7.0.14-26-pve** (Proxmox kernel) |
| Yandex Disk Linux client | **0.1.6.1081** |
| Hardware | **HP ProLiant MicroServer Gen8**, Xeon E3-1260L, 16 GB RAM |
| Storage | ZFS pool; monitoring does not require ZFS |
| Package | **`openmediavault-yaomv 0.1.0~alpha9`** |

Validated: package upgrade from `alpha8`, PHP syntax, APT install simulation, read-only RPC responses, English Dashboard and Services views, service health, and normal cloud quota reporting. Error diagnostics have been implemented but **not fully browser-validated in every failure condition**. Other OS releases, kernels, CPU architectures, and Yandex Disk versions have **not** been confirmed compatible.

See [TESTING.md](docs/TESTING.md) for scope and known limitations.

## Current limitations

- **Read-only.** No account setup, OAuth, start/stop/restart, pause/resume, upload/download controls, or filesystem operations.
- Requires a pre-existing Yandex Disk client and matching systemd service name.
- Connectivity states such as `No connection` reflect what the client reports; the plugin does not diagnose or repair the network.
- Private error paths may appear to **authenticated OMV administrators** in the Services diagnostics view and may be copied using the UI's copy button.
- No broadly validated compatibility matrix, packaged repository, or release automation.
- Package installation can temporarily interrupt **OMV management RPC** while it restarts; it is not intended to interrupt data synchronization.

## Development roadmap

See [ROADMAP.md](docs/ROADMAP.md) for milestones: reliable diagnostics, tests/CI, installation packaging, broader compatibility validation, optional configuration overview, and carefully gated service controls.

## Development

```sh
python3 -m unittest discover -s tests -v
php -l engined/rpc/yaomv.inc
php tests/test_rpc.php
sh packaging/build-deb.sh
```

`src/yaomv/status.py` is an early standalone Python status parser/probe used for tests. The installed OMV integration uses **PHP RPC** in `engined/rpc/yaomv.inc` plus native Workbench YAML manifests.

Project documentation: [Installation](docs/INSTALL.md) · [Testing](docs/TESTING.md) · [Roadmap](docs/ROADMAP.md) · [RPC contract](docs/rpc-contract.md) · [Build details](docs/build-and-test.md).

## License

Released under **GNU GPL-3.0-only**. See [LICENSE](LICENSE). The license includes warranty and liability provisions; this project does not provide an additional guarantee or official support.

---

## Русский

**yaomv** — независимый, неофициальный плагин мониторинга клиента Яндекс Диска для **OpenMediaVault 8**. Не связан с командами Yandex и OpenMediaVault и не поддерживается ими. Текущая версия **0.1.0~alpha9** — экспериментальная: на одном NAS проверены установка, Dashboard и страница Services. Интерфейс плагина — на английском.

Программа предоставляется **«как есть»**, без гарантий. Автор и участники не несут ответственности за возможный ущерб или потерю данных в пределах, допускаемых законодательством. Перед использованием рекомендуется резервное копирование. Инструкции по установке, проверкам и откату приведены выше.
