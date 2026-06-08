# 04 — Fusion Electronics Workflow & Manufacturing

Autodesk Fusion's Electronics environment **is the former Eagle** — its libraries are
Eagle `.lbr` under the hood, and schematic + board are two linked documents inside one
Fusion design.

## 1. Getting symbols + footprints

| Part | Best source | Format / note |
|---|---|---|
| ESP32-S3-WROOM-1(-N8R8) | **SnapEDA / SnapMagic** (exports Eagle `.lbr` directly) or **UltraLibrarian**; **Espressif official KiCad libs** as a cross-check | grab the Eagle `.lbr`; validate pinout vs Espressif's |
| DRV8313PWPR (HTSSOP-28) | **SnapEDA / SnapMagic** (verified symbol + thermal-pad footprint) or **SamacSys / Component Search Engine** | Eagle `.lbr`; TI ships no Eagle lib |
| MT6701QT-STD | likely build it: generic **QFN16 3×3 0.5 mm** footprint + custom symbol (SnapEDA may not have it) | confirm pad 1 / center pad |
| AO3400A | any generic **SOT-23** (Eagle built-in `transistor` lib has SOT23) or SnapEDA | trivial |
| USB-C 16-pin | SnapEDA / SamacSys for your exact LCSC PN | match the assembled PN |
| LMR51430 (SOT-23-6) | SnapEDA / SamacSys, or generic SOT-23-6 + custom symbol | verify pinout vs datasheet |
| USBLC6-2SC6 | SnapEDA (SOT-23-6L) | — |
| 0402/0603 passives | **Fusion built-in managed libs** (`rcl`, `resistor`, `capacitor`) | no download |

**Three ways to pull parts into Fusion:**
1. **SnapEDA/SnapMagic & UltraLibrarian** → download Eagle `.lbr`, then *Library Manager
   → add to project* (or use the SnapMagic Eagle plugin to push directly).
2. **SamacSys / Component Search Engine** → Autodesk publishes a free SamacSys Eagle
   library plugin that works inside Fusion Electronics.
3. **Fusion managed library / "Place → Manufacturer Part"** → enable the Library.io
   filter in Library Manager for cloud-hosted, supply-chain-linked parts.

> **KiCad-vs-Eagle caveat:** Fusion is natively Eagle. It can import whole **KiCad
> *projects*** (lossy, for boards/schematics — not a clean drag-in of KiCad symbol/
> footprint libraries). Since many modern libs (incl. Espressif's) are KiCad-first,
> **prefer the Eagle `.lbr`** export from SnapEDA/UltraLibrarian/SamacSys, and use the
> Espressif KiCad lib only as a pinout reference. (Autodesk has announced Eagle EOL and
> is folding everything into Fusion — doesn't change today's flow.)

## 2. Design flow

1. **New Fusion design → Electronics → Schematic.** Place parts; each device carries its
   assigned footprint. For multi-package parts pick the right variant (WROOM-1 vs -1U;
   DRV8313 PWP).
2. **Capture by block** following [`02-schematic-blocks.md`](02-schematic-blocks.md):
   MCU → USB → power(protection+buck) → DRV8313 → LED switches → connectors. Net-label
   liberally; the ref-des namespacing in that doc maps 1:1.
3. **ERC** clean on the schematic.
4. **Switch to PCB Document** (push netlist to board).
5. **Layout** (§3 below). **DRC** against the JLCPCB rule set.
6. **CAM** → Gerbers (RS-274X) + NC drill (or ODB++), plus **BOM** + **pick-and-place
   (centroid)** for assembly.

## 3. Layout priorities (this board specifically)

- **2-layer with a solid bottom ground pour** (via-stitched). The WROOM-1 module carries
  its own RF ground + shield internally, so an external dedicated plane isn't required —
  the continuous bottom pour is the reference. The job a 4th layer would do (isolating
  noisy returns) is handled by **return-aware routing**: bring motor/LED return currents
  back to the power connector so they don't cross under the MCU/RF ground (see star-ground
  below).
- **RF keep-out:** WROOM-1 antenna end hangs off the board edge, ground-cleared per the
  module footprint. Nothing (no copper/pour) under the antenna on either layer.
- **Star ground** the 24 V power return at `J_PWR`; motor (DRV8313 PGND/bulk) and LED
  return currents come back there, not through the ESP32 ground.
- **DRV8313 power loop** tight: VM bulk → device → PGND. Wide OUT1/2/3 copper to the
  motor connector. Charge-pump caps hard against pins 1/2/3. Exposed pad → ground pour
  + 4–9 thermal vias.
- **Buck hot-loop** (C_IN → VIN/GND) tight; small SW node; feedback divider away from
  the SW node and inductor field.
