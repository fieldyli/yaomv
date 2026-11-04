<?php
/** Standalone PHP RPC tests: no real OMV, network or Yandex Disk client. */
// SPDX-License-Identifier: GPL-3.0-only
namespace OMV\Rpc {
    class ServiceAbstract {
        public function registerMethod($name) {}
        public function validateMethodContext($context, $requirements) {
            if (($context["role"] ?? null) !== "admin") {
                throw new \RuntimeException("admin required");
            }
        }
    }
}
namespace OMV\System {
    class Process {
        public static $service = "active";
        public static $status = "";
        public static $fail = false;
        public static $calls = [];
        private $args;
        public function __construct($command, $args) {
            if ($command !== "/usr/bin/timeout") throw new \RuntimeException("unexpected executable");
            self::$calls[] = $args;
            $this->args = $args;
        }
        public function setQuiet($quiet) {}
        public function execute(&$output) {
            $binary = $this->args[3] ?? null;
            if ($binary === "/usr/bin/systemctl") {
                $output = [self::$service];
            } elseif ($binary === "/usr/bin/yandex-disk") {
                if (self::$fail) throw new \RuntimeException("simulated timeout");
                $output = explode("\n", self::$status);
            } else {
                throw new \RuntimeException("unexpected binary");
            }
        }
    }
}
namespace {
    define("OMV_ROLE_ADMINISTRATOR", "admin");
    require dirname(__DIR__) . "/engined/rpc/yaomv.inc";
    use OMV\System\Process;
    $svc = new \Engined\Rpc\Yaomv();
    $admin = ["role" => "admin"];
    $checks = 0;
    $failures = 0;
    function assertSameValue($actual, $expected, $label) {
        global $checks, $failures;
        ++$checks;
        if ($actual !== $expected) {
            ++$failures;
            fwrite(STDERR, "FAIL $label: expected " . json_encode($expected)
                . ", received " . json_encode($actual) . "\n");
        }
    }
    function assertContainsText($needle, $haystack, $label) {
        assertSameValue(strpos($haystack, $needle) !== false, true, $label);
    }
    function mockStatus($state, $withQuota = true, $service = "active") {
        Process::$service = $service;
        Process::$fail = false;
        Process::$calls = [];
        Process::$status = "Synchronization core status: $state\n"
            . ($withQuota ? "        Total: 1.04 TB\n"
                . "        Used: 271.36 GB\n        Available: 796.64 GB\n" : "");
    }
    mockStatus("idle");
    $overview = $svc->getOverview([], $admin);
    assertContainsText("## Service status", $overview, "overview heading");
    assertContainsText("Up to date", $overview, "idle label");
    assertContainsText("26.1%", $overview, "overview capacity");
    assertSameValue(strpos($overview, "## Diagnostics") === false, true, "no false error");
    $data = $svc->getDashboardUsage([], $admin);
    assertSameValue($data["total"], 1, "dashboard envelope");
    assertSameValue($data["data"][0]["usagePercent"], 26.1, "quota calculation");
    assertSameValue($data["data"][0]["cloud"], "271.36 GB / 1.04 TB", "quota label");
    assertSameValue($svc->getStatus([], $admin)["total"], 6, "normal rows");

    mockStatus("busy");
    Process::$status = "Sync progress: 0 B/ 2.65 KB (0 %)\n" . Process::$status;
    assertSameValue($svc->getDashboardUsage([], $admin)["data"][0]["sync"], "Syncing", "busy");
    assertContainsText("Current operation", $svc->getOverview([], $admin), "operation details");

    mockStatus("index");
    assertSameValue($svc->getDashboardUsage([], $admin)["data"][0]["sync"], "Indexing", "index");
    mockStatus("paused");
    assertSameValue($svc->getDashboardUsage([], $admin)["data"][0]["sync"], "Paused", "paused");
    mockStatus("no internet access");
    assertSameValue($svc->getDashboardUsage([], $admin)["data"][0]["sync"], "No connection", "offline");
    mockStatus("error");
    Process::$status .= "Error: server failure\nPath: 'folder/secret|file.md'\n";
    $overview = $svc->getOverview([], $admin);
    assertContainsText("## Diagnostics", $overview, "error section");
    assertContainsText("server failure", $overview, "error message");
    assertContainsText("secret\\|file.md", $overview, "markdown-safe path");
    assertSameValue($svc->getStatus([], $admin)["total"], 8, "error rows");

    mockStatus("idle", false);
    $row = $svc->getDashboardUsage([], $admin)["data"][0];
    assertSameValue($row["usageText"], "N/A", "unknown quota label");
    assertSameValue($row["usagePercent"], 0, "unknown quota placeholder");
    mockStatus("idle", true, "inactive");
    assertSameValue($svc->getDashboardUsage([], $admin)["data"][0]["sync"], "Service: inactive", "stopped service");

    mockStatus("idle");
    Process::$fail = true;
    assertSameValue($svc->getDashboardUsage([], $admin)["data"][0]["usageText"], "N/A", "timeout fallback");
    mockStatus("idle");
    try {
        $svc->getOverview([], ["role" => "guest"]);
        assertSameValue(true, false, "reject non-admin");
    } catch (\RuntimeException $e) {
        assertSameValue(true, true, "reject non-admin");
    }
    $svc->getDashboardUsage([], $admin);
    foreach (Process::$calls as $args) {
        assertSameValue(in_array($args[3], ["/usr/bin/systemctl", "/usr/bin/yandex-disk"], true),
            true, "fixed binaries");
        assertSameValue($args[0], "-k", "kill timeout argument");
    }
    echo "PHP RPC checks: $checks; failures: $failures\n";
    exit($failures === 0 ? 0 : 1);
}
