# 06 — PCBWay Manufacturing + Assembly (PCBA)

What PCBWay needs to **fabricate and assemble** the board, what's auto-generated from
your finished layout, and the BOM work to finalize. Assumes you've completed the 2D PCB
(placement + routing).

## The deliverables

| # | File | Purpose | Source |
|---|------|---------|--------|
| 1 | **Gerbers** (RS-274X) + **NC drill** | fabricate the bare board | Fusion CAM (after layout) |
| 2 | **Pick-and-Place / Centroid** (RefDes, X, Y, Rotation, Layer) | *where + orientation* of every placed part | Fusion CAM (after layout) |
| 3 | **BOM (.xlsx)** | *which* part at each RefDes | ✅ [`PCBWay-BOM.xlsx`](PCBWay-BOM.xlsx) |
| 4 | (optional) assembly drawing | only if silk lacks polarity marks | not needed — silk carries them |

Files 1 & 2 **are the placement documentation** — you don't write it by hand; routing the
board produces it. File 3 is **done**.

## Checklist vs PCBWay's SMT ordering guide

Per [pcbway.com/smt_ordering_guide.html](https://www.pcbway.com/smt_ordering_guide.html):

| PCBWay requires | Status |
|---|---|
| **BOM as Excel `.xlsx`** (they reject PDF/CSV), columns: Designators, Qty, Package, Part number / value, **Type PTH-or-SMD** | ✅ [`PCBWay-BOM.xlsx`](PCBWay-BOM.xlsx) — exact columns, Type = SMD/PTH, comma-separated designators |
| **DNP clearly indicated** | ✅ `Populate = DNP` (rows highlighted) for `D_CLW/D_CLC`, `J_PROG` |
| **Pick&place / centroid, designators matching the BOM** (THT may be excluded) | ⏳ export from Fusion after routing; our RefDes are consistent |
| **Gerbers** + silkscreen | ⏳ export from Fusion after routing |
| **Polarity / pin-1 on silkscreen** (cathode bar, `+`, pin-1) — or an assembly drawing | ✅ vendor footprints carry their own; the generic diodes (cathode bar), polarised caps (`+`), and SOT-23-6 buck (pin-1 dot) now have silk marks too |
| Orientation differs from PCBWay convention → clarify | review the placement preview PCBWay sends before they build |

## Design rules — load [`PCBWay.dru`](PCBWay.dru) before routing

A ready PCBWay rule file lives at [`../PCB/PCBWay.dru`](PCBWay.dru). Load it in Fusion's
PCB editor: **Rules / DRC dialog → Load… → select `PCBWay.dru` → Apply**. It sets:

| Rule | Value | PCBWay limit | Margin |
|---|---|---|---|
| Min trace width | 6 mil (0.152 mm) | 4 mil / 0.1 mm | safe |
| Min clearance | 6 mil (0.152 mm) | 4 mil / 0.1 mm | safe |
| Min finished drill | 0.30 mm | 0.15 mm | no small-hole surcharge |
| Via annular ring | ≥ 6 mil (0.15 mm) | 0.15 mm | at spec |
| Pad annular ring | ≥ 10 mil | 0.15 mm | safe |
| Copper → board edge | 16 mil (0.4 mm) | 0.25 mm | safe |
| Solder-mask expansion | 4 mil | — | standard |

Running DRC with this loaded clears the default-ruleset **Drill Size / Copper Width**
flags. (Overlap / Board-Outline / Air-Wire flags are placement/routing, not rules — they
clear as you place and route.) The board is **2-layer** (the `.dru` is already set for it);
the WROOM-1 carries its own RF ground internally so no inner plane is needed.

## Exporting 1 & 2 from Fusion (after routing)

1. PCB editor → **Manufacturing** (or **Outputs / CAM**).
2. **Generate Gerbers + Drill:** export an RS-274X Gerber set + Excellon drill. PCBWay
   accepts the standard EAGLE/Fusion CAM gerber set (`.GTL/.GBL/.GTS/.GBS/.GTO/.GBO/.GKO`
   + `.DRL`). Zip them.
3. **Generate the Place/Centroid file:** export the **pick-and-place** (a.k.a. "mount" /
   centroid) `.csv`. Columns: `Designator, Mid X, Mid Y, Rotation, Layer`.
4. Note the **stackup** you used (2-layer, 1.6 mm — see [`04-fusion-workflow…md`](04-fusion-workflow-and-manufacturing.md)) so you set the same on the PCBWay order.

## BOM — ready: [`06-bom-assembly.csv`](06-bom-assembly.csv)

**The assembly-ready BOM is done** — [`06-bom-assembly.csv`](06-bom-assembly.csv) is
reconciled 1:1 against the 71 parts in the schematic, with a concrete MPN per line
(including the 4 part-specific picks), `Populate` flags (DNP = `D_CLW/D_CLC`, `J_PROG`),
and LCSC numbers where confirmed. Hand that file to PCBWay alongside the Gerbers +
centroid. Before ordering, just: (a) verify the few LCSC# marked "verify", and (b)
double-check the 4 `*`-flagged part-specific land patterns match the parts (below).

Every line has:
- **RefDes** (grouped, e.g. `R_NSLEEP,R_NRESET,...`)
- **Qty**
- **Value / description** (for passives: package + value + tolerance + voltage/dielectric)
- **MPN** (orderable) — or "generic, any" for jellybean passives on turnkey
- **DNP** flag for do-not-populate parts

### MPNs already confirmed (the ones that matter)
| RefDes | MPN | Source |
|---|---|---|
| U_MCU | ESP32-S3-WROOM-1-N8R8 | Espressif |
| U_DRV | DRV8313PWPR | TI |
| U_BUCK | LMR51430YDDCR (LCSC C5210749) | TI |
| D_USB | USBLC6-2SC6 | ST |
| Q_WW, Q_CW | AO3400A (LCSC C20917) | — |
| J_USB | TYPE-C-31-M-12 (LCSC C165948) | HRO |
| J_BTN | B2B-XH-A | JST |
| J_KNOB, J_LED | B3B-XH-A | JST |
| J_PWR | KF301-5.0-2P | — |
| J_MOTOR | KF301-5.0-3P | — |
| J_SENSOR | KF350-3.5-5P | — |

### Passives / discretes
Turnkey-OK as generic specs (PCBWay/LCSC house parts), e.g. `R_* = 0603, value, 1%`;
`C_* = 0603/0805/0402, value, X7R/C0G, ≥ rated V`; `D_Q1 = BZT52C10` (SOD-123);
`TVS1 = SMBJ28A` (SMB); `D_OR_USB = SS14` (SMA); `Q1 = AO3401A` or similar P-FET (SOT-23).
Give the value+package and let them source, or pin exact LCSC numbers from
`03-bom-mainboard.csv`.

### ⚠️ Decide these 4 before ordering (footprint depends on the exact part)
Per [`fusion/FOOTPRINTS.md`](fusion/FOOTPRINTS.md):
- **C_IN_BULK, C_VM_BULK** — 100 µF / 50 V aluminium electrolytic: pick a can size (e.g.
  8×10 mm SMD V-chip) and confirm the `CAP_V` land matches.
- **C_MCU_BULK** — 470 µF polymer (e.g. Panasonic SP-Cap / 7343 "D"): confirm `CAP_D`.
- **L1** — 4.7 µH ≥1.5 A shielded (e.g. Würth 74438335047): confirm `IND43` land.
- **SW_RST, SW_BOOT** — SMD tactile: confirm `TACT` land.

### DNP (do not populate)
Mark these so PCBWay skips them: `C_DP`, `C_DM` (USB EMI caps, reserved), `D_CLW`, `D_CLC`
(LED drain clamps), `R_AZ_A`/`R_AZ_B`/`R_AZ_C` (encoder pull-up stiffening, populate only
for a long/noisy cable), and `J_PROG` (bare fallback pads — no part). `C_BOOT` / `C_FF` are
optional — populate or DNP as you choose.

## PCBWay assembly form — values

For the "Other Parameters" count fields (optional; the uploaded BOM is authoritative):

| Field | Value |
|---|---|
| Number of Unique Parts | **42** (distinct kinds placed) |
| Number of SMD Parts | **61** |
| Number of BGA/QFP Parts | **0** (DRV8313 = HTSSOP, WROOM-1 = module) |
| Number of Through-Hole Parts | **7** (`J_PWR/J_MOTOR/J_SENSOR/J_KNOB/J_BTN/J_LED` + `J_EXP`) |
| Total placed | 65 (DNP: `D_CLW`, `D_CLC`, `R_AZ_A/B/C`, `J_PROG`) |

Form choices: **Turnkey**, **Single pieces** (re-check vs panel after layout if a board
side < 50 mm), **Top side** (all SMD on top; THT assembled separately), sensitive
components = **No** (WROOM-1 is MSL-3 but handled routinely), China substitutes = **No**
(+ remark: "exact MPN for ICs/module/connectors; equivalent passives OK").

## PCBWay order notes
- **Turnkey vs consigned:** turnkey = they source all parts from your MPNs; consigned =
  you ship parts to them. The ESP32 module + DRV8313 are easiest turnkey.
- **Placement preview:** PCBWay sends a component-placement preview for **your approval**
  before assembly — **check pin-1 / polarity / connector orientation** there (the common
  centroid-rotation gotcha for polarized parts: diodes, electrolytics, ICs, USB-C).
- **2-layer, 1.6 mm, no controlled impedance** — RF is inside the WROOM-1 module and USB
  is full-speed, so no impedance control needed. Just respect the WROOM-1 antenna keep-out
  (already in the module footprint) and keep a solid bottom ground pour.
- **THT parts:** the 7 connectors (screw terminals, JST-XH, header) are through-hole —
  confirm PCBWay's quote includes THT hand-assembly for them (a small surcharge vs the
  all-SMT parts).

## Bottom line
Once the board is routed, **Gerbers + Centroid carry all the placement info** PCBWay needs
— that's automatic. The single thing to finish by hand is the **assembly BOM**: confirm an
MPN per line, mark DNP, and lock the 4 part-specific components above.
