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
