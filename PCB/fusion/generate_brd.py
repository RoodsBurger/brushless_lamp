#!/usr/bin/env python3
"""
Generate a Fusion / EAGLE 9.x BOARD (.brd) for the BrushlessLamp custom PCB with
EVERY component PLACED and EVERY net present as a routable airwire/signal. Routing
is left to the user.

Derived from the SAME netlist tables as generate_sch.py (PARTS / REPLACED / DEVSETS
/ PKG + the vendor .lbr files) so BrushlessLamp.brd stays board/schematic-consistent
with BrushlessLamp.sch.

Layout policy:
  * SINGLE-SIDED placement (all parts on the TOP copper) -> one SMT reflow.
  * 4-LAYER board: inner copper layers (Route2 / Route15) are defined so you can set
    a Top / GND / PWR / Bottom stack in Layer Setup and pour the planes.
  * The ESP32-S3-WROOM-1 sits at the TOP edge with its PCB antenna pointing off the
    board (the pad-free +Y end; keep-out on layers 41/42/43 reaches the edge).
  * 4x M2.5 mounting holes (2.8 mm) in the corners with keep-outs the placer avoids.
  * Rounded board corners.
  * Bottom-left-fill packing minimises the footprint; connectors cluster on the
    bottom edge. Nudge during routing -- this is a compact STARTING placement.

Output: BrushlessLamp.brd  (next to BrushlessLamp.sch).
Usage:  python3 generate_brd.py [force_board_width_mm]
"""
import os, sys, math, xml.etree.ElementTree as ET
import xml.sax.saxutils as su

from generate_sch import (PKG, DEVSETS, PARTS, REPLACED, REAL_FILES, COMP_DIR,
                          build_package)

def esc(s): return su.escape(str(s), {'"': "&quot;"})

# ----------------------------------------------------------------- geometry
def pkg_bbox(pkg_el):
    xs, ys = [], []
    for s in pkg_el.findall("smd"):
        x, y = float(s.get("x")), float(s.get("y"))
        dx, dy = float(s.get("dx")), float(s.get("dy"))
        if s.get("rot", "R0") in ("R90", "R270"): dx, dy = dy, dx
        xs += [x - dx/2, x + dx/2]; ys += [y - dy/2, y + dy/2]
    for p in pkg_el.findall("pad"):
        x, y = float(p.get("x")), float(p.get("y"))
        d = float(p.get("diameter") or (float(p.get("drill", "1")) * 1.5))
        xs += [x - d/2, x + d/2]; ys += [y - d/2, y + d/2]
    for w in pkg_el.findall("wire"):
        xs += [float(w.get("x1")), float(w.get("x2"))]
        ys += [float(w.get("y1")), float(w.get("y2"))]
    for c in pkg_el.findall("circle"):
        x, y, r = float(c.get("x")), float(c.get("y")), float(c.get("radius"))
        xs += [x - r, x + r]; ys += [y - r, y + r]
    for h in pkg_el.findall("hole"):
        x, y = float(h.get("x")), float(h.get("y")); r = float(h.get("drill"))/2
        xs += [x - r, x + r]; ys += [y - r, y + r]
    for r in pkg_el.findall("rectangle"):
        xs += [float(r.get("x1")), float(r.get("x2"))]
        ys += [float(r.get("y1")), float(r.get("y2"))]
    for pg in pkg_el.findall("polygon"):            # copper pours / courtyards / keep-outs
        for v in pg.findall("vertex"):
            xs.append(float(v.get("x"))); ys.append(float(v.get("y")))
    if not xs: return (-0.5, -0.5, 0.5, 0.5)
    return (min(xs), min(ys), max(xs), max(ys))

GENERIC_PKG = {}      # name -> (xml, padset, bbox)
for n, pads in PKG.items():
    xml = build_package(n, pads)[0]
    GENERIC_PKG[n] = (xml, set(t[1] for t in pads), pkg_bbox(ET.fromstring(xml)))

def parse_vendor():
    libs = {}
    for libname, fn in REAL_FILES.items():
        root = ET.parse(os.path.join(COMP_DIR, fn)).getroot()
        lib = root.find(".//library")
        ds2pkg, ds2conn, ds2attrs = {}, {}, {}
        for ds in lib.findall("devicesets/deviceset"):
            dev = ds.find("devices/device")
            ds2pkg[ds.get("name")] = dev.get("package")
            conn = {}
            cs = dev.find("connects")
            if cs is not None:
                for c in cs.findall("connect"):
                    conn[c.get("pin")] = c.get("pad")
            ds2conn[ds.get("name")] = conn
            attrs = []
            for tech in dev.findall("technologies/technology"):
                for a in tech.findall("attribute"):
                    attrs.append((a.get("name"), a.get("value") or ""))
            ds2attrs[ds.get("name")] = attrs
        pkgs = {}
        for pk in lib.find("packages").findall("package"):
            pads = set()
            for s in pk.findall("smd"): pads.add(s.get("name"))
            for p in pk.findall("pad"): pads.add(p.get("name"))
            # copper-only bbox (no silk/keep-out) -> lets the WROOM antenna overhang
            cx, cy = [], []
            for s in pk.findall("smd"):
                dx, dy = float(s.get("dx")), float(s.get("dy"))
                if s.get("rot", "R0") in ("R90", "R270"): dx, dy = dy, dx
                cx += [float(s.get("x"))-dx/2, float(s.get("x"))+dx/2]
                cy += [float(s.get("y"))-dy/2, float(s.get("y"))+dy/2]
            for p in pk.findall("pad"):
                d = float(p.get("diameter") or float(p.get("drill", "1"))*1.5)
                cx += [float(p.get("x"))-d/2, float(p.get("x"))+d/2]
                cy += [float(p.get("y"))-d/2, float(p.get("y"))+d/2]
            pkgs[pk.get("name")] = dict(xml=ET.tostring(pk, encoding="unicode"),
                pads=pads, bbox=pkg_bbox(pk),
                pad_bbox=(min(cx), min(cy), max(cx), max(cy)) if cx else pkg_bbox(pk))
        packs_xml = "".join(p["xml"] for p in pkgs.values())
        libs[libname] = dict(
            lib_xml=f'<library name="{esc(libname)}"><packages>{packs_xml}</packages></library>',
            ds2pkg=ds2pkg, ds2conn=ds2conn, ds2attrs=ds2attrs, pkgs=pkgs)
    return libs

