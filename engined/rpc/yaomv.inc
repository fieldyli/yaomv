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
            "busy" => "Синхронизация",
            "index" => "Индексирование",
            "idle" => "Синхронизировано",
            "paused" => "Приостановлено",
            "error" => "Ошибка синхронизации",
            "no internet access" => "Нет подключения"
        ];

        if ($service !== "active") {
            $summary = "Служба: " . ($service ?: "unknown");
        } else {
            $summary = $labels[$state] ?? ($state === "N/A" ? "Статус недоступен" : $state);
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
