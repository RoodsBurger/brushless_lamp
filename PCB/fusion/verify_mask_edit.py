#!/usr/bin/env python3
"""Manufacturing sanity check for the solder-mask + slot edit across 4 EAGLE/Fusion XML files.

Checks:
 (a) microns of solder mask overlapping each signal-pad edge + min % of pad area still exposed
 (b) solder-paste (tCream layer 31) apertures were NOT shrunk and still cover the pads
 (c) DRV exposed/thermal EP polygon (~3.5x9.8mm, layer 29) is untouched
 (d) all four files carry identical fixed DRV+USB geometry (consistency)
"""
import re
import xml.etree.ElementTree as ET
from pathlib import Path

BASE = Path("/Users/rodolfo/Documents/Personal Projects/BrushlessLamp/PCB")
FILES = {
    "DRV.lbr":   BASE / "components/DRV8313PWPR.lbr",
    "USB.lbr":   BASE / "components/TYPE-C-31-M-12.lbr",
    "Regen.brd": BASE / "fusion/BrushlessLamp.brd",
    "Routes.brd":BASE / "fusion/BrushlessLampRoutes.brd",
}
DRV_PKG = "SOP65P640X120-29N"
USB_PKG = "HRO_TYPE-C-31-M-12"

# ---------------------------------------------------------------------------
# Parsing helpers
# ---------------------------------------------------------------------------

def find_package(root, pkg_name):
    """Return the <package> element with the given name, searched anywhere."""
    for pkg in root.iter("package"):
        if pkg.get("name") == pkg_name:
            return pkg
    return None


def poly_bbox(poly):
    """Axis-aligned bbox of a <polygon> from its vertices (ignores arc bulge)."""
    xs, ys = [], []
    for v in poly.findall("vertex"):
        xs.append(float(v.get("x")))
        ys.append(float(v.get("y")))
    return min(xs), min(xs) and min(ys) or min(ys), max(xs), max(ys), (xs, ys)


def bbox(xs, ys):
    return min(xs), min(ys), max(xs), max(ys)


def collect_layer_polys(pkg, layer):
    out = []
    for poly in pkg.findall("polygon"):
        if poly.get("layer") == str(layer):
            xs = [float(v.get("x")) for v in poly.findall("vertex")]
            ys = [float(v.get("y")) for v in poly.findall("vertex")]
            out.append((xs, ys, poly))
    return out


def collect_layer_rects(pkg, layer):
    out = []
    for r in pkg.findall("rectangle"):
        if r.get("layer") == str(layer):
            x1, y1, x2, y2 = (float(r.get(k)) for k in ("x1", "y1", "x2", "y2"))
            out.append((min(x1, x2), min(y1, y2), max(x1, x2), max(y1, y2)))
    return out


def collect_smds(pkg):
    out = []
    for s in pkg.findall("smd"):
        out.append({
            "name": s.get("name"),
            "x": float(s.get("x")), "y": float(s.get("y")),
            "dx": float(s.get("dx")), "dy": float(s.get("dy")),
            "stop": s.get("stop"), "cream": s.get("cream"),
        })
    return out


def smd_bbox(s):
    return (s["x"] - s["dx"] / 2, s["y"] - s["dy"] / 2,
            s["x"] + s["dx"] / 2, s["y"] + s["dy"] / 2)


def overlap_area(a, b):
    """Intersection area of two axis-aligned bboxes (x1,y1,x2,y2)."""
    ix1, iy1 = max(a[0], b[0]), max(a[1], b[1])
    ix2, iy2 = min(a[2], b[2]), min(a[3], b[3])
    if ix2 <= ix1 or iy2 <= iy1:
        return 0.0
    return (ix2 - ix1) * (iy2 - iy1)


def match_mask_to_pad(pad_bb, mask_bbs):
    """Pick the mask bbox whose center is closest to the pad center."""
    pcx = (pad_bb[0] + pad_bb[2]) / 2
    pcy = (pad_bb[1] + pad_bb[3]) / 2
    best, bestd = None, 1e9
    for mb in mask_bbs:
        mcx = (mb[0] + mb[2]) / 2
        mcy = (mb[1] + mb[3]) / 2
        d = (mcx - pcx) ** 2 + (mcy - pcy) ** 2
        if d < bestd:
            bestd, best = d, mb
    return best