VEND = parse_vendor()

def build_parts():
    out = []
    for name, (ds, val, pm) in PARTS.items():
        sym, pkg, conn, pref = DEVSETS[ds]
        out.append(dict(name=name, lib="BLLIB", pkg=pkg, value=val, pinnet=pm,
                        conn=conn, bbox=GENERIC_PKG[pkg][2], attrs=[]))
    for name, (libname, ds, pm) in REPLACED.items():
        v = VEND[libname]; pkg = v["ds2pkg"][ds]
        out.append(dict(name=name, lib=libname, pkg=pkg, value=ds, pinnet=pm,
                        conn=v["ds2conn"][ds], bbox=v["pkgs"][pkg]["bbox"],
                        attrs=v["ds2attrs"][ds]))
    return out

ITEMS = build_parts()

def validate():
    errs = []
    for p in ITEMS:
        padset = (GENERIC_PKG[p["pkg"]][1] if p["lib"] == "BLLIB"
                  else VEND[p["lib"]]["pkgs"][p["pkg"]]["pads"])
        for pin in p["pinnet"]:
            if pin not in p["conn"]:
                errs.append(f'{p["name"]}: pin {pin} has no connect'); continue
            for pad in p["conn"][pin].split():
                if pad not in padset:
                    errs.append(f'{p["name"]}: pad {pad} not in pkg {p["pkg"]}')
    return errs

# ----------------------------------------------------------------- placement
GAP = 1.05          # mm courtyard between interior parts (routing channels)
CGAP = 1.6          # mm between edge-mounted connectors
MARGIN = 1.2        # mm part-to-board-edge clearance
HOLE_DRILL = 2.8    # M2.5 clearance hole
HOLE_INSET = 4.2    # mounting-hole centre inset from each board edge
HOLE_KEEP_R = 3.0   # keep-out radius around each hole (no parts)
CORNER_R = 3.5      # rounded board-corner radius
PLANE_INSET = 0.5   # inner GND-plane pull-back from the board edge
# Off-board connectors pinned flush to an edge; rotation faces each part's wire /
# cable opening OFF the board (USB-C mouth is -Y in R0 -> R90 points it off +X).
EDGE = {"bottom": ["J_PWR", "J_MOTOR"],            # power + motor (high current)
        "left":   ["J_SENSOR", "J_LED"],           # encoder + LED strip
        "right":  ["J_USB", "J_KNOB", "J_BTN"]}    # USB-C + knob + button
EDGE_ROT = {"bottom": "R0", "left": "R270", "right": "R90"}
EDGE_NAMES = [n for e in EDGE.values() for n in e]
# Each IC / connector keeps its own support parts clustered around it:
GROUPS = [
    ("U_MCU",   ["C_MCU1", "C_MCU2", "C_MCU3", "C_MCU4", "C_MCU_BULK", "D_MCU",
                 "R_EN", "C_EN", "SW_RST", "R_BOOT", "SW_BOOT", "C_BOOT",
                 "R_STATUS", "LED_STATUS"]),
    ("J_USB",   ["D_USB", "R_CC1", "R_CC2", "R_DP", "R_DM", "C_VBUS", "D_OR_USB"]),
    ("J_PWR",   ["F1", "Q1", "TVS1", "C_IN_BULK", "R_Q1", "D_Q1"]),
    ("U_BUCK",  ["L1", "C_IN1", "C_IN2", "C_OUT1", "C_OUT2", "R_ENB", "C_BOOT_B",
                 "R_FBT", "R_FBB", "C_FF"]),
    ("U_DRV",   ["C_VM_BULK", "C_CP", "C_VCP", "C_V3P3", "C_VM1", "C_VM2",
                 "R_NSLEEP", "R_NRESET", "R_NFAULT", "R_EN_DRV"]),
    ("J_SENSOR", ["C_SENS", "R_AZ_A", "R_AZ_B", "R_AZ_C"]),
    ("J_LED",   ["Q_WW", "Q_CW", "R_GW", "R_GC", "R_PDW", "R_PDC", "D_CLW", "D_CLC"]),
]
LEFTOVER = ["J_EXP", "J_PROG"]
# Wider copper for the power rails (the autorouter routes each net at its class width).
NET_CLASS = {"P24": "1", "P24_RAW": "1", "P24_F": "1", "P3V3": "2", "VBUS": "2"}
# Extra keep-out (mm) around the two dense breakouts so their pins have fan-out room
# (the DRV8313 PWM/VM pins and the USB-C power-pin row are the routing choke points).
BREAKOUT = {"U_DRV": 1.8, "J_USB": 1.8}

def rbox(bb, rot):
    a, b, c, d = bb
    if rot == "R0":   return (a, b, c, d)
    if rot == "R90":  return (-d, a, -b, c)
    if rot == "R180": return (-c, -d, -a, -b)
    if rot == "R270": return (b, -c, d, -a)

def fr(p, rot="R0"):
    return rbox(p["bbox"], rot)                    # rmnx, rmny, rmxx, rmxy

def overlap(r, rects):
    x0, y0, x1, y1 = r
    return any(x0 < a1-1e-9 and a0 < x1-1e-9 and y0 < b1-1e-9 and b0 < y1-1e-9
               for a0, b0, a1, b1 in rects)

def infl(r):                                        # pad an obstacle by half a gap so
    g = GAP / 2                                     # interior parts keep clearance from it
    return (r[0]-g, r[1]-g, r[2]+g, r[3]+g)

def hole_rects(W, H):
    I, K = HOLE_INSET, HOLE_KEEP_R
    return [(I-K, I-K, I+K, I+K), (W-I-K, I-K, W-I+K, I+K),
            (I-K, H-I-K, I+K, H-I+K), (W-I-K, H-I-K, W-I+K, H-I+K)]

def fits(r, W, H, rects):
    x0, y0, x1, y1 = r
    return not (x0 < MARGIN-1e-9 or y0 < MARGIN-1e-9 or x1 > W-MARGIN+1e-9
                or y1 > H-MARGIN+1e-9 or overlap(r, rects))

def seed(cands, x0, y0, x1, y1):
    cands |= {(x1, y0), (x0, y1), (x1, y1)}

