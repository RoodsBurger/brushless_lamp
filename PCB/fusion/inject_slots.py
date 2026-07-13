#!/usr/bin/env python3
"""Inject plated slots (Excellon G85) into a drill file exported by Fusion CAM.

EAGLE/Fusion cannot encode slotted drills natively; this board represents each slot
as a round pad plus a layer-46 (Milling) wire whose width equals the slot width.
Fusion's CAM export writes only the round drill, which is how PCBWay ended up
drilling 0.6 mm round holes for the USB-C shell legs. This script reads the .brd,
collects every layer-46 wire (element-local wires are transformed by element
position/rotation), and appends matching G85 slot records to the Excellon file.

Usage:
    python3 inject_slots.py BrushlessLampRoutes.brd drill_1_16.xln

Writes <drill>.slotted.xln next to the input and prints a PCBWay-ready fab note.
"""
import math, re, sys
import xml.etree.ElementTree as ET


def rot_pt(x, y, rot):
    m = rot or "R0"
    mir = m.startswith("M")
    ang = int(m.lstrip("MR") or 0)
    if mir:
        x = -x
    a = math.radians(ang)
    return (x * math.cos(a) - y * math.sin(a), x * math.sin(a) + y * math.cos(a))


def collect_slots(brd_path):
    root = ET.parse(brd_path).getroot()
    pkg_slots = {}
    for lib in root.iter("library"):
        for p in lib.iter("package"):
            slots = [(float(w.get("x1")), float(w.get("y1")),
                      float(w.get("x2")), float(w.get("y2")), float(w.get("width")))
                     for w in p.findall("wire") if w.get("layer") == "46"]
            if slots:
                pkg_slots[(lib.get("name"), p.get("name"))] = slots
    out = []
    for e in root.iter("element"):
        key = (e.get("library"), e.get("package"))
        if key not in pkg_slots:
            continue
        ex, ey, erot = float(e.get("x")), float(e.get("y")), e.get("rot") or "R0"
        for x1, y1, x2, y2, w in pkg_slots[key]:
            a = rot_pt(x1, y1, erot)
            b = rot_pt(x2, y2, erot)
            out.append((ex + a[0], ey + a[1], ex + b[0], ey + b[1], w))
    plain = root.find(".//board/plain")
    if plain is not None:
        for w in plain.findall("wire"):
            if w.get("layer") == "46":
                out.append((float(w.get("x1")), float(w.get("y1")),
                            float(w.get("x2")), float(w.get("y2")), float(w.get("width"))))
    return out


def inject(drill_path, slots):
    text = open(drill_path).read()
    if "METRIC" not in text.upper() and "M71" not in text:
        sys.exit("drill file does not look metric — export CAM with metric units")
    lines = text.splitlines()
    # existing tools: T<num>C<dia>
    tools = {}
    hdr_end = 0
    for i, ln in enumerate(lines):
        m = re.match(r"T(\d+)C([\d.]+)", ln)
        if m:
            tools[float(m.group(2))] = int(m.group(1))
        if ln.strip() == "%" or ln.strip() == "M95":
            hdr_end = i
            break
    widths = sorted({round(s[4], 3) for s in slots})
    next_t = max(tools.values(), default=0) + 1
    new_tools = []
    for wdt in widths:
        if wdt not in tools:
            tools[wdt] = next_t
            new_tools.append(f"T{next_t:02d}C{wdt:.3f}")
            next_t += 1
    # match existing coordinate style: implied 3-decimal (X25775) vs explicit (X25.775)
    implied = bool(re.search(r"^X\d+Y\d+$", text, re.M)) and "." not in \
        (re.search(r"^X(\S+?)Y", text, re.M).group(1))
    def fmt(v):
        return f"{round(v * 1000):d}" if implied else f"{v:.3f}"
    body = []
    for wdt in widths:
        body.append(f"T{tools[wdt]:02d}")
        for x1, y1, x2, y2, w in slots:
            if round(w, 3) != wdt:
                continue
            body.append(f"X{fmt(x1)}Y{fmt(y1)}G85X{fmt(x2)}Y{fmt(y2)}")
    out = lines[:hdr_end] + new_tools + lines[hdr_end:]
    # insert slot records before M30 (end of program)
    for i in range(len(out) - 1, -1, -1):
        if out[i].strip() == "M30":
            out[i:i] = body
            break
    else:
        out += body
    dst = re.sub(r"(\.[^.]+)$", r".slotted\1", drill_path)
    open(dst, "w").write("\n".join(out) + "\n")
    return dst


if __name__ == "__main__":
    brd, drill = sys.argv[1], sys.argv[2]
    slots = collect_slots(brd)
    if not slots:
        sys.exit("no layer-46 milling wires found")
    dst = inject(drill, slots)
    print(f"{len(slots)} slot(s) written -> {dst}")
    print("\nPCBWay fab note (paste into order remarks):")
    print("  The drill file contains G85 plated slots for the USB-C (TYPE-C-31-M-12)")
    print("  shell legs. Please cut these as PLATED slots, not round drills:")
    for x1, y1, x2, y2, w in slots:
        L = math.hypot(x2 - x1, y2 - y1) + w
        print(f"    slot {w:.2f} x {L:.2f} mm centered ({(x1+x2)/2:.3f}, {(y1+y2)/2:.3f}) mm")