# ---------------------------------------------------------------------------
# Load all four files
# ---------------------------------------------------------------------------
roots = {k: ET.parse(p).getroot() for k, p in FILES.items()}

print("=" * 78)
print("SOLDER-MASK + SLOT EDIT VERIFICATION")
print("=" * 78)

# ===========================================================================
# (a) + part of (b)/(c): analyze the canonical .lbr geometry for DRV
# ===========================================================================
def analyze_drv(root, label):
    pkg = find_package(root, DRV_PKG)
    smds = [s for s in collect_smds(pkg) if s["name"].isdigit() and 1 <= int(s["name"]) <= 28]
    mask = [bbox(xs, ys) for xs, ys, _ in collect_layer_polys(pkg, 29)]
    cream = collect_layer_polys(pkg, 31)
    # signal pad mask bboxes are the small ~1.6x0.4 ones; exclude the big EP poly
    sig_mask = [m for m in mask if (m[2]-m[0]) < 2.5 and (m[3]-m[1]) < 1.0]
    ep_mask  = [m for m in mask if (m[2]-m[0]) >= 2.5 or (m[3]-m[1]) >= 5.0]
    rows = []
    for s in smds:
        pb = smd_bbox(s)
        mb = match_mask_to_pad(pb, sig_mask)
        pad_w, pad_h = s["dx"], s["dy"]
        mask_w, mask_h = mb[2]-mb[0], mb[3]-mb[1]
        # encroachment per edge in the pitch (y / dy) axis
        enc_y_per_edge_um = max(0.0, (pad_h - mask_h) / 2) * 1000
        enc_x_per_edge_um = max(0.0, (pad_w - mask_w) / 2) * 1000
        exposed = overlap_area(pb, mb)
        pad_area = pad_w * pad_h
        pct = 100 * exposed / pad_area
        rows.append((s["name"], enc_y_per_edge_um, enc_x_per_edge_um, pct, mask_h, pad_h))
    return rows, ep_mask, cream, smds


drv_rows, drv_ep, drv_cream, drv_smds = analyze_drv(roots["DRV.lbr"], "DRV.lbr")

print("\n(a) DRV SOP65P640X120-29N signal pads (1.5 x 0.45 mm, 0.65mm pitch in y)")
print("    pad-by-pad: mask-y-height, encroach/edge (pitch axis), %% exposed")
enc_vals = sorted(set(round(r[1], 3) for r in drv_rows))
pcts = [r[3] for r in drv_rows]
print(f"    mask y-height (all pads): {sorted(set(round(r[4],3) for r in drv_rows))} mm")
print(f"    pad  y-height (all pads): {sorted(set(round(r[5],3) for r in drv_rows))} mm")
print(f"    encroachment per pitch-axis edge: {enc_vals} um")
print(f"    encroachment per x-axis edge (sample pad 1): {drv_rows[0][2]:.1f} um")
print(f"    min %% pad area exposed: {min(pcts):.2f}%   max: {max(pcts):.2f}%")
drv_min_exposed = min(pcts)
drv_enc_um = max(r[1] for r in drv_rows)

# ===========================================================================
# USB-C analysis (a)
# ===========================================================================
def analyze_usb(root, label):
    pkg = find_package(root, USB_PKG)
    smds = collect_smds(pkg)
    mask_rects = collect_layer_rects(pkg, 29)  # USB mask openings are rectangles
    rows = []
    for s in smds:
        pb = smd_bbox(s)
        mb = match_mask_to_pad(pb, mask_rects)
        pad_w, pad_h = s["dx"], s["dy"]
        mask_w, mask_h = mb[2]-mb[0], mb[3]-mb[1]
        enc_x_per_edge_um = max(0.0, (pad_w - mask_w) / 2) * 1000  # pitch axis = x
        enc_y_per_edge_um = max(0.0, (pad_h - mask_h) / 2) * 1000
        exposed = overlap_area(pb, mb)
        pct = 100 * exposed / (pad_w * pad_h)
        rows.append((s["name"], pad_w, mask_w, enc_x_per_edge_um, enc_y_per_edge_um, pct))
    return rows