def place_near(p, target, W, H, rects, cands, rot="R0"):
    """Drop the part on the free candidate point nearest `target` (clusters it
    around its group anchor). Returns (x,y,rot) or None."""
    a, b, c, d = fr(p, rot); w, h = (c - a) + GAP, (d - b) + GAP
    best = None
    for cx, cy in cands:
        if fits((cx, cy, cx + w, cy + h), W, H, rects):
            dist = (cx + w/2 - target[0])**2 + (cy + h/2 - target[1])**2
            if best is None or dist < best[0]:
                best = (dist, cx, cy)
    if best is None:
        return None
    _, cx, cy = best
    rects.append((cx, cy, cx + w, cy + h)); seed(cands, cx, cy, cx + w, cy + h)
    return (cx - a, cy - b, rot)

def place_edges(W, H, by, rects, cands):
    pos = {}
    e0 = HOLE_INSET + HOLE_KEEP_R + CGAP           # start past the corner keep-outs
    for side, names in EDGE.items():
        rot = EDGE_ROT[side]; t = e0
        for n in names:
            a, b, c, d = fr(by[n], rot); w, h = c - a, d - b
            if side == "bottom":
                x0, y0 = t, MARGIN
            elif side == "left":
                x0, y0 = MARGIN, t
            else:                                   # right
                x0, y0 = W - MARGIN - w, t
            r = (x0, y0, x0 + w, y0 + h)
            lim = (W - e0) if side == "bottom" else (H - e0)
            if (r[2] if side == "bottom" else r[3]) > lim or not fits(r, W, H, rects):
                return None
            ex = BREAKOUT.get(n, 0.0); g = GAP / 2     # extra clearance inboard of the edge
            ri = (r[0]-g-(ex if side == "right" else 0), r[1]-g-(ex if side == "top" else 0),
                  r[2]+g+(ex if side == "left" else 0), r[3]+g+(ex if side == "bottom" else 0))
            pos[n] = (x0 - a, y0 - b, rot); rects.append(ri); seed(cands, *ri)
            t += (w if side == "bottom" else h) + CGAP
    return pos

def cen(rects, r):                                  # centre of a placed rect
    return ((r[0] + r[2]) / 2, (r[1] + r[3]) / 2)

# Interior parts = everything that is NOT the WROOM and NOT an edge connector.
INTERIOR = [p["name"] for p in ITEMS
            if p["name"] != "U_MCU" and p["name"] not in EDGE_NAMES]

# ----- net connectivity (for greedy min-HPWL placement) ---------------------
# part -> set(nets) and net -> set(parts), excluding GND/P3V3 power planes that
# every part shares (they route to inner planes via, so they don't pull placement).
PLANE_NETS = {"GND", "P3V3"}
def _connectivity():
    p2n, n2p = {}, {}
    for p in ITEMS:
        nets = set(p["pinnet"].values())
        p2n[p["name"]] = nets
        for net in nets:
            n2p.setdefault(net, set()).add(p["name"])
    return p2n, n2p
PART_NETS, NET_PARTS = _connectivity()
# Signal nets that actually pull two parts together (drop planes + single-part nets).
def _pull_nets(name):
    return [n for n in PART_NETS[name]
            if n not in PLANE_NETS and len(NET_PARTS[n]) >= 2]

def ecenter(name, pos):
    """Board-coord centre of a placed part's bbox (matches measure.py ecen)."""
    x, y, rot = pos[name]; a, b, c, d = fr(by_global[name], rot)
    return (x + (a + c) / 2, y + (b + d) / 2)

def incr_hpwl(name, cx, cy, pos):
    """Added HPWL if part `name` is centred at (cx,cy): sum over its pulling nets
    of the half-perimeter growth of that net's placed-centre bounding box."""
    add = 0.0
    for net in _pull_nets(name):
        xs, ys = [], []
        for q in NET_PARTS[net]:
            if q != name and q in pos:
                qx, qy = ecenter(q, pos); xs.append(qx); ys.append(qy)
        if not xs:
            continue
        bx0, bx1 = min(xs), max(xs); by0, by1 = min(ys), max(ys)
        base = (bx1 - bx0) + (by1 - by0)
        nx0, nx1 = min(bx0, cx), max(bx1, cx); ny0, ny1 = min(by0, cy), max(by1, cy)
        add += (nx1 - nx0) + (ny1 - ny0) - base
    return add

by_global = {p["name"]: p for p in ITEMS}

# Anchor -> its group members (from GROUPS). Edge connectors are pre-placed by
# place_edges; interior IC anchors (U_BUCK, U_DRV) get placed greedily first.
GROUP_OF = {a: list(m) for a, m in GROUPS}
INTERIOR_ANCHORS = [a for a, _ in GROUPS if a in INTERIOR]   # U_BUCK, U_DRV
# Interior parts NOT in any group (leftover expansion / prog headers etc.).
_grouped = {m for _, mem in GROUPS for m in mem} | set(GROUP_OF)
UNGROUPED = [n for n in INTERIOR if n not in _grouped]
# member part -> its group anchor (so every member is pulled toward its anchor, which
# is essential for decoupling caps whose only nets are GND/P3V3 planes -> no HPWL pull).
MEMBER_ANCHOR = {m: a for a, mem in GROUPS for m in mem}
# Weight on anchor-distance in the placement cost. Dominant for plane-only parts (caps)
# so they hug their anchor; for signal parts the HPWL term leads and this only breaks ties.
ANCHOR_W = 0.6

def _net_centroid(name, pos):
    """Mean centre of the part's already-placed pulling-net neighbours (or None)."""
    xs, ys = [], []
    for net in _pull_nets(name):
        for q in NET_PARTS[net]:
            if q != name and q in pos:
                qx, qy = ecenter(q, pos); xs.append(qx); ys.append(qy)
    if not xs:
        return None
    return (sum(xs) / len(xs), sum(ys) / len(ys))

