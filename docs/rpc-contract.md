# yaomv RPC contract (draft, pre-alpha)

The OMV Workbench UI definitions in `workbench/` are **not installable yet**.
They require an authenticated OMV RPC backend.

## Endpoint

- RPC service: `Yaomv`
- Method: `getStatus`
- Parameters: none
- Return: the OMV datatable envelope `{total, data}`. The `data` array contains rows (`name`, `value`).

Example response (synthetic; no user data):

```json
{
  "total": 4,
  "data": [
    {"name": "Service", "value": "active"},
    {"name": "Sync state", "value": "busy"},
    {"name": "Progress", "value": "15 KB / 3 GB (0%)"},
    {"name": "Used quota", "value": "271 GB"}
  ]
}
```

## Progress gauge endpoint

- RPC service: `Yaomv`
- Method: `getProgress`
- Parameters: none
- Returns: `{"progressPercent": 82}` for reported progress, or `{"progressPercent": null}` if the client reports no percentage.
- Administrator role required. This is read-only and does not alter synchronization.

The gauge is in `workbench/dashboard.d/yaomv_progress.yaml`; the existing dashboard status grid and the working status table are unchanged.

## Security requirements

- Only authenticated, authorized OMV administrators may invoke the method.
- Back end executes allowlisted read-only binaries without a shell and with timeouts.
- Do not expose credentials, environment variables, complete process command lines, private tokens or config contents.
- Redact/avoid synchronization directory paths in shared screenshots/telemetry and public test fixtures.
- Any unavailability should surface a clear error without retry loops that could burden the NAS.
- No sync/start/stop/resume commands in v0.1.0.

## Workbench integration

`workbench/navigation.d/services.yaomv.yaml` defines the menu.
`workbench/route.d/services.yaomv.yaml` connects URL and component.
`workbench/component.d/omv-services-yaomv-datatable-page.yaml` consumes `Yaomv.getStatus`.

The dashboard widget is intentionally deferred until the RPC endpoint and its permissions work in a disposable OMV 8 test environment.

## Compact Dashboard endpoint (alpha4)

`Yaomv.getDashboardStatus` returns a `{total, data}` list with two rows: `Sync` (current state and percentage when reported) and `Cloud` (used / total quota). The existing `Yaomv.getStatus` powers the full Services page; `getProgress` remains available for compatibility but the incorrect oversized gauge was removed from the package.

## alpha5 error fields

`Yaomv.getStatus` keeps its existing `{total, data}` format and conditionally adds rows `Error` and `Error path` when the client's status output contains an error. The `Error path` may contain private filenames and is visible only in the admin-only Services page. The compact Dashboard never shows the error path; its `Sync` row has a short localized state. An absent progress percentage is not represented as completed work.

## alpha6 cloud quota widget

`Yaomv.getDashboardUsage` returns a single-row OMV list envelope containing `sync` (short state), `cloud` (human-readable used/total), `usagePercent` (numeric 0–100), `usageText` (overlay percentage / warning). If quota values cannot be parsed, `usagePercent` is 0 and `usageText` is `N/A`; do not interpret that as 0% usage. No filenames or authentication details are returned to the Dashboard. The widget uses the native OMV `progressBar` cell, which has a green/red warning threshold only.

## alpha8 read-only Services overview

`Yaomv.getOverview` returns a Markdown-formatted string for a native OMV `textPage`; the dashboard and prior RPC methods remain unchanged. The output contains separate sections for service/synchronization state and cloud quota, a current-operation line only if reported by the client, and an error diagnostic section only when an error is reported. Error paths may contain private filenames but are visible only to an authenticated OMV administrator. The returned text is not stored or committed.
