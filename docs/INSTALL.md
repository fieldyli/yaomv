# Installation and maintenance (experimental alpha9)

> yaomv is an independent community project, not an official Yandex or OpenMediaVault package. **Use at your own risk.** Refer to [README](../README.md) and [GPL-3.0](../LICENSE) for warranty/liability information.

## 1. Prerequisites

- OpenMediaVault **8** on Debian **13**.
- Existing Linux `yandex-disk` client installed, authorized, configured and able to report status.
- Systemd service **`yandex-disk.service`** exists and manages the client.
- Root shell and functioning APT, PHP CLI, `dpkg-deb` and `curl` or an alternative archive downloader.
- Working access to the OMV admin web interface.

Check before proceeding:

```sh
cat /etc/os-release
dpkg-query -W openmediavault yandex-disk
command -v yandex-disk
systemctl is-active yandex-disk.service
yandex-disk status | head -12
```

**Do not paste or publish credential/token files, authorization output, or private filenames from production diagnostics.** These are operational precautions, not a prerequisite to installing the plugin.

The plugin does not install or configure `yandex-disk` or its systemd service. Set those up separately using the vendor's instructions.

## 2. Preserve a rollback path

Before upgrading, **retain a known-working previous `.deb`** in a safe place. If you have an existing installed version, you can record it and make an optional backup of the files managed by the plugin:

```sh
dpkg-query -W openmediavault-yaomv 2>/dev/null || true
mkdir -p /root/yaomv-rollback
cp -a /usr/share/openmediavault/engined/rpc/yaomv.inc /root/yaomv-rollback/ 2>/dev/null || true
cp -a /usr/share/openmediavault/workbench/dashboard.d/yaomv_status.yaml /root/yaomv-rollback/ 2>/dev/null || true
```

These copies are **not** a replacement for a rollback Debian package or for your system backup. Do not manually restore package-owned files unless recovery requires it.

## 3. Obtain source and build alpha9

```sh
cd /tmp
curl -fsSL https://github.com/fieldyli/yaomv/archive/refs/heads/main.tar.gz -o yaomv.tar.gz
mkdir -p yaomv-build
tar -xzf yaomv.tar.gz -C yaomv-build
cd yaomv-build/yaomv-main

# Confirm this is still the desired version; main can move.
grep '^VERSION=' packaging/build-deb.sh
php -l engined/rpc/yaomv.inc
python3 -m unittest discover -s tests -v
sh packaging/build-deb.sh

dpkg-deb --info dist/openmediavault-yaomv_0.1.0~alpha9_all.deb
dpkg-deb --contents dist/openmediavault-yaomv_0.1.0~alpha9_all.deb
```

Because `main` changes over time, **verify the VERSION and inspect the source before installing**. For reproducibility, use a specific reviewed Git commit/archive rather than a moving branch when possible. This is a source build; no binary release is promised.

## 4. Simulate then install

```sh
apt -s install ./dist/openmediavault-yaomv_0.1.0~alpha9_all.deb
```

Review the complete simulation. For an upgrade, the only package to upgrade should be `openmediavault-yaomv`, with no unplanned removals or extra system upgrades. For a first installation, APT should propose the plugin plus only explicitly understood prerequisites.

If the plan is safe:

```sh
apt install ./dist/openmediavault-yaomv_0.1.0~alpha9_all.deb
```

**Side effect:** the package requests OMV's `update-workbench` and `restart-engined` triggers. During their processing, OMV rebuilds its UI and restarts `openmediavault-engined`. The plugin does **not** start, stop, restart, or modify `yandex-disk.service` or the synchronization directory.

## 5. Validate installation

```sh
dpkg-query -W openmediavault-yaomv
php -l /usr/share/openmediavault/engined/rpc/yaomv.inc
omv-rpc -u admin Yaomv getOverview '{}'
omv-rpc -u admin Yaomv getDashboardUsage '{}'
omv-rpc -u admin Yaomv getStatus '{}'
systemctl is-active openmediavault-engined
systemctl is-active yandex-disk.service
```

Open **Services → Yandex Disk** in OMV. If the dashboard widget is not shown, enable **Yandex Disk** in Dashboard settings. If the interface displays an outdated page or a route error, try a private/incognito window or refresh the Workbench cache before modifying system files.

**Expected:** a read-only English overview of service state and cloud quota, optional error details, and a compact dashboard widget with cloud-capacity progress bar. The bar must not be confused with overall sync completion.

## 6. Rollback or uninstall

To roll back, **install your previously retained, known-working `.deb`** after inspecting the APT simulation:

```sh
apt -s install /absolute/path/to/previous/openmediavault-yaomv_0.1.0~alpha8_all.deb
apt install /absolute/path/to/previous/openmediavault-yaomv_0.1.0~alpha8_all.deb
```

The paths above are **examples**; substitute the real local path and version. Do not run until the file exists and the APT simulation has been reviewed. APT may treat downgrades specially and request confirmation.

To remove only the plugin:

```sh
apt -s remove openmediavault-yaomv
apt remove openmediavault-yaomv
```

Review the simulated package changes first. OMV may rebuild Workbench and restart its RPC engine. Removing the plugin **should not uninstall `yandex-disk`**, but inspect APT's exact proposed actions instead of assuming.

## 7. Troubleshooting

- **`No connection` while service is `Active`:** the Yandex Disk client reported the condition; this does not by itself prove all NAS networking is offline. Check `yandex-disk status` and network/DNS connectivity separately. Do not delete cloud files simply to clear the status.
- **`N/A` quota:** the client did not provide a parseable quota. The dashboard may display a 0-width placeholder with `N/A`; this is *unknown*, not confirmed 0% usage.
- **Missing RPC method or Workbench page:** confirm installed version, `systemctl is-active openmediavault-engined`, and UI cache. Do not blindly restart unrelated services.
- **APT wants to upgrade a kernel or remove unrelated packages:** stop and resolve repository/dependency issues first.
- **Client uses a different systemd service name:** this pre-alpha package assumes `yandex-disk.service`; it is not yet configurable.
- **Error paths on Services page:** filenames may contain sensitive information; sharing screenshots or clipboard output is your decision.

## 8. Compatibility and safety

Validated only on the [documented test system](TESTING.md). Pre-alpha code is **not production-certified**; use maintenance windows, backups, and rollback packages. No client credentials are required by the plugin; never put credentials into GitHub issues or support logs.