def _candidates(W, H, rects, cands, focus=None, cap=140):
    """Free-corner candidate points: the incrementally-seeded corners plus the cross
    product of obstacle right/top edges (L-notches). To stay cheap the cross product is
    built only from the obstacle edges nearest `focus`; the result is capped to `cap`."""
    pts = set(cands)
    if focus is not None:
        fx, fy = focus
        # nearest ~16 right-edges and top-edges to the focus -> a small L-notch grid
        xs = sorted({MARGIN} | {r[2] for r in rects}, key=lambda v: abs(v - fx))[:16]
        ys = sorted({MARGIN} | {r[3] for r in rects}, key=lambda v: abs(v - fy))[:16]
        pts |= {(cx, cy) for cx in xs for cy in ys}
        if len(pts) > cap:
            pts = sorted(pts, key=lambda p: (p[0]-fx)**2 + (p[1]-fy)**2)[:cap]
    else:
        xs = {MARGIN} | {r[2] for r in rects}
        ys = {MARGIN} | {r[3] for r in rects}
        pts |= {(cx, cy) for cx in xs for cy in ys}
    return pts

def greedy_place(name, W, H, rects, cands, pos, allow_rot=True, anchor_xy=None,
                 part_rect=None):
    """Greedy min-HPWL insertion: among fitting free positions (R0/R90), choose the one
    minimising incremental HPWL to already-placed pulling-net neighbours PLUS a weighted
    pull toward the part's group anchor (keeps plane-only decoupling caps hugging their
    IC; for signal parts the HPWL term dominates). Mutates rects/cands/pos."""
    rots = ("R0", "R90") if allow_rot else ("R0",)
    g = GAP + 2 * BREAKOUT.get(name, 0.0)
    # resolve the anchor this part should hug (explicit arg wins, else its group anchor)
    axy = anchor_xy
    if axy is None and name in MEMBER_ANCHOR and MEMBER_ANCHOR[name] in pos:
        axy = ecenter(MEMBER_ANCHOR[name], pos)
    focus = _net_centroid(name, pos) or axy
    best = None                                    # (cost, tie, x0, y0, rot, w, h, a, b)
    for cap in (120, 10**9):                         # capped near-focus set, then all candidates
        corner = _candidates(W, H, rects, cands, focus, cap)
        for rot in rots:
            a, b, c, d = fr(by_global[name], rot); w, h = (c - a) + g, (d - b) + g
            for cx, cy in corner:
                r = (cx, cy, cx + w, cy + h)
                if not fits(r, W, H, rects):
                    continue
                ctr = (cx + w / 2, cy + h / 2)
                cost = incr_hpwl(name, ctr[0], ctr[1], pos)
                if axy is not None:
                    dist = math.hypot(ctr[0] - axy[0], ctr[1] - axy[1])
                    cost += ANCHOR_W * dist        # pull toward the group anchor
                    tie = dist
                else:
                    tie = cx * cx + cy * cy
                if best is None or (cost, tie) < (best[0], best[1]):
                    best = (cost, tie, cx, cy, rot, w, h, a, b)
        if best is not None:
            break
    if best is None:
        return False
    _, _, x0, y0, rot, w, h, a, b = best
    rect = (x0, y0, x0 + w, y0 + h)
    rects.append(rect); seed(cands, x0, y0, x0 + w, y0 + h)
    cands.add((x0, y0))
    pos[name] = (x0 + g / 2 - a, y0 + g / 2 - b, rot)
    if part_rect is not None:
        part_rect[name] = rect
    return True

def _place_cost(name, ctr, pos):
    """Combined placement cost at centre `ctr`: incremental HPWL + anchor pull (mirrors
    greedy_place so refine accepts a move only when this total cost drops)."""
    cost = incr_hpwl(name, ctr[0], ctr[1], pos)
    a = MEMBER_ANCHOR.get(name)
    if a is not None and a in pos:
        ax, ay = ecenter(a, pos)
        cost += ANCHOR_W * math.hypot(ctr[0] - ax, ctr[1] - ay)
    return cost

def refine(W, H, by, rects, cands, pos, part_rect, names, passes=2):
    """Local cleanup: with all parts placed, lift each movable part out and re-insert it
    greedily (its net centroid + anchor are now known). Keeps the move only if the
    combined cost drops. Repeats while anything improves."""
    for _ in range(passes):
        improved = False
        for nm in names:
            if nm not in part_rect:
                continue
            old = part_rect[nm]; saved = pos[nm]
            base = _place_cost(nm, ecenter(nm, pos), pos)
            try:
                rects.remove(old)
            except ValueError:
                continue
            del part_rect[nm]; del pos[nm]
            if greedy_place(nm, W, H, rects, cands, pos, part_rect=part_rect):
                new = _place_cost(nm, ecenter(nm, pos), pos)
                if new + 1e-6 < base:
                    improved = True
                elif new > base + 1e-6:                    # worse: restore original spot
                    rects.remove(part_rect[nm]); del part_rect[nm]; del pos[nm]
                    rects.append(old); pos[nm] = saved; part_rect[nm] = old
            else:                                          # couldn't re-place: restore
                rects.append(old); pos[nm] = saved; part_rect[nm] = old
        if not improved:
            break

def _place_one(W, H, by, mcu, do_refine):
    """Place all parts for one fixed U_MCU anchor `mcu`=(ax,ay,arot). Returns pos or
    None. do_refine runs the local re-insertion cleanup (skip it during board search)."""
    ax, ay, arot = mcu
    a, b, c, d = fr(by["U_MCU"], arot)
    mx0, my0 = ax + a, ay + b
    if (mx0 < MARGIN - 1e-9 or my0 < MARGIN - 1e-9
            or mx0 + (c - a) > W - MARGIN + 1e-9 or my0 + (d - b) > H - MARGIN + 1e-9):
        return None
    rects = list(hole_rects(W, H))
    mr = (mx0, my0, mx0 + (c - a), my0 + (d - b))
    if overlap(mr, rects):
        return None
    cands = {(MARGIN, MARGIN)}
    for r in rects:
        seed(cands, *r)
    rects.append(infl(mr)); seed(cands, *infl(mr))
    epos = place_edges(W, H, by, rects, cands)
    if epos is None:
        return None
    pos = {"U_MCU": (ax, ay, arot)}; pos.update(epos)
    part_rect = {}
    # 1) interior IC anchors first (pulled toward motor/power by their nets)
    for nm in INTERIOR_ANCHORS:
        if not greedy_place(nm, W, H, rects, cands, pos, part_rect=part_rect):
            return None
    # 2) large ungrouped parts next (big headers need room before gaps fill)
    big = sorted(UNGROUPED, key=lambda n: -(fr(by[n])[2] - fr(by[n])[0])
                                          * (fr(by[n])[3] - fr(by[n])[1]))
    for nm in big:
        if not greedy_place(nm, W, H, rects, cands, pos, part_rect=part_rect):
            return None
    # 3) each group's members clustered around its (now-placed) anchor
    for anchor, mem in GROUPS:
        if anchor not in pos:
            continue
        axy = ecenter(anchor, pos)
        mm = sorted(mem, key=lambda n: -(fr(by[n])[2] - fr(by[n])[0])
                                      * (fr(by[n])[3] - fr(by[n])[1]))
        for nm in mm:
            if not greedy_place(nm, W, H, rects, cands, pos, anchor_xy=axy,
                                part_rect=part_rect):
                return None
    # 4) local refinement: re-insert each movable part now that all are placed
    if do_refine:
        movable = INTERIOR_ANCHORS + big + [m for _, mem in GROUPS for m in mem]
        refine(W, H, by, rects, cands, pos, part_rect, movable)
    return pos

