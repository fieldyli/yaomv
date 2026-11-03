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

        $rows = array_map(function ($field) {
            return ["name" => $field[0], "value" => $field[1]];
        }, $fields);

        // Workbench datatable pages expect the standard OMV list envelope.
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
