# yaomv

**Yandex Disk integration for OpenMediaVault 8**

[![License: GPL-3.0](https://img.shields.io/badge/License-GPL--3.0-blue.svg)](LICENSE)

**Status:** early development (pre-alpha). This repository does **not** yet contain an installable OpenMediaVault plugin.

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

**Do not install this prototype onto a production OMV system as an OMV plugin.** Packaging, OMV RPC integration, error handling and GUI permissions must be reviewed before deployment.

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

В первой версии планируется страница **Службы → Яндекс Диск**, статус синхронизации, квота и виджет панели управления. **Пока опубликована только безопасная заготовка с CLI-модулем чтения статуса; полноценного устанавливаемого плагина ещё нет.**

Никаких личных путей, конфигурации, токенов или данных синхронизации в репозиторий не добавляем.