def greedy_layout(W, H, by, do_refine=False, anchor_limit=None, return_anchor=False):
    """Try each U_MCU anchor, keep the lowest-HPWL feasible placement. During the W/H
    search use do_refine=False + a small anchor_limit (fast); the winning board re-runs
    the single best anchor with refinement for the final emit."""
    best_pos = None; best_hpwl = None; best_anchor = None
    anchors = _mcu_anchors(W, H, by)
    if anchor_limit is not None:
        anchors = anchors[:anchor_limit]
    for mcu in anchors:
        pos = _place_one(W, H, by, mcu, do_refine)
        if pos is None:
            continue
        hp = _placed_hpwl(pos)
        if best_hpwl is None or hp < best_hpwl:
            best_hpwl = hp; best_pos = pos; best_anchor = mcu
    if return_anchor:
        return best_pos, best_anchor
    return best_pos

def _placed_hpwl(pos, exclude_planes=False):
    """Total HPWL over nets for a finished placement. exclude_planes=False matches
    measure.py (counts every multi-part net, GND/P3V3 included) -> board selection;
    exclude_planes=True is the signal-only routability proxy."""
    tot = 0.0
    for net, parts in NET_PARTS.items():
        if exclude_planes and net in PLANE_NETS:
            continue
        pts = [ecenter(p, pos) for p in parts if p in pos]
        if len(pts) >= 2:
            xs = [q[0] for q in pts]; ys = [q[1] for q in pts]
            tot += (max(xs) - min(xs)) + (max(ys) - min(ys))
    return tot

def _gravity(x0, y0, w, h, rects, W, H, scan):
    """Slide a placed cell toward the origin: 'rows' drops in -y then -x; 'cols'
    pushes -x then -y. Snaps it flush against the nearest obstacle/edge."""
    if scan == "rows":
        order = ((0, -1), (-1, 0))
    else:
        order = ((-1, 0), (0, -1))
    for _ in range(3):
        for dx, dy in order:
            if dy < 0:
                ny = MARGIN
                for a0, b0, a1, b1 in rects:
                    if x0 < a1 - 1e-9 and a0 < x0 + w - 1e-9 and b1 <= y0 + 1e-9:
                        ny = max(ny, b1)
                y0 = ny
            else:
                nx = MARGIN
                for a0, b0, a1, b1 in rects:
                    if y0 < b1 - 1e-9 and b0 < y0 + h - 1e-9 and a1 <= x0 + 1e-9:
                        nx = max(nx, a1)
                x0 = nx
    return x0, y0

def blf_pack(W, H, by, rects, cands, order, scan, allow_rot, thorough=False):
    """Bottom-left-fill: drop each part at the best free candidate, then slide it
    toward the origin (gravity) to snap flush. thorough=False uses the seeded
    corner candidates only (fast, for the W/H search); thorough=True also probes
    the cross product of obstacle right-edges and top-edges so a part can wedge
    into an L-notch (denser, for the final emit). scan='rows' sorts by (y,x);
    'cols' by (x,y). Tries R0 and (if allow_rot) R90. Returns pos dict or None."""
    rects = list(rects); cands = set(cands); pos = {}
    keyf = (lambda t: (t[1], t[0])) if scan == "rows" else (lambda t: (t[0], t[1]))
    for nm in order:
        rots = ("R0", "R90") if allow_rot else ("R0",)
        g = GAP + 2 * BREAKOUT.get(nm, 0.0)            # extra fan-out halo for choke parts
        if thorough:
            xs = {MARGIN} | {r[2] for r in rects}
            ys = {MARGIN} | {r[3] for r in rects}
            corner = cands | {(cx, cy) for cx in xs for cy in ys}
        else:
            corner = cands
        best = None                                # (sortkey, x0, y0, rot, w, h, a, b)
        for rot in rots:
            a, b, c, d = fr(by[nm], rot); w, h = (c - a) + g, (d - b) + g
            for cx, cy in corner:
                if not fits((cx, cy, cx + w, cy + h), W, H, rects):
                    continue
                gx, gy = _gravity(cx, cy, w, h, rects, W, H, scan)
                if not fits((gx, gy, gx + w, gy + h), W, H, rects):
                    gx, gy = cx, cy
                k = keyf((gx, gy))
                if best is None or k < best[0]:
                    best = (k, gx, gy, rot, w, h, a, b)
        if best is None:
            return None
        _, x0, y0, rot, w, h, a, b = best
        rects.append((x0, y0, x0 + w, y0 + h)); seed(cands, x0, y0, x0 + w, y0 + h)
        cands.add((x0, y0))
        pos[nm] = (x0 + g/2 - a, y0 + g/2 - b, rot)   # centre bbox -> equal clearance all sides
    return pos

