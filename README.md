# yaomv

**Yandex Disk integration for OpenMediaVault 8**

[![License: GPL-3.0](https://img.shields.io/badge/License-GPL--3.0-blue.svg)](LICENSE)

**Status:** experimental pre-alpha. A manually built Debian plugin was installed and its read-only status page + RPC verified on OMV 8.6.0-1; the new progress gauge is not yet browser-tested.

## Goal

Provide a native **Services → Yandex Disk** page and an optional dashboard widget in OpenMediaVault 8 (Synchrony), showing the existing Linux Yandex Disk client's service state, synchronization state, progress and quota.

## Scope for v0.1.0

- Read-only integration with an **existing** `yandex-disk` installation and `yandex-disk.service`
- No OAuth setup, file deletion, sync triggers or modifications to the sync directory
- No transfer of passwords, tokens, configuration files or personal files to the OMV web UI
- OMV Workbench UI + authenticated OMV RPC backend (to be implemented)
- Debian package name planned: `openmediavault-yaomv`

## Present prototype

`src/yaomv/status.py` is a read-only status probe using Python's standard library. It prints JSON and performs only:

- `systemctl is-active yandex-disk.service`
- `yandex-disk status`

It does **not** read the Yandex Disk authentication token or start/stop synchronization.

```bash
python3 src/yaomv/status.py
python3 -m unittest discover -s tests -v
```

**Caution:** this is an experimental project, not a production-supported package. Back up OMV Workbench/RPC files and test in an isolated VM where possible. Installing or upgrading the Debian package triggers an OMV RPC engine restart and Workbench rebuild.

## Latest experimental package

Build from source with `sh packaging/build-deb.sh`. The current alpha5 retains the compact two-value Dashboard widget and adds readable synchronization states plus error details on the Services page. The working Services status page remains unchanged. See [build instructions](docs/build-and-test.md). Inspect the package and preserve the previous working alpha4 build before upgrading.

## Development plan

1. Confirm Workbench navigation, page, dashboard and RPC conventions against OMV 8 installed examples.
2. Implement a read-only authenticated RPC method that wraps the status probe.
3. Create native Workbench YAML UI and dashboard widget.
4. Add Debian packaging, integration tests and CI; test on a disposable OMV 8 VM first.
5. Add explicitly confirmed service controls in a later release.

## Security

Never commit `passwd`, `config.cfg`, `.sync`, logs, credentials or personal paths. The Yandex Disk data directory must never be modified by this prototype.

## License

GPL-3.0-only. See [LICENSE](LICENSE).

---

## Русский

**yaomv** — разрабатываемый плагин для веб-интерфейса OpenMediaVault 8, предназначенный для мониторинга уже установленного консольного клиента Яндекс Диска.

Экспериментальная версия alpha2 уже прошла первый тест на OMV 8.6.0-1: страница **Службы → Яндекс Диск** и RPC работают. В alpha5 компактный виджет показывает понятные состояния синхронизации; на странице службы дополнительно отображаются причина ошибки и путь проблемного файла, если клиент их сообщает.

Никаких личных путей, конфигурации, токенов или данных синхронизации в репозиторий не добавляем.
