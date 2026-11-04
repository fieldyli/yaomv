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

- No clean-install validation on an empty disposable OMV 8 VM.
- No end-to-end remove/purge and downgrade regression testing.
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