# Deterministic interior orderings explored per (W,H): sort keys + a few rotations.
def _orderings(by, full=True):
    base = list(INTERIOR)
    dim = lambda n, i: (fr(by[n])[i+2] - fr(by[n])[i])
    area_k = lambda n: -(dim(n, 0) * dim(n, 1))
    if not full:                                            # cheap feasibility set
        return [sorted(base, key=area_k), sorted(base, key=lambda n: -dim(n, 1))]
    keys = [
        area_k,                                             # area desc
        lambda n: -dim(n, 1),                               # height desc
        lambda n: -dim(n, 0),                               # width desc
        lambda n: -(dim(n, 0) + dim(n, 1)),                 # half-perimeter desc
        lambda n: -max(dim(n, 0), dim(n, 1)),               # max-side desc
        lambda n: -min(dim(n, 0), dim(n, 1)),               # min-side desc
    ]
    outs = [sorted(base, key=k) for k in keys]
    a_desc = outs[0]
    for shift in (len(a_desc)//4, len(a_desc)//2, 3*len(a_desc)//4):
        outs.append(a_desc[shift:] + a_desc[:shift])        # index rotations of area-desc
    return outs

def _mcu_anchors(W, H, by):
    """Candidate (x0, y0, rot) for U_MCU. The antenna end (+Y in R0) must end up on
    a board edge; each placement here puts the module flush against one edge so the
    antenna keep-out reaches the boundary. Top edge keeps the plane-notch math sane,
    so it's tried first; side placements (rotated) trade a shorter board for width."""
    out = []
    for rot in ("R0", "R90", "R270"):
        a, b, c, d = fr(by["U_MCU"], rot); mw, mh = c - a, d - b
        if mw > W - 2 * MARGIN + 1e-9 or mh > H - 2 * MARGIN + 1e-9:
            continue
        if rot == "R0":                            # antenna up -> top edge
            top = H - MARGIN - mh
            lo_x, hi_x = MARGIN, W - MARGIN - mw
            xs = {lo_x, hi_x, (lo_x + hi_x) / 2}
            for f in (0.25, 0.375, 0.5, 0.625, 0.75):  # finer sampling of hub x-position
                xs.add(lo_x + f * (hi_x - lo_x))
            for x0 in sorted(xs):
                out.append((x0 - a, top - b, rot))
        elif rot == "R90":                         # antenna -> left edge
            left = MARGIN
            for y0 in ((H - mh) / 2, MARGIN, H - MARGIN - mh):
                out.append((left - a, y0 - b, rot))
        else:                                      # R270 antenna -> right edge
            right = W - MARGIN - mw
            for y0 in ((H - mh) / 2, MARGIN, H - MARGIN - mh):
                out.append((right - a, y0 - b, rot))
    return out

def _used_extent(pos, by):
    """Bounding (max-x, max-y) of all placed part bboxes -> density proxy."""
    mxx = mxy = 0.0
    for nm, (x, y, rot) in pos.items():
        a, b, c, d = fr(by[nm], rot)
        mxx = max(mxx, x + c); mxy = max(mxy, y + d)
    return mxx, mxy

def layout(W, H, by, full=False):
    """full=False: return the FIRST feasible packing (fast; for the W/H search).
    full=True: try every restart and return the one whose parts fit in the
    smallest bounding box (densest), for the final emitted board."""
    best_pos = None; best_cost = None
    for ax, ay, arot in _mcu_anchors(W, H, by):
        a, b, c, d = fr(by["U_MCU"], arot)
        mx0, my0 = ax + a, ay + b
        if mx0 < MARGIN - 1e-9 or my0 < MARGIN - 1e-9 \
           or mx0 + (c - a) > W - MARGIN + 1e-9 or my0 + (d - b) > H - MARGIN + 1e-9:
            continue
        rects0 = list(hole_rects(W, H))
        mr = (mx0, my0, mx0 + (c - a), my0 + (d - b))
        if overlap(mr, rects0):
            continue
        cands0 = {(MARGIN, MARGIN)}
        for r in rects0:
            seed(cands0, *r)
        rects0.append(infl(mr)); seed(cands0, *infl(mr))
        epos = place_edges(W, H, by, rects0, cands0)
        if epos is None:
            continue
        for order in _orderings(by, full):
            for scan in ("rows", "cols"):
                ipos = blf_pack(W, H, by, rects0, cands0, order, scan, True, full)
                if ipos is None:
                    continue
                pos = {"U_MCU": (ax, ay, arot)}
                pos.update(epos); pos.update(ipos)
                if not full:
                    return pos                      # first feasible packing wins
                mxx, mxy = _used_extent(pos, by)
                cost = mxx * mxy
                if best_cost is None or cost < best_cost:
                    best_cost = cost; best_pos = pos
    return best_pos

AREA_CAP_CM2 = 30.0                                # routability budget (keep < this)

def _area_floor(by):
    """Analytic board-size floor (no packing): total footprint area inflated for the
    GAP halos + holes, made square-ish, clamped to be at least the widest part."""
    tot = 0.0
    for p in ITEMS:
        a, b, c, d = fr(p)
        tot += ((c - a) + GAP) * ((d - b) + GAP)
    tot *= 1.35                                    # routing-channel + corner-hole overhead
    side = math.sqrt(tot)
    widest = max(fr(p)[2] - fr(p)[0] for p in ITEMS) + 2 * MARGIN + 1
    W = int(math.ceil(max(side, widest)))
    H = int(math.ceil(tot / W))
    return W, H

def place(force_width=None):
    by = {p["name"]: p for p in ITEMS}
    AW, AH = _area_floor(by)                        # analytic size floor (cheap)
    FW, FH = AW, AH
    # Clustering needs slack vs. the analytic floor. Probe a grid of board sizes around
    # it; keep the lowest-HPWL greedy placement within the area cap.
    if force_width:
        Wset = [int(force_width)]
    else:
        Wset = list(range(FW - 3, FW + 5))         # FW-3 .. FW+4
    # Cheap search: no refinement, R0 MCU anchors only (the 7 top-edge x-positions; the
    # rotated anchors widen the board and rarely win). Rank boards by the unrefined HPWL
    # proxy + best anchor, then refine only the few most promising boards.
    R0_ANCHORS = 7
    cand_boards = []                               # (hpwl, area, W, H, anchor)
    for W in Wset:
        for dH in range(-2, 6):
            H = FH + dH
            if H < 30 or W * H / 100.0 > AREA_CAP_CM2:
                continue
            p, anc = greedy_layout(W, H, by, do_refine=False, return_anchor=True,
                                   anchor_limit=R0_ANCHORS)
            if p is None:
                continue
            cand_boards.append((_placed_hpwl(p), W * H, W, H, anc))
    best = None                                    # (hpwl, area, W, H, pos)
    if cand_boards:
        cand_boards.sort(key=lambda t: (round(t[0], 1), t[1]))
        for hp0, area0, W, H, anc in cand_boards[:6]:   # refine the 6 most promising boards
            p = _place_one(W, H, by, anc, do_refine=True)
            if p is None:
                continue
            hp = _placed_hpwl(p)
            if best is None or (round(hp, 1), W * H) < (round(best[0], 1), best[1]):
                best = (hp, W * H, W, H, p)
    if best is None:                               # greedy failed everywhere: area packer
        pos = layout(FW, FH, by, full=True)
        BW, BH = FW, FH
    else:
        _, _, BW, BH, pos = best
    holes = [(HOLE_INSET, HOLE_INSET), (BW - HOLE_INSET, HOLE_INSET),
             (HOLE_INSET, BH - HOLE_INSET), (BW - HOLE_INSET, BH - HOLE_INSET)]
    return pos, BW, BH, holes

# ----------------------------------------------------------------- signals
def build_signals():
    sig = {}
    for p in ITEMS:
        for pin, net in p["pinnet"].items():
            for pad in p["conn"][pin].split():
                sig.setdefault(net, []).append((p["name"], pad))
    return sig

# ----------------------------------------------------------------- emit
LAYERS = [
    (1,"Top"),(2,"Route2"),(15,"Route15"),(16,"Bottom"),(17,"Pads"),(18,"Vias"),
    (19,"Unrouted"),(20,"Dimension"),(21,"tPlace"),(22,"bPlace"),(23,"tOrigins"),
    (24,"bOrigins"),(25,"tNames"),(26,"bNames"),(27,"tValues"),(28,"bValues"),
    (29,"tStop"),(30,"bStop"),(31,"tCream"),(32,"bCream"),(33,"tFinish"),
    (34,"bFinish"),(35,"tGlue"),(36,"bGlue"),(37,"tTest"),(38,"bTest"),
    (39,"tKeepout"),(40,"bKeepout"),(41,"tRestrict"),(42,"bRestrict"),(43,"vRestrict"),
    (44,"Drills"),(45,"Holes"),(46,"Milling"),(47,"Measures"),(48,"Document"),
    (49,"Reference"),(51,"tDocu"),(52,"bDocu"),(90,"Modules"),(91,"Nets"),
    (92,"Busses"),(93,"Pins"),(94,"Symbols"),(95,"Names"),(96,"Values"),(97,"Info"),
    (98,"Guide"),
]

def rounded_outline(W, H, r):
    """CCW rounded rectangle on layer 20 (Dimension). Corners are quarter-arc wires."""
    seg = lambda x1,y1,x2,y2,c="": (f'<wire x1="{x1:.3f}" y1="{y1:.3f}" x2="{x2:.3f}" '
        f'y2="{y2:.3f}" width="0.2" layer="20"{c}/>')
    cv = ' curve="90"'
    return "".join([
        seg(r, 0, W-r, 0),            seg(W-r, 0, W, r, cv),
        seg(W, r, W, H-r),            seg(W, H-r, W-r, H, cv),
        seg(W-r, H, r, H),            seg(r, H, 0, H-r, cv),
        seg(0, H-r, 0, r),            seg(0, r, r, 0, cv),
    ])

def part_smds(p):
    """name -> (x, y, dx, dy) for every SMD pad in the part's footprint (local frame)."""
    xml = GENERIC_PKG[p["pkg"]][0] if p["lib"] == "BLLIB" else VEND[p["lib"]]["pkgs"][p["pkg"]]["xml"]
    return {s.get("name"): (float(s.get("x")), float(s.get("y")),
                            float(s.get("dx")), float(s.get("dy")))
            for s in ET.fromstring(xml).findall("smd")}

def gnd_thermal_vias(placed, by):
    """Pre-place GND vias the autorouter will not place itself: the inaccessible exposed
    thermal pads (ESP32 EP array + DRV8313 EP) and the USB-C GND power pads. Each ties its
    pad to the layer-2 GND plane -- this is what otherwise leaves the board short of 100%.
    Returns (x, y, diameter, drill) per via."""
    pts = []
    for nm in ("U_MCU", "U_DRV", "J_USB"):
        if nm not in placed or nm not in by: continue
        ex, ey, rot = placed[nm]; p = by[nm]
        ang = math.radians(int(rot[1:]) if rot.startswith("R") else 0)
        ca, sa = math.cos(ang), math.sin(ang)
        smds = part_smds(p)
        pad2net = {pad: net for pin, net in p["pinnet"].items()
                   for pad in p["conn"].get(pin, "").split()}
        for pad, (lx, ly, dx, dy) in smds.items():
            if pad2net.get(pad) != "GND": continue
            if nm == "J_USB":                        # USB-C GND power pads -> small via to plane
                if min(dx, dy) < 0.45: continue
                dia, drill = 0.45, 0.25
            else:
                nb = sum(1 for q, (qx, qy, _, _) in smds.items()
                         if q != pad and pad2net.get(q) == "GND" and math.hypot(lx-qx, ly-qy) <= 2.0)
                if min(dx, dy) >= 0.55:
                    if nb < 2: continue              # isolated perimeter GND pad: router handles it
                else:                                # narrow pad: only the clear centre EP, never a
                    ok = all(math.hypot(max(abs(lx-qx)-qdx/2, 0), max(abs(ly-qy)-qdy/2, 0)) >= 0.5
                             for q, (qx, qy, qdx, qdy) in smds.items() if q != pad)
                    if not ok: continue              # signal/PGND finger (a via there would short)
                dia, drill = 0.6, 0.3
            pts.append((ex + lx*ca - ly*sa, ey + lx*sa + ly*ca, dia, drill))
    return pts

def power_escape_vias(placed, by):
    """Via-in-pad escapes for power pins the autorouter can't break out of dense parts:
    the two USB-C VBUS pins and the DRV8313 VM pin drop to the open bottom layer so the
    router can reach the rest of their net. Returns {net: [(x, y, diameter, drill), ...]}."""
    PINS = [("J_USB", "A4_B9", "VBUS", 0.45, 0.25), ("J_USB", "B4_A9", "VBUS", 0.45, 0.25),
            ("U_DRV", "4", "P24", 0.40, 0.20)]
    out = {}
    for el, pad, net, dia, drill in PINS:
        if el not in placed or el not in by: continue
        ex, ey, rot = placed[el]
        ang = math.radians(int(rot[1:]) if rot.startswith("R") else 0)
        ca, sa = math.cos(ang), math.sin(ang)
        smds = part_smds(by[el])
        if pad not in smds: continue
        lx, ly, _, _ = smds[pad]
        out.setdefault(net, []).append((ex + lx*ca - ly*sa, ey + lx*sa + ly*ca, dia, drill))
    return out

def main():
    errs = validate()
    if errs:
        for e in errs: print("ERR", e)
        raise SystemExit(f"{len(errs)} reference errors")

    force_w = float(sys.argv[1]) if len(sys.argv) > 1 else None
    placed, BW, BH, holes = place(force_w)
    signals = build_signals()

    layers_xml = "\n".join(
        f'<layer number="{n}" name="{esc(nm)}" color="7" fill="1" '
        f'visible="yes" active="yes"/>' for n, nm in LAYERS)

    plain = rounded_outline(BW, BH, CORNER_R)
    plain += "".join(f'<hole x="{x:.3f}" y="{y:.3f}" drill="{HOLE_DRILL}"/>'
                     for x, y in holes)

    bllib = (f'<library name="BLLIB"><description>BrushlessLamp generic land patterns'
             f'</description><packages>\n' + "\n".join(GENERIC_PKG[n][0] for n in PKG)
             + '\n</packages></library>')
    vendor_libs = "\n".join(VEND[n]["lib_xml"] for n in REAL_FILES)

    def element_xml(p):
        x, y, rot = placed[p["name"]]
        head = (f'<element name="{esc(p["name"])}" library="{esc(p["lib"])}" '
                f'package="{esc(p["pkg"])}"'
                + (f' value="{esc(p["value"])}"' if p["value"] is not None else "")
                + f' x="{x:.3f}" y="{y:.3f}" rot="{rot}"')
        attrs = p.get("attrs") or []
        if not attrs:
            return head + "/>"
        body = "".join(
            f'<attribute name="{esc(an.upper())}" value="{esc(av)}" x="{x:.3f}" '
            f'y="{y:.3f}" size="1.778" layer="27" rot="{rot}" display="off"/>'
            for an, av in attrs)
        return head + ">" + body + "</element>"
    elements = "\n".join(element_xml(p) for p in ITEMS)

    # Inner GND plane on layer 2 (Route2) -> makes the board genuinely 4-layer and
    # lets every GND pin drop a via to the plane instead of routing a track. The top
    # edge is NOTCHED around the WROOM antenna so no copper sits under it (RF).
    pi = PLANE_INSET
    mcu_item = next(p for p in ITEMS if p["name"] == "U_MCU")
    a, b, c, d = fr(mcu_item); mxx, my0 = placed["U_MCU"][0], placed["U_MCU"][1]
    nx0 = max(pi, mxx + a - 1.5); nx1 = min(BW - pi, mxx + c + 1.5)
    ny = my0 + b + (d - b) - 11.0                  # exclude the top ~11 mm (antenna) of the module
    verts = [(pi, pi), (BW-pi, pi), (BW-pi, BH-pi)]
    if pi < ny < BH - pi and nx0 < nx1:            # cut a notch into the top edge
        verts += [(nx1, BH-pi), (nx1, ny), (nx0, ny), (nx0, BH-pi)]
    verts += [(pi, BH-pi)]
    # Sig / GND(Route2) / PWR(Route15) / Sig stack: GND + P3V3 inner planes, both
    # notched around the antenna. Power/ground via straight to a plane -> the router
    # only has to place the signal nets + P24 on Top/Bottom.
    def plane(layer):
        return (f'<polygon width="0.2032" layer="{layer}" spacing="1.27" pour="solid" '
                f'isolate="0.3" thermals="yes" rank="1">'
                + "".join(f'<vertex x="{vx:.3f}" y="{vy:.3f}"/>' for vx, vy in verts)
                + '</polygon>')
    gnd_plane, p3v3_plane = plane("2"), plane("15")
    _by = {p["name"]: p for p in ITEMS}
    via_by_net = {"GND": list(gnd_thermal_vias(placed, _by))}
    for net, vs in power_escape_vias(placed, _by).items():
        via_by_net.setdefault(net, []).extend(vs)
    def via_xml(net):
        return "".join(
            f'<via x="{vx:.4f}" y="{vy:.4f}" extent="1-16" drill="{drill}" diameter="{dia}"/>'
            for vx, vy, dia, drill in via_by_net.get(net, []))
    def sig_xml(net, refs):
        cls = NET_CLASS.get(net, "0")
        extra = via_xml(net) + (gnd_plane if net == "GND" else (p3v3_plane if net == "P3V3" else ""))
        return (f'<signal name="{esc(net)}"' + (f' class="{cls}"' if cls != "0" else "") + '>'
                + "".join(f'<contactref element="{esc(en)}" pad="{esc(pad)}"/>' for en, pad in refs)
                + extra + '</signal>')
    signals_xml = "\n".join(sig_xml(net, refs) for net, refs in sorted(signals.items()))

    doc = f'''<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE eagle SYSTEM "eagle.dtd">
<eagle version="9.6.2">
<drawing>
<settings>
<setting alwaysvectorfont="no"/>
<setting verticaltext="up"/>
</settings>
<grid distance="0.1" unitdist="inch" unit="inch" style="lines" multiple="1" display="no" altdistance="0.01" altunitdist="inch" altunit="inch"/>
<layers>
{layers_xml}
</layers>
<board>
<plain>
{plain}
</plain>
<libraries>
{bllib}
{vendor_libs}
</libraries>
<attributes/>
<variantdefs/>
<classes>
<class number="0" name="default" width="0" drill="0"/>
<class number="1" name="pwr24" width="0.5" drill="0"/>
<class number="2" name="pwr3v3" width="0.4" drill="0"/>
</classes>
<designrules name="default"/>
<elements>
{elements}
</elements>
<signals>
{signals_xml}
</signals>
</board>
</drawing>
</eagle>
'''
    open("BrushlessLamp.brd", "w").write(doc)
    print(f"board {BW:.1f} x {BH:.1f} mm  ({BW*BH/100:.1f} cm^2)  4-layer-ready, single-side")
    print(f"elements={len(ITEMS)}  signals={len(signals)}  "
          f"contactrefs={sum(len(r) for r in signals.values())}  "
          f"holes={len(holes)} (M2.5)  corner_r={CORNER_R}mm")

if __name__ == "__main__":
    main()
