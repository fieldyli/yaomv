#!/usr/bin/env python3
"""Read-only status probe for an existing Yandex Disk Linux client.

SPDX-License-Identifier: GPL-3.0-only
"""
from __future__ import annotations

import json
import re
import subprocess
import sys


def command_output(argv: list[str], timeout: int = 10) -> str:
    """Run an allowlisted command without a shell; never echo credentials."""
    try:
        result = subprocess.run(argv, capture_output=True, text=True,
                                check=False, timeout=timeout)
    except (OSError, subprocess.TimeoutExpired) as exc:
        raise RuntimeError(f"{argv[0]} unavailable or timed out") from exc
    if result.returncode != 0:
        raise RuntimeError(f"{argv[0]} returned exit code {result.returncode}")
    return result.stdout


def parse_status(output: str) -> dict[str, object]:
    """Parse non-sensitive fields from the Linux client's human-readable status."""
    result: dict[str, object] = {}
    patterns = {
        "sync_state": r"^Synchronization core status:\s*(.+)$",
        "directory": r"^Path to Yandex\.Disk directory:\s*'([^']*)'$",
        "progress": r"^Sync progress:\s*(.+)$",
        "quota_total": r"^\s*Total:\s*(.+)$",
        "quota_used": r"^\s*Used:\s*(.+)$",
        "quota_available": r"^\s*Available:\s*(.+)$",
    }
    for key, pattern in patterns.items():
        match = re.search(pattern, output, re.MULTILINE)
        if match:
            result[key] = match.group(1).strip()
    return result


def collect_status() -> dict[str, object]:
    # No writes, no daemon lifecycle changes and no token/config access.
    service = command_output(["systemctl", "is-active", "yandex-disk.service"]).strip()
    status = parse_status(command_output(["yandex-disk", "status"]))
    return {"service": service, **status}


def main() -> int:
    try:
        print(json.dumps(collect_status(), ensure_ascii=False, sort_keys=True))
        return 0
    except RuntimeError as exc:
        print(json.dumps({"error": str(exc)}, ensure_ascii=False))
        return 1


if __name__ == "__main__":
    sys.exit(main())
