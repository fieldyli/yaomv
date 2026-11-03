"""Unit tests for the read-only status parser.

SPDX-License-Identifier: GPL-3.0-only
"""
import pathlib
import sys
import unittest

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parents[1] / "src"))
from yaomv.status import parse_status


class ParseStatusTests(unittest.TestCase):
    def test_busy_with_quota(self):
        output = """Sync progress: 15.22 KB/ 3.00 GB (0 %)
Synchronization core status: busy
Path to Yandex.Disk directory: '/srv/data/YandexDisk'
        Total: 1.04 TB
        Used: 271.36 GB
        Available: 796.64 GB
"""
        self.assertEqual(parse_status(output), {
            "sync_state": "busy",
            "directory": "/srv/data/YandexDisk",
            "progress": "15.22 KB/ 3.00 GB (0 %)",
            "quota_total": "1.04 TB",
            "quota_used": "271.36 GB",
            "quota_available": "796.64 GB",
        })

    def test_offline(self):
        self.assertEqual(parse_status(
            "Synchronization core status: no internet access\n"
            "The quota has not been received yet.\n"
        ), {"sync_state": "no internet access"})

    def test_empty(self):
        self.assertEqual(parse_status(""), {})


if __name__ == "__main__":
    unittest.main()
