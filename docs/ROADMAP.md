# Development roadmap

This roadmap is provisional, not a promise of dates or future compatibility. The priority is to preserve the stability of a NAS and avoid disrupting an existing Yandex Disk synchronization.

## Current baseline — 0.1.0~alpha9 (completed)

- Native **Services → Yandex Disk** read-only English overview.
- Native Dashboard widget with sync status, cloud quota and horizontal usage bar.
- Optional error/affected-item text shown only in administrator Services view.
- Admin-only RPC endpoints, fixed read-only commands and timeouts.
- Debian source-build packaging and manual update testing on one OMV 8.6.0-1 system.
- Independent-project attribution, license/disclaimer, installation documentation, and tested-environment record.

## Next: monitoring and diagnosis

- Improve `No connection` diagnosis with clear distinction between client-reported sync state, service state and separately verified reachability.
- Report last successful sync / last error **only if the client provides trustworthy timestamps**; avoid manufacturing history.
- Review error-message path escaping/redaction in the web UI and copy-to-clipboard behavior.
- Improve resilience when `yandex-disk status` times out or omits quota/progress values.
- Keep the Dashboard compact and do not mistake per-operation progress for completion.

## Quality and packaging

- Add PHP RPC unit/integration tests with synthetic output fixtures for idle, busy, index, error, offline, missing quota and timeout scenarios.
- Add native Workbench configuration validation and CI coverage for Debian package assembly.
- Test install, upgrade, downgrade, remove and purge in a **disposable OMV 8 VM**.
- Revisit clean packaging, dependencies, maintenance hooks and reproducibility, then consider tagged source releases and checksums.
- Expand the tested matrix to other OMV 8 package revisions, Debian-compatible kernels and Yandex Disk client versions.

## Optional read-only capabilities

- Configuration overview (sync directory, status-relevant options) **without** displaying authentication secrets.
- Service uptime and last state change when obtainable from reliable systemd properties.
- Clear diagnostics and user-facing troubleshooting guidance, without automatic deletion, service restarts or database resets.

## Later, subject to design and explicit approval

- Controlled start, stop and restart actions with administrator authorization, confirmation and an audit trail.
- Potential pause/resume and other lifecycle actions only after safe semantics and impacts have been tested.
- Optional configuration management only if explicit scopes, validation, backup, and rollback are designed.
- Possibly distribute a properly versioned release artifact after sufficient testing.

## Explicitly out of scope for current builds

- OAuth/account authorization and credential management.
- File browsing or deleting cloud files.
- Automatic sync conflict resolution, changes to the synced data tree, or destructive recovery actions.
- Full-featured replacement for the official Yandex Disk Linux client.

**Compatibility and support:** this is an unofficial community project, not supported by Yandex or the OpenMediaVault teams. See the [README](../README.md) for disclaimer and license.
