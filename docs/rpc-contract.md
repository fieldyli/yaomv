# yaomv RPC interface — 0.1.0~alpha9

The installed OMV backend is **PHP**, in `engined/rpc/yaomv.inc`; the Python status probe in `src/yaomv/status.py` is a separate early diagnostic/test helper. All RPC methods below require an authorized **OMV administrator** and are read-only.

The backend uses fixed, allowlisted commands (not user-controlled shell commands), timeout limits and no credential/config file reads. Client-returned error messages may contain private filenames, which are exposed only to authenticated administrators through the Services page.

## Available methods

| Method | Response | UI consumer |
| --- | --- | --- |
| `Yaomv.getOverview` | Markdown **string** with Service status, Cloud storage, optional Diagnostics | **Services → Yandex Disk** native `textPage` |
| `Yaomv.getDashboardUsage` | `{total: 1, data: [{sync, cloud, usagePercent, usageText}]}` | **Dashboard** |
| `Yaomv.getStatus` | `{total, data: [{name, value}, ...]}` | Legacy/diagnostic status table |
| `Yaomv.getDashboardStatus` | `{total: 2, data: [{name, value}, ...]}` | Legacy compact grid compatibility |
| `Yaomv.getProgress` | `{progressPercent: number | null}` | Legacy gauge compatibility (not used in current UI) |

All methods accept no parameters in the alpha9 UI.

Example (synthetic):

```json
{
  "total": 1,
  "data": [
    {
      "sync": "Up to date",
      "cloud": "250 GB / 1 TB",
      "usagePercent": 25.0,
      "usageText": "25%"
    }
  ]
}
```

- `getDashboardUsage.usagePercent` is the **percentage of cloud storage capacity used** (0–100), **not** the percentage of files synchronized.
- If the client does not provide parseable quota values, `usagePercent` is currently returned as `0` with `usageText: "N/A"`. That is an unknown/unavailable metric, not a confirmed empty cloud.
- The native Workbench `progressBar` remains green below 90% and red from 90%. The text says `Warning` from 70–89%, `Critical` from 90%.
- `getOverview` returns English Markdown headings and bullet lines, with optional diagnostics when the client reports `Error` and an affected path. It may render status symbols. The Workbench text page sanitizes rendered Markdown.
- Source-side sync values include `busy`, `index`, `idle`, `paused`, `error`, `no internet access`; the current interface maps them to `Syncing`, `Indexing`, `Up to date`, `Paused`, `Sync error`, `No connection`.
- A service can be `active` while its synchronization state is `No connection` or `Sync error`. These are different signals, not an inconsistency.

## Workbench integration

- `workbench/navigation.d/services.yaomv.yaml`: menu entry.
- `workbench/route.d/services.yaomv.yaml`: route `/services/yaomv`.
- `workbench/component.d/omv-services-yaomv-datatable-page.yaml`: native `textPage` with 30-second refresh and copy button. The filename reflects an earlier datatable iteration and is kept for compatibility.
- `workbench/dashboard.d/yaomv_status.yaml`: native `datatable` widget with 30-second reload and a quota progress bar.

## Security and operations

- No Yandex Disk credentials/tokens, account authorization, file operations, sync triggers or service lifecycle operations are part of the current RPC API.
- The Services diagnostics view can include affected filenames and may be copied by an administrator; review before publicly sharing.
- Client status and quota reporting depend on the installed Yandex Disk client and can become temporarily stale or unavailable.
- The install/upgrade package restarts the **OMV engined RPC daemon** via standard dpkg triggers; the plugin does not restart `yandex-disk.service`.

For installation, compatibility, limitations, and independent-project status, see [INSTALL.md](INSTALL.md), [TESTING.md](TESTING.md), and [README](../README.md).
