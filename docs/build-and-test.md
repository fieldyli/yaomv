# Build, package and test

**Current source-build version: `0.1.0~alpha9`.** Experimental, unofficial, read-only OpenMediaVault 8 Yandex Disk status plugin.

See [INSTALL.md](INSTALL.md) for complete installation, upgrade, rollback, and uninstall steps and [TESTING.md](TESTING.md) for the precise manual test environment. See also the independent-project statement and warranty disclaimer in [README](../README.md).

## Build prerequisites

An OMV 8 / Debian 13 installation with `php`, `dpkg-deb`, `apt`, and the **already installed and configured** `yandex-disk` client. An isolated/disposable test VM is recommended.

```sh
php -l engined/rpc/yaomv.inc
python3 -m unittest discover -s tests -v
sh packaging/build-deb.sh
dpkg-deb --info dist/openmediavault-yaomv_0.1.0~alpha9_all.deb
dpkg-deb --contents dist/openmediavault-yaomv_0.1.0~alpha9_all.deb
apt -s install ./dist/openmediavault-yaomv_0.1.0~alpha9_all.deb
```

**The build step alone does not install anything or restart a service.** The package is created in `dist/`.

## Package contents

- OMV RPC backend: `/usr/share/openmediavault/engined/rpc/yaomv.inc`.
- Workbench navigation: `workbench/navigation.d/services.yaomv.yaml`.
- Workbench route: `workbench/route.d/services.yaomv.yaml`.
- Native English Services text page: `workbench/component.d/omv-services-yaomv-datatable-page.yaml` (the historical filename is retained, but its component **type is textPage**).
- Dashboard: `workbench/dashboard.d/yaomv_status.yaml` (native datatable with quota usage progress bar).

The package does not contain or modify Yandex Disk login tokens, client configuration, or synchronized data.

## Installation side effects

`postinst` and `postrm` request OMV's `update-workbench` and `restart-engined` triggers. OMV's own trigger handling rebuilds Workbench and restarts the **OMV management RPC engine**, not the Yandex Disk client. Plan the change for a suitable maintenance window.

Only proceed after reading the complete `apt -s install` proposal. Keep an older known-working package for rollback.

## Post-install checks

```sh
dpkg-query -W openmediavault-yaomv
omv-rpc -u admin Yaomv getOverview '{}'
omv-rpc -u admin Yaomv getDashboardUsage '{}'
omv-rpc -u admin Yaomv getStatus '{}'
systemctl is-active openmediavault-engined
systemctl is-active yandex-disk.service
```

Verify **Services → Yandex Disk** and the **Yandex Disk** Dashboard widget in a private/incognito browser session.

## Known limitations

The `getProgress` RPC remains available for compatibility, but the abandoned alpha3 large gauge was removed. The working Dashboard bar shows **cloud quota usage**, not transfer or synchronization completion. A 70% text warning and a 90% red bar are supported by current OMV native components.

The project does not yet have a published APT repository, a prebuilt official binary release, a broad compatibility matrix, or a full clean-install/remove regression suite. Error states should be tested with synthetic fixtures or in a disposable environment, never by deleting real data to trigger them.
