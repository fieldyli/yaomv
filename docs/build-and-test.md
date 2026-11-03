# Experimental Debian package: build and test

**Pre-alpha / not production ready.** The build script creates a package but
does not install it or restart the OMV engine.

## Build on Debian 13 / a disposable OMV 8 VM

```sh
sh packaging/build-deb.sh
dpkg-deb --info dist/openmediavault-yaomv_0.1.0~alpha9_all.deb
dpkg-deb --contents dist/openmediavault-yaomv_0.1.0~alpha9_all.deb
```

## Contents

The package stages `yaomv.inc` in the OMV engined RPC directory and the four
YAML manifests in the appropriate Workbench directories. The post-install and removal hooks request the verified `update-workbench` and
`restart-engined` dpkg triggers. During installation or removal, OMV's own
trigger handler rebuilds Workbench and **restarts openmediavault-engined**.
The package does not start, stop or modify `yandex-disk.service`.

The Yandex Disk credentials are never packaged.

## alpha3 updates

- Adds a second, numerical read-only `Yaomv.getProgress` RPC method.
- Adds the optional dashboard progress gauge `yaomv_progress.yaml`.
- Keeps the existing status page and status grid unchanged.

Before upgrading a production installation, confirm the local status page works and retain the previous `.deb` as a rollback package. Installing a new version triggers OMV Workbench rebuild and RPC engine restart. The gauge has not yet been tested in a browser.

## Remaining verification

1. Confirm that the OMV 8 daemon discovers the RPC service on startup.
2. Confirm `Yaomv.getStatus` returns `{total, data}` with role checks.
3. Confirm Workbench datatable and dashboard schemas and cache rebuilding.
4. Add explicit uninstall/upgrade handling if required, and verify clean removal.
5. Test install/upgrade/remove in a disposable VM, then document maintenance impact.

Do **not** install this initial build on a production NAS.

## alpha4 dashboard correction

The alpha3 large synchronization gauge was removed because it displayed 0% even when the RPC reported nonzero progress. The remaining Dashboard grid now calls `Yaomv.getDashboardStatus` and shows only two concise values: sync state with progress percentage, and used/total cloud capacity. The working Services page remains unchanged. This is still a test build: verify the compact widget in a private browser window after upgrading. Previously enabled gauge preferences might need to be disabled in Dashboard settings.

## alpha5: sync state and error reporting

- Dashboard: localized short states for `busy`, `index`, `idle`, `paused`, `error`, and `no internet access`.
- Dashboard: suppress misleading `0%` shown while starting a new operation; positive percentages are *per-operation*, not overall sync completion.
- Services page: conditionally append `Error` and `Error path` from the client's `yandex-disk status` output, if the error is reported.
- No changes to Yandex Disk configuration, service lifecycle, authentication, or synchronized files.
- Upgrade triggers restart of the OMV RPC engine and Workbench rebuild. Verify with `Yaomv.getStatus` and `Yaomv.getDashboardStatus` before checking the browser in a private window.

## alpha6 native cloud usage progress bar

- Replaces the 2-cell grid Dashboard widget with a single-row OMV datatable (same widget ID) containing sync state, cloud size and a horizontal quota usage progress bar.
- Implements `Yaomv.getDashboardUsage` with fields `sync`, `cloud`, `usagePercent`, `usageText`.
- Progress indicates **percentage of cloud storage capacity used**, not file synchronization progress. Decimal units from `yandex-disk status` are converted to bytes before calculating the percentage.
- Uses OMV's native `progressBar` cell template, which is **green below 90% and red from 90%**. At 70–89% the overlay text says `Внимание`, and from 90% `Критично`. OMV has no built-in amber third color for this template; no custom CSS or JavaScript is injected.
- Existing `getStatus`, `getProgress` and `getDashboardStatus` RPC methods remain available; the Services status page is unchanged.
- Verify the Dashboard in a browser after installing: the widget should show one row with three columns and a colored bar. On narrow screens, the dashboard's datatable layout may be less compact than the old grid. Roll back to alpha5 if undesirable.

## alpha7 compact Dashboard layout

- Changes only the Dashboard YAML and package version; no PHP RPC or synchronized-file changes.
- Hides the table header (`hasHeader: false`).
- Displays status and quota in one two-line `template` cell (`{{ sync }}<br>{{ cloud }}`) with 3 flex units; gives the native `progressBar` 5 flex units.
- Dashboard still uses `Yaomv.getDashboardUsage` and its warning threshold of 90%. Text warnings still start at 70%.
- A full-width stacked layout is not supported by the generic Dashboard datatable; this is the native, low-risk compromise. Validate the visual result in a private browser window and roll back to alpha6 if needed.

## alpha8 Services overview

- Keeps the alpha7 Dashboard YAML and existing RPC endpoints intact.
- Reuses the existing Services component ID and route; changes its type from `datatablePage` to native `textPage` with `contentFormat: markdown`.
- Adds admin-only read-only RPC method `Yaomv.getOverview` returning a Markdown string with separate state and cloud quota tables, optional current-operation status, and optional diagnostic error/path.
- Automatically refreshes the Services page every 30 seconds and enables the native copy-to-clipboard button for diagnostics.
- Values from the Yandex Disk process are Markdown-escaped, and rendered using OMV's built-in sanitized Markdown view; no custom HTML, JavaScript, external assets or write operations.
- Test `omv-rpc -u admin Yaomv getOverview '{}'` before opening Services → Yandex Disk in a private browser window. Roll back using alpha7 if the Workbench page does not load.

## alpha9: English language and clearer Services layout

- Changes no synchronization settings and adds no service actions.
- Replaces Services Markdown tables with headings, bullet lines and section separators, with status indicators and conditional diagnostics. The page remains an OMV native `textPage` and refreshes every 30 seconds.
- Uses English for all plugin-generated sync states in Dashboard and Services: Syncing, Indexing, Up to date, Paused, Sync error, No connection. Cloud quota warnings read Warning/Critical.
- Dashboard layout, progress bar and existing RPC contracts are unchanged. Existing error messages and filenames come verbatim from `yandex-disk status` and are escaped before display.
- As always, syntax-check with `php -l engined/rpc/yaomv.inc`, build, simulate apt install, and only then upgrade.
