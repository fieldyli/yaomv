<?php
/**
 * yaomv: read-only Yandex Disk status for OpenMediaVault 8.
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

namespace Engined\Rpc;

class Yaomv extends \OMV\Rpc\ServiceAbstract
{
    public function getName()
    {
        return "Yaomv";
    }

    public function initialize()
    {
        $this->registerMethod("getStatus");
        $this->registerMethod("getProgress");
        $this->registerMethod("getDashboardStatus");
        $this->registerMethod("getDashboardUsage");
        $this->registerMethod("getOverview");
    }

    /**
     * Return read-only data rows for the Workbench datatable.
     * Never reads credentials or modifies synchronization settings.
     */
    public function getStatus($params, $context)
    {
        $this->validateMethodContext($context, [
            "role" => OMV_ROLE_ADMINISTRATOR
        ]);

        $service = trim($this->readCommand(
            ["/usr/bin/systemctl", "is-active", "yandex-disk.service"],
            4
        ));
        if ($service === "") {
            $service = "unknown";
        }

        $output = $this->readCommand(
            ["/usr/bin/yandex-disk", "status"],
            12
        );

        $fields = [
            ["Service", $service],
            ["Sync state", $this->findValue($output, '/^Synchronization core status:\s*(.+)$/m')],
            ["Progress", $this->findValue($output, '/^Sync progress:\s*(.+)$/m')],
            ["Total quota", $this->findValue($output, '/^\s*Total:\s*(.+)$/m')],
            ["Used quota", $this->findValue($output, '/^\s*Used:\s*(.+)$/m')],
            ["Available quota", $this->findValue($output, '/^\s*Available:\s*(.+)$/m')]
        ];

        $error = $this->findValue($output, '/^Error:\s*(.+)$/m');
        $path = $this->findValue($output, '/^Path:\s*\x27([^\x27]*)\x27\s*$/m');
        if ($error !== "N/A") {
            $fields[] = ["Error", $error];
            if ($path !== "N/A") {
                $fields[] = ["Error path", $path];
            }
        }

        $rows = array_map(function ($field) {
            return ["name" => $field[0], "value" => $field[1]];
        }, $fields);

        // Workbench datatable pages expect the standard OMV list envelope.
        return ["total" => count($rows), "data" => $rows];
    }

    /**
     * Dashboard gauge: return numeric progress, or null if the Yandex
     * Disk client did not report a percentage. Read-only and admin-only.
     */
    public function getProgress($params, $context)
    {
        $this->validateMethodContext($context, [
            "role" => OMV_ROLE_ADMINISTRATOR
        ]);

        $output = $this->readCommand(
            ["/usr/bin/yandex-disk", "status"],
            12
        );
        $progress = $this->findValue(
            $output,
            '/^Sync progress:\\s*(.+)$/m'
        );
        $percent = null;
        if (preg_match('/\\(\\s*(\\d{1,3})\\s*%\\s*\\)/', $progress, $matches)) {
            $percent = min(100, max(0, (int) $matches[1]));
        }

        return ["progressPercent" => $percent];
    }

    /**
     * Compact dashboard: a short state and quota summary. Never displays
     * filenames, directory paths or the misleading 0% of a new operation.
     */
    public function getDashboardStatus($params, $context)
    {
        $this->validateMethodContext($context, [
            "role" => OMV_ROLE_ADMINISTRATOR
        ]);

        $service = trim($this->readCommand(
            ["/usr/bin/systemctl", "is-active", "yandex-disk.service"], 4
        ));
        $status = $this->readCommand(
            ["/usr/bin/yandex-disk", "status"], 12
        );
        $state = $this->findValue(
            $status, '/^Synchronization core status:\s*(.+)$/m'
        );
        $progress = $this->findValue(
            $status, '/^Sync progress:\s*(.+)$/m'
        );

        $labels = [
            "busy" => "Syncing",
            "index" => "Indexing",
            "idle" => "Up to date",
            "paused" => "Paused",
            "error" => "Sync error",
            "no internet access" => "No connection"
        ];

        if ($service !== "active") {
            $summary = "Service: " . ($service ?: "unknown");
        } else {
            $summary = $labels[$state] ?? ($state === "N/A" ? "Status unavailable" : $state);
            // The client's percentage tracks its current operation, not total sync.
            // Suppress 0%, which caused confusion during operation transitions.
            if (in_array($state, ["busy", "index"], true) &&
                preg_match('/\(\s*(\d{1,3})\s*%\s*\)/', $progress, $m)) {
                $percentage = min(100, (int) $m[1]);
                if ($percentage > 0) {
                    $summary .= " · " . $percentage . "%";
                }
            }
        }

        $used = $this->findValue($status, '/^\s*Used:\s*(.+)$/m');
        $total = $this->findValue($status, '/^\s*Total:\s*(.+)$/m');
        $quota = ($used === "N/A" || $total === "N/A")
            ? "—" : $used . " / " . $total;

        $rows = [
            ["name" => "Sync", "value" => $summary],
            ["name" => "Cloud", "value" => $quota]
        ];
        return ["total" => count($rows), "data" => $rows];
    }

    /**
     * One-row dashboard data for OMV's native datatable progressBar.
     * Percentage reflects cloud quota usage, NOT sync task progress.
     */
    public function getDashboardUsage($params, $context)
    {
        $this->validateMethodContext($context, [
            "role" => OMV_ROLE_ADMINISTRATOR
        ]);

        $service = trim($this->readCommand(
            ["/usr/bin/systemctl", "is-active", "yandex-disk.service"], 4
        ));
        $output = $this->readCommand(
            ["/usr/bin/yandex-disk", "status"], 12
        );
        $state = $this->findValue(
            $output, '/^Synchronization core status:\s*(.+)$/m'
        );
        $labels = [
            "busy" => "Syncing",
            "index" => "Indexing",
            "idle" => "Up to date",
            "paused" => "Paused",
            "error" => "Sync error",
            "no internet access" => "No connection"
        ];
        $sync = $service === "active"
            ? ($labels[$state] ?? ($state === "N/A" ? "Status unavailable" : $state))
            : "Service: " . ($service ?: "unknown");

        $used = $this->findValue($output, '/^\s*Used:\s*(.+)$/m');
        $total = $this->findValue($output, '/^\s*Total:\s*(.+)$/m');
        $usedBytes = $this->quotaToBytes($used);
        $totalBytes = $this->quotaToBytes($total);
        $percentage = null;
        if ($usedBytes !== null && $totalBytes !== null && $totalBytes > 0) {
            $percentage = round(min(100, max(0, 100 * $usedBytes / $totalBytes)), 1);
        }

        $quota = ($used === "N/A" || $total === "N/A")
            ? "Unavailable" : $used . " / " . $total;
        $usageText = $percentage === null ? "N/A" : $percentage . "%";
        if ($percentage !== null && $percentage >= 90) {
            $usageText = "Critical: " . $usageText;
        } elseif ($percentage !== null && $percentage >= 70) {
            $usageText = "Warning: " . $usageText;
        }
        $data = [[
            "sync" => $sync,
            "cloud" => $quota,
            "usagePercent" => $percentage ?? 0,
            "usageText" => $usageText
        ]];
        return ["total" => 1, "data" => $data];
    }

    /** Convert human-readable decimal or binary Yandex Disk quotas to bytes. */
    private function quotaToBytes($value)
    {
        if (!preg_match('/^(\d+(?:[.,]\d+)?)\s*(B|KB|MB|GB|TB|PB|KiB|MiB|GiB|TiB|PiB)$/i', $value, $m)) {
            return null;
        }
        $units = ["B" => 0, "KB" => 1, "MB" => 2, "GB" => 3, "TB" => 4, "PB" => 5,
                  "KIB" => 1, "MIB" => 2, "GIB" => 3, "TIB" => 4, "PIB" => 5];
        $unit = strtoupper($m[2]);
        $base = strpos($unit, "I") !== false ? 1024 : 1000;
        return (float) str_replace(",", ".", $m[1]) * pow($base, $units[$unit]);
    }

    /**
     * Read-only English Markdown overview for OMV textPage.
     * No Markdown tables: they are too tightly spaced in OMV Workbench.
     * All Yandex Disk output is escaped before formatting.
     */
    public function getOverview($params, $context)
    {
        $this->validateMethodContext($context, [
            "role" => OMV_ROLE_ADMINISTRATOR
        ]);

        $service = trim($this->readCommand(
            ["/usr/bin/systemctl", "is-active", "yandex-disk.service"], 4
        ));
        $status = $this->readCommand(
            ["/usr/bin/yandex-disk", "status"], 12
        );
        $state = $this->findValue(
            $status, '/^Synchronization core status:\s*(.+)$/m'
        );
        $labels = [
            "busy" => "Syncing",
            "index" => "Indexing",
            "idle" => "Up to date",
            "paused" => "Paused",
            "error" => "Sync error",
            "no internet access" => "No connection"
        ];
        $serviceLabel = $service === "active"
            ? "Active" : ($service === "" ? "Unknown" : $service);
        $syncLabel = $labels[$state] ?? ($state === "N/A" ? "Status unavailable" : $state);
        $progress = $this->findValue(
            $status, '/^Sync progress:\s*(.+)$/m'
        );
        $total = $this->findValue($status, '/^\s*Total:\s*(.+)$/m');
        $used = $this->findValue($status, '/^\s*Used:\s*(.+)$/m');
        $available = $this->findValue($status, '/^\s*Available:\s*(.+)$/m');

        $totalBytes = $this->quotaToBytes($total);
        $usedBytes = $this->quotaToBytes($used);
        $usage = "Unavailable";
        if ($totalBytes !== null && $totalBytes > 0 && $usedBytes !== null) {
            $usage = number_format(
                min(100, max(0, 100 * $usedBytes / $totalBytes)), 1, ".", ""
            ) . "%";
        }
        $serviceIndicator = $service === "active" ? "🟢" : "🔴";
        $syncIndicator = $state === "idle" ? "🟢"
            : (in_array($state, ["busy", "index", "paused"], true) ? "🟡"
            : (in_array($state, ["error", "no internet access"], true) ? "🔴" : "⚪"));

        $lines = [
            "## Service status",
            "",
            "- **Yandex Disk service:** " . $serviceIndicator . " " . $this->escapeMarkdown($serviceLabel),
            "- **Synchronization:** " . $syncIndicator . " " . $this->escapeMarkdown($syncLabel)
        ];
        if ($progress !== "N/A" && in_array($state, ["busy", "index"], true)) {
            $lines[] = "- **Current operation:** " . $this->escapeMarkdown($progress);
        }

        $lines = array_merge($lines, [
            "",
            "---",
            "",
            "## Cloud storage",
            "",
            "- **Used:** " . $this->escapeMarkdown($used),
            "- **Available:** " . $this->escapeMarkdown($available),
            "- **Total:** " . $this->escapeMarkdown($total),
            "- **Capacity used:** " . $this->escapeMarkdown($usage)
        ]);

        $error = $this->findValue($status, '/^Error:\s*(.+)$/m');
        if ($error !== "N/A") {
            $path = $this->findValue(
                $status, '/^Path:\s*\x27([^\x27]*)\x27\s*$/m'
            );
            $lines = array_merge($lines, [
                "",
                "---",
                "",
                "## Diagnostics",
                "",
                "- **Error:** " . $this->escapeMarkdown($error)
            ]);
            if ($path !== "N/A") {
                $lines[] = "- **Affected item:** " . $this->escapeMarkdown($path);
            }
        }
        $lines[] = "";
        $lines[] = "*Current-operation progress is not the overall synchronization percentage.*";
        return implode("\n", $lines);
    }

    /** Escape untrusted client output before embedding it in Markdown. */
    private function escapeMarkdown($value)
    {
        $value = str_replace(["\r", "\n"], " ", (string) $value);
        return strtr($value, [
            "\\" => "\\\\", "*" => "\\*", "_" => "\\_",
            "[" => "\\[", "]" => "\\]", "|" => "\\|",
            "<" => "&lt;", ">" => "&gt;"
        ]);
    }

    private function findValue($output, $pattern)
    {
        if (preg_match($pattern, $output, $matches)) {
            return trim($matches[1]);
        }
        return "N/A";
    }

    /**
     * The command and arguments are internal fixed values; no user input,
     * no shell, and the process is constrained by GNU timeout.
     */
    private function readCommand($argv, $seconds)
    {
        $args = array_merge(
            ["-k", "2s", strval($seconds) . "s"],
            $argv
        );
        try {
            $cmd = new \OMV\System\Process("/usr/bin/timeout", $args);
            $cmd->setQuiet(true);
            $cmd->execute($output);
            return implode("\n", $output ?? []);
        } catch (\Throwable $e) {
            return "";
        }
    }
}
