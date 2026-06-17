#!/usr/bin/env bash
# Cut a signed OTA release for BrushlessLamp.
#
#   ./release.sh <semver> <int>      e.g.  ./release.sh 0.15.10 160
#
# Bumps the three version locations, builds the signed image in the no-space
# shadow tree, publishes a GitHub Release (brushlesslamp.bin + manifest.json),
# then commits + pushes the version bump. Deployed lamps poll the release
# manifest and self-update (see s3/firmware/main/ota.cpp). <int> must exceed the
# currently-released version or no device will update.
set -euo pipefail

VER="${1:?usage: release.sh <semver e.g. 0.15.10> <int e.g. 160>}"
NUM="${2:?usage: release.sh <semver> <int>}"
REPO="RoodsBurger/brushless_lamp"
SRC="/Users/rodolfo/Documents/Personal Projects/BrushlessLamp"
SHADOW="$HOME/esp/brushlesslamp-s3"

# Toolchain: anaconda python3.12 for idf.py, homebrew for gh.
export PATH="/opt/anaconda3/bin:/opt/homebrew/bin:$PATH"
source ~/esp/esp-idf-v5.5.4/export.sh >/dev/null
source ~/esp/esp-matter/export.sh   >/dev/null

echo "==> bump version -> $VER ($NUM)"
sed -i '' "s/set(PROJECT_VER \".*\")/set(PROJECT_VER \"$VER\")/"            "$SRC/s3/firmware/CMakeLists.txt"
sed -i '' "s/set(PROJECT_VER_NUMBER .*)/set(PROJECT_VER_NUMBER $NUM)/"      "$SRC/s3/firmware/CMakeLists.txt"
sed -i '' "s|OTA_FW_VERSION *= [0-9]*;.*|OTA_FW_VERSION        = $NUM;                 // = PROJECT_VER $VER|" "$SRC/s3/common/config.h"
sed -i '' "s/CONFIG_DEVICE_SOFTWARE_VERSION_NUMBER=.*/CONFIG_DEVICE_SOFTWARE_VERSION_NUMBER=$NUM/" "$SRC/s3/firmware/sdkconfig.defaults"

echo "==> build (signed) in $SHADOW"
rsync -a --delete \
  --exclude=build --exclude=managed_components --exclude=dependencies.lock \
  --exclude=sdkconfig --exclude=sdkconfig.old --exclude=mfg_out \
  "$SRC/s3/" "$SHADOW/"
# rm sdkconfig so the bumped DEVICE_SOFTWARE_VERSION_NUMBER default takes effect.
( cd "$SHADOW/firmware" && rm -f sdkconfig && idf.py build )

echo "==> write manifest"
cat > "$SHADOW/firmware/build/manifest.json" <<EOF
{ "version": $NUM, "version_str": "$VER", "url": "https://github.com/$REPO/releases/latest/download/brushlesslamp.bin" }
EOF

echo "==> publish GitHub release fw-$VER"
gh release create "fw-$VER" \
  "$SHADOW/firmware/build/brushlesslamp.bin" \
  "$SHADOW/firmware/build/manifest.json" \
  --repo "$REPO" --title "fw-$VER" --notes "BrushlessLamp firmware $VER (signed OTA)."

echo "==> commit + push version bump"
cd "$SRC"
git add s3/firmware/CMakeLists.txt s3/common/config.h s3/firmware/sdkconfig.defaults
git commit -m "S3: release fw-$VER (OTA version $NUM)"
git push origin main

echo "==> done — lamps self-update to $VER on next boot, or within 5 days."
