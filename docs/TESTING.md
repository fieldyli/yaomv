# Verified platform and test status

## Confirmed manual test configuration

**Build:** `openmediavault-yaomv 0.1.0~alpha9` (source-built Debian package; pre-alpha).

| Item | Configuration |
| --- | --- |
| NAS | HP ProLiant MicroServer Gen8 |
| CPU / RAM | Intel Xeon E3-1260L / 16 GB |
| OpenMediaVault | **8.6.0-1 (Synchrony)** |
| OS | **Debian 13 (Trixie)** |
| Kernel | **7.0.14-26-pve** |
| Yandex Disk client | **0.1.6.1081** |
| Init/service | systemd, `yandex-disk.service` |
| Storage | ZFS pool, healthy during tests |
| Browser | Microsoft Edge private/InPrivate window |
| Interface | English UI labels, read-only RPC integration |

The Proxmox kernel and ZFS are features of the test NAS; the plugin is not supposed to require either.

## Tests actually performed

- PHP syntax check with `php -l engined/rpc/yaomv.inc`.
- Packaging with `sh packaging/build-deb.sh` and inspection/simulation of local `.deb` upgrades.
- Upgrade from `0.1.0~alpha8` to `0.1.0~alpha9`; APT simulation listed **only** `openmediavault-yaomv` for upgrade and no removals.
- Verified version via `dpkg-query -W openmediavault-yaomv`.
- Verified `Yaomv.getOverview`, `Yaomv.getDashboardUsage`, and `Yaomv.getStatus` via `omv-rpc` (previous iterations).
- Browser verification of **Services → Yandex Disk** and Dashboard widget in OMV 8.6.0-1.
- Verified English messages, the native quota bar, and `No connection` and `Up to date` sync-state representations.
- Confirmed OMV RPC engine and Yandex Disk systemd services active after upgrade; ZFS pool healthy.

These are **manual observations on one system**, not an automated compatibility certification. Existing Python unit tests exercise the standalone status parser; they do not fully test the production PHP RPC + Workbench UI.

## Known gaps

- Clean install, remove, and reinstall have been manually verified on a separate OMV 8 VM (details below); **purge and downgrade remain untested**.
- No end-to-end purge and downgrade regression testing.
- Error diagnostics are implemented, but not all error-state presentations have been browser-tested.
- No tests confirming behavior across Debian releases other than 13, alternative kernels, different service names, architectures, client versions, or restrictive environments.
- No systematic network outage/failure-injection testing.
- No performance/load benchmarks for prolonged operation.
- No external security audit, release-signing process, or public binary package feed.

Please report reproducible issues **without tokens, credentials, or private file paths**. Include the plugin version, OMV version, Yandex Disk client version, a redacted error description, and clear reproduction steps.

## Release status

`alpha9` is the **latest manually verified baseline**, not a stable, officially supported or endorsed release. Changes on `main` after this baseline may not have undergone the same validations.

## Standalone PHP regression suite (added after alpha9 verification)

The repository includes `tests/test_rpc.php`, a self-contained mock of OMV's PHP service and process interfaces. It tests status/quotas, client failures, authorization and UI response shapes without connecting to a NAS or Yandex account.

Run with `php -l engined/rpc/yaomv.inc && php tests/test_rpc.php`. GitHub Actions has been configured to invoke this suite and build the Debian package on pushes and pull requests. **Passing CI has not yet been independently confirmed here**; the suite does not replace a clean OMV integration test.

## Standalone regression results — 2026-11-04

Manually executed on the existing OMV 8 NAS after the PHP test harness was added:

- `php -l engined/rpc/yaomv.inc` — no syntax errors.
- `php -l tests/test_rpc.php` — no syntax errors.
- `php tests/test_rpc.php` — **26 checks, 0 failures**.
- `python3 -m unittest discover -s tests -p 'test_*.py' -v` — **3 tests, OK**.

This confirms the standalone mock-based regression checks ran successfully in this environment; it does **not** validate a clean installation on a disposable OMV 8 VM, all external API failures, or GitHub Actions execution.

## Separate Hyper-V VM clean-install and reinstall tests — 2026-11-04

A separate Windows 11 Pro / Hyper-V Generation 2 VM running **Debian 13.7, OpenMediaVault 8.6.0-1 (Synchrony), amd64** was used for clean-install testing. The VM has an isolated test environment and a dedicated LAN connection. Yandex Disk Linux client **0.1.6.1081** was installed **without account authorization or `yandex-disk.service`**.

Validated on this VM:

1. Built package `openmediavault-yaomv_0.1.0~alpha9_all.deb` from source; `php -l` passed; `apt -s install` proposed exactly one package.
2. First installation completed; OMV's package triggers restarted engined and rebuilt Workbench. Package state became installed; `getOverview` and `getDashboardUsage` RPCs worked; engined and nginx remained active.
3. Native Services page and Dashboard widget were **visually verified** in the OMV browser UI. The Services page showed the expected unconfigured-client diagnostic `Error: option 'dir' is missing`. A transient `#/404` was seen at first and disappeared after reopening the page in a new browser session; root cause not established.
4. `apt -s remove` proposed removal of **only** `openmediavault-yaomv`. After actual `apt remove`, package state was `rc` (removed, config-files metadata retained), package-owned RPC and Workbench YAML files were gone, `dpkg --audit` was clean, and engined/nginx remained active. The separate `yandex-disk` package remained installed.
5. Reinstalled the same alpha9 `.deb`; package state returned to `ii`, both UI RPC methods returned expected data, engined/nginx stayed active, and the **Services and Dashboard pages reopened successfully in a browser**.

**Not yet tested:** `apt purge`, downgrade between alpha versions, clean install with a fully configured/authorized Yandex Disk on this VM, or sustained network fault conditions. Production NAS remains untouched by these VM tests.
