# Experimental Debian package: build and test

**Pre-alpha / not production ready.** The build script creates a package but
does not install it or restart the OMV engine.

## Build on Debian 13 / a disposable OMV 8 VM

```sh
sh packaging/build-deb.sh
dpkg-deb --info dist/openmediavault-yaomv_0.1.0~alpha1_all.deb
dpkg-deb --contents dist/openmediavault-yaomv_0.1.0~alpha1_all.deb
```

## Contents

The package stages `yaomv.inc` in the OMV engined RPC directory and the four
YAML manifests in the appropriate Workbench directories. The post-install hook
requests the `update-workbench` dpkg trigger, following OMV's plugin conventions.
It deliberately does **not** restart `openmediavault-engined` or operate on
`yandex-disk.service`.

The Yandex Disk credentials are never packaged.

## Remaining verification

1. Confirm that the OMV 8 daemon discovers the RPC service on startup.
2. Confirm `Yaomv.getStatus` returns `{total, data}` with role checks.
3. Confirm Workbench datatable and dashboard schemas and cache rebuilding.
4. Add explicit uninstall/upgrade handling if required, and verify clean removal.
5. Test install/upgrade/remove in a disposable VM, then document maintenance impact.

Do **not** install this initial build on a production NAS.