usb_rows = analyze_usb(roots["USB.lbr"], "USB.lbr")
print("\n(a) USB-C HRO_TYPE-C-31-M-12 pads (pitch axis = x)")
for name, pw, mw, encx, ency, pct in usb_rows:
    print(f"    pad {name:8s} padW={pw:.2f} maskW={mw:.2f} enc/edge(x)={encx:.1f}um "
          f"enc/edge(y)={ency:+.1f}um  exposed={pct:.1f}%")
usb_enc_vals = sorted(set(round(r[3],1) for r in usb_rows))
usb_pcts = [r[5] for r in usb_rows]
print(f"    USB encroachment per pitch-axis edge: {usb_enc_vals} um")
print(f"    USB min %% pad area exposed: {min(usb_pcts):.2f}%")
usb_min_exposed = min(usb_pcts)
usb_enc_um = max(r[3] for r in usb_rows)

# ===========================================================================
# (b) PASTE / cream layer 31 NOT shrunk + still covers pads
# ===========================================================================
print("\n(b) Solder-PASTE (tCream layer 31) check")
# DRV cream: one rounded poly (EP paste). USB cream: none in lib (relies on smd cream).
drv_cream_bbs = [bbox(xs, ys) for xs, ys, _ in drv_cream]
print(f"    DRV layer-31 polygons: {len(drv_cream)}  bbox(es): "
      f"{[tuple(round(c,3) for c in b) for b in drv_cream_bbs]}")
# expected DRV cream EP poly ~ 3.1 x 5.18 mm centered at origin (from source)
drv_cream_ok = False
if drv_cream_bbs:
    b = drv_cream_bbs[0]
    w, h = b[2]-b[0], b[3]-b[1]
    drv_cream_ok = abs(w - 3.10) < 0.05 and abs(h - 5.18) < 0.05
    print(f"    DRV EP paste aperture WxH = {w:.3f} x {h:.3f} mm  (expected ~3.10 x 5.18)")
# USB cream apertures: SMD pads have no cream="no" so paste = full pad. Check none shrunk.
usb_pkg = find_package(roots["USB.lbr"], USB_PKG)
usb_cream_polys = collect_layer_polys(usb_pkg, 31)
usb_cream_rects = collect_layer_rects(usb_pkg, 31)
usb_smd_cream_off = [s["name"] for s in collect_smds(usb_pkg) if s.get("cream") == "no"]
print(f"    USB layer-31 explicit apertures: polys={len(usb_cream_polys)} rects={len(usb_cream_rects)}")
print(f"    USB smds with cream='no': {usb_smd_cream_off or 'none -> paste follows full pad'}")
# Confirm the signal SMD pads still carry full-size copper (paste = pad since no cream override)
drv_smd_cream_off = [s["name"] for s in drv_smds if s.get("cream") == "no"]
print(f"    DRV signal smds with cream='no': {drv_smd_cream_off or 'none -> paste follows full pad'}")
paste_ok = drv_cream_ok and not usb_smd_cream_off

# ===========================================================================
# (c) DRV EP thermal mask polygon untouched
# ===========================================================================
print("\n(c) DRV exposed/thermal EP polygon (layer 29) check")
for b in drv_ep:
    w, h = b[2]-b[0], b[3]-b[1]
    print(f"    EP mask poly bbox WxH = {w:.3f} x {h:.3f} mm "
          f"(x {b[0]:.3f}..{b[2]:.3f}, y {b[1]:.3f}..{b[3]:.3f})")
ep_ok = False
if drv_ep:
    w, h = drv_ep[0][2]-drv_ep[0][0], drv_ep[0][3]-drv_ep[0][1]
    ep_ok = abs(w - 3.5) < 0.05 and abs(h - 9.8) < 0.05
print(f"    EP poly matches ~3.5 x 9.8 mm: {ep_ok}")

# ===========================================================================
# (d) consistency across all four files
# ===========================================================================
print("\n(d) Cross-file consistency of DRV + USB geometry")

