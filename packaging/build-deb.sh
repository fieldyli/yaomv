#!/bin/sh
# Build an experimental yaomv .deb without installing anything.
# SPDX-License-Identifier: GPL-3.0-only
set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
VERSION="0.1.0~alpha3"
PKG="openmediavault-yaomv"
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT HUP INT TERM

STAGE="$TMP/$PKG"
mkdir -p "$STAGE/DEBIAN" \
  "$STAGE/usr/share/openmediavault/engined/rpc" \
  "$STAGE/usr/share/openmediavault/workbench/navigation.d" \
  "$STAGE/usr/share/openmediavault/workbench/route.d" \
  "$STAGE/usr/share/openmediavault/workbench/component.d" \
  "$STAGE/usr/share/openmediavault/workbench/dashboard.d"

install -m 0644 "$ROOT/engined/rpc/yaomv.inc" \
  "$STAGE/usr/share/openmediavault/engined/rpc/yaomv.inc"
for section in navigation.d route.d component.d dashboard.d; do
  for file in "$ROOT/workbench/$section/"*.yaml; do
    [ -f "$file" ] || continue
    install -m 0644 "$file" \
      "$STAGE/usr/share/openmediavault/workbench/$section/$(basename "$file")"
  done
done

cat > "$STAGE/DEBIAN/control" <<EOF
Package: $PKG
Version: $VERSION
Section: admin
Priority: optional
Architecture: all
Maintainer: fieldyli <https://github.com/fieldyli>
Depends: openmediavault (>= 8), yandex-disk, coreutils, systemd
Description: Experimental Yandex Disk status interface for OpenMediaVault 8
 Read-only status page and dashboard for the installed Yandex Disk client.
 This is a pre-alpha build intended for isolated testing only.
EOF

cat > "$STAGE/DEBIAN/postinst" <<'EOF'
#!/bin/sh
set -e
case "$1" in
  configure)
    dpkg-trigger update-workbench
    dpkg-trigger restart-engined
    # OMV's trigger handler performs the Workbench rebuild and RPC engine restart.
    ;;
esac
exit 0
EOF
chmod 0755 "$STAGE/DEBIAN/postinst"

cat > "$STAGE/DEBIAN/postrm" <<'EOF'
#!/bin/sh
set -e
case "$1" in
  remove|purge)
    dpkg-trigger update-workbench
    dpkg-trigger restart-engined
    ;;
esac
exit 0
EOF
chmod 0755 "$STAGE/DEBIAN/postrm"

mkdir -p "$ROOT/dist"
dpkg-deb --build --root-owner-group "$STAGE" \
  "$ROOT/dist/${PKG}_${VERSION}_all.deb"
echo "Built $ROOT/dist/${PKG}_${VERSION}_all.deb"
echo "EXPERIMENTAL: build only; not validated for production installation."