- **470 µF MCU bulk** sits on the module's 3V3 pads (not at the buck).
- **USB** D+/D− kept short and side-by-side over the bottom ground pour; USBLC6 at the
  connector. (USB 2.0 full-speed — no controlled impedance needed; just keep it short.)
- **24 V trace widths** sized for motor + LED current (use a pour/plane region); keep
  high-dV/dt SW node and OUTx away from the antenna and the encoder/knob signal traces.

## 4. Manufacturing (JLCPCB)

- **2-layer, no controlled impedance** — the RF is inside the WROOM-1 module (no external
  50 Ω antenna trace) and USB is full-speed (no 90 Ω requirement), so a plain 2-layer
  board is sufficient and cheaper. (4-layer is optional margin only — not needed for this
  module-based, low-power, low-speed design.)
- **Design rules:** load [`../PCB/PCBWay.dru`](../PCBWay.dru) (or target **6 mil trace/space,
  0.3 mm drill**) to stay in the cheapest
  tier (JLC supports finer, but you don't need it — WROOM-1 castellated/LGA routes fine
  at 6 mil).
- **All-SMD board, through-hole connectors.** Every active/passive (passives, ICs, SOT-23
  MOSFETs, SMD tactile programming buttons) is surface-mount → **one SMT reflow pass**. The
  six off-board connectors are the only through-hole parts: keyed **JST-XH**
  (`J_KNOB` / `J_BTN` / `J_LED`) and screw terminals (`J_PWR` / `J_MOTOR` / `J_SENSOR`),
  chosen THT for mechanical strength. Add them to **JLCPCB's through-hole assembly** (extra
  process step / cost) **or hand-solder** the six after SMT reflow. Keep all SMD parts on
  the top side to avoid a second-side reflow charge.
- **Assembly (LCSC parts):** prefer **Basic/Preferred** parts for passives + AO3400A
  (C20917) + common SOT-23 to avoid feeder fees; **Extended** parts (DRV8313 C92482,
  LMR51430 C5210749, WROOM-1, MT6701 C2913974, USBLC6 C7519) incur a small per-part
  setup fee — confirm stock before locking the BOM.
- **THT connectors:** the screw terminals (`J_PWR` / `J_MOTOR` / `J_SENSOR`) and pin
  headers (`J_KNOB` / `J_BTN` / `J_LED`) take the cable strain — that's why they're
  through-hole. Place them on the board edges (power/motor/encoder on the 24 V edge).
- **Verify every LCSC part number before ordering.** The numbers in
  [`03-bom-mainboard.csv`](03-bom-mainboard.csv) are correct for the key ICs (DRV8313
  C92482, LMR51430 C5210749, USBLC6 C7519, AO3400A C20917, MT6701 C2913974) but the
  passives / module / connector numbers are indicative — confirm each against LCSC
  stock and the exact package you want.
- Generate **BOM + CPL/centroid** from Fusion CAM; match column names to JLC's template.
- Mark TVS drain-clamps, DNP pull-ups, and optional caps as **DNP** so they aren't
  placed.
- **MT6701 1024-PPR EEPROM burn is NOT done by assembly** — it needs 5 V (see
  [`02-schematic-blocks.md`](02-schematic-blocks.md) Block E). Source a pre-programmed
  part/module, or run a firmware/jig programming step at bring-up.

## 5. What lands back in this folder

Once capture starts, commit into `PCB/`:
- the Fusion design (`.f3z` archive) and/or exported `.sch` + `.brd`,
- generated `gerbers/` (zip ready for JLC),
- `cam/` BOM + centroid,
- any custom `.lbr` you author (e.g. the MT6701 footprint).

## Sources
- SnapEDA/SnapMagic — <https://www.snapeda.com> · UltraLibrarian — <https://app.ultralibrarian.com>
- SamacSys / Component Search Engine + Autodesk Eagle plugin —
  <https://www.samacsys.com/library-loader/> ·
  <https://www.autodesk.com/products/fusion-360/blog/now-available-new-free-eagle-library-plugin-samacsys/>
- Espressif KiCad libraries — <https://github.com/espressif/kicad-libraries>
- Fusion managed/Eagle libraries & KiCad import —
  <https://www.autodesk.com/support/technical/article/caas/sfdcarticles/sfdcarticles/How-to-use-and-see-EAGLE-managed-libraries-in-Fusion-360.html>
- JLCPCB impedance / ESP32 layer guidance — <https://jlcpcb.com/impedance> ·
  <https://jlcpcb.com/blog/how-to-design-an-esp32-s2-module-pcb> ·
  <https://docs.espressif.com/projects/esp-hardware-design-guidelines/en/latest/esp32/pcb-layout-design.html>