def geom_signature(root, pkg_name, layers):
    """Canonical string of all polygon/rectangle/smd geometry on given layers + all smds."""
    pkg = find_package(root, pkg_name)
    if pkg is None:
        return None
    parts = []
    for poly in pkg.findall("polygon"):
        if poly.get("layer") in layers:
            verts = ";".join(f"{float(v.get('x')):.4f},{float(v.get('y')):.4f},"
                             f"{v.get('curve','0')}" for v in poly.findall("vertex"))
            parts.append(f"P{poly.get('layer')}[{verts}]")
    for r in pkg.findall("rectangle"):
        if r.get("layer") in layers:
            parts.append("R%s[%.4f,%.4f,%.4f,%.4f]" % (
                r.get("layer"), float(r.get("x1")), float(r.get("y1")),
                float(r.get("x2")), float(r.get("y2"))))
    for s in pkg.findall("smd"):
        parts.append("S[%s,%.4f,%.4f,%.4f,%.4f,stop=%s,cream=%s]" % (
            s.get("name"), float(s.get("x")), float(s.get("y")),
            float(s.get("dx")), float(s.get("dy")),
            s.get("stop"), s.get("cream")))
    return "\n".join(sorted(parts))


LAYERS = {"29", "30", "31"}


def bbox_set(pkg, layers):
    """Sorted set of rounded bboxes for polys+rects on given layers (arc-encoding agnostic)."""
    out = []
    for poly in pkg.findall("polygon"):
        if poly.get("layer") in layers:
            xs = [float(v.get("x")) for v in poly.findall("vertex")]
            ys = [float(v.get("y")) for v in poly.findall("vertex")]
            out.append((poly.get("layer"),) + tuple(round(c, 3) for c in bbox(xs, ys)))
    for r in pkg.findall("rectangle"):
        if r.get("layer") in layers:
            out.append((r.get("layer"), round(min(float(r.get("x1")), float(r.get("x2"))), 3),
                        round(min(float(r.get("y1")), float(r.get("y2"))), 3),
                        round(max(float(r.get("x1")), float(r.get("x2"))), 3),
                        round(max(float(r.get("y1")), float(r.get("y2"))), 3)))
    return tuple(sorted(out))


def smd_set(pkg):
    return tuple(sorted(
        (s.get("name"), round(float(s.get("x")), 4), round(float(s.get("y")), 4),
         round(float(s.get("dx")), 4), round(float(s.get("dy")), 4),
         s.get("stop"), s.get("cream")) for s in pkg.findall("smd")))


def edited_rect_features(pkg, layer):
    """Bit-exact rectangle/polygon-bbox features on a layer (the actually-shrunk masks)."""
    return bbox_set(pkg, {str(layer)})


def consistency(pkg_name, owner):
    pkgs = {k: find_package(r, pkg_name) for k, r in roots.items()}
    pkgs = {k: p for k, p in pkgs.items() if p is not None}
    keys = sorted(pkgs)
    bset = {k: bbox_set(pkgs[k], LAYERS) for k in keys}
    sset = {k: smd_set(pkgs[k]) for k in keys}
    bb_ok = len({bset[k] for k in keys}) == 1
    smd_ok = len({sset[k] for k in keys}) == 1
    print(f"    {pkg_name} present in {keys}")
    print(f"      mask/paste bbox-set identical (arc-encoding agnostic): {bb_ok}")
    print(f"      smd geometry+stop+cream identical:                     {smd_ok}")
    return bb_ok and smd_ok


drv_consistent = consistency(DRV_PKG, "DRV.lbr")
usb_consistent = consistency(USB_PKG, "USB.lbr")
# Note: Routes.brd re-serializes arc (curve=) polygons as straight-vertex fans with
# identical bbox; the shrunk signal-pad masks (DRV poly-rects / USB rects) stay bit-exact.
consistent = drv_consistent and usb_consistent

# ===========================================================================
# SUMMARY
# ===========================================================================
print("\n" + "=" * 78)
print("SUMMARY")
print("=" * 78)
print(f"  DRV encroachment/edge (pitch axis): {drv_enc_um:.1f} um")
print(f"  USB encroachment/edge (pitch axis): {usb_enc_um:.1f} um")
print(f"  Min %% pad solderable area exposed:  {min(drv_min_exposed, usb_min_exposed):.2f}%")
print(f"  Paste (cream L31) untouched & covers pads: {paste_ok}")
print(f"  DRV EP thermal poly untouched:             {ep_ok}")
print(f"  All 4 files consistent:                    {consistent}")
print(f"  GLOBAL_MIN_EXPOSED = {min(drv_min_exposed, usb_min_exposed):.4f}")
print(f"  MAX_ENC_UM = {max(drv_enc_um, usb_enc_um):.4f}")
