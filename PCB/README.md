# BrushlessLamp — Custom PCB

Folder for the **fully custom control PCB** that replaces the two pre-built
modules currently used in the lamp:

| Today (modules on a simple carrier PCB) | This project (one custom board) |
|---|---|
| Seeed **XIAO ESP32-S3** (ESP32-S3R8 module) | bare **ESP32-S3-WROOM-1-N8R8** module + support circuitry |
| **SimpleFOC Mini** (DRV8313 breakout) | **DRV8313** driver block laid out inline |
| **Mini360** buck (24 V → 5 V) + XIAO LDO | onboard **24 V → 3.3 V** synchronous buck |
| **MT6701** breakout at the motor | **same MT6701 breakout**, plugged into a `J_SENSOR` connector |
| 2× **AO3400A** flying MOSFETs | 2× AO3400A LED low-side switches inline |
| USB on the XIAO | **USB-C** + native USB-Serial-JTAG |

The firmware does **not** change behaviorally — same ESP-IDF + esp-matter image,
same FOC loop, same control surface. Only the GPIO map moves to a new
`BRUSHLESSLAMP_BOARD_CUSTOM` branch in [`s3/common/pins.h`](../s3/common/pins.h)
(proposed map in [`01-architecture-and-pinmap.md`](01-architecture-and-pinmap.md)).

The board is designed in **Autodesk Fusion (Electronics / former Eagle)**.

---

## Read in this order

1. **[`01-architecture-and-pinmap.md`](01-architecture-and-pinmap.md)** — system block
   diagram, power tree, the 2-board split, and the full ESP32-S3 GPIO assignment
   mapped from the firmware.
2. **[`02-schematic-blocks.md`](02-schematic-blocks.md)** — schematic-level design of
   every block (MCU core, USB, power, DRV8313 driver, encoder satellite, LED
   switches) with component lists and net-by-net connections.
3. **[`03-bom-mainboard.csv`](03-bom-mainboard.csv)** — main-board bill of materials with
   LCSC part numbers for JLCPCB assembly. (The encoder is the existing breakout — only a
   connector is on the main board; **[`03-bom-sensor.csv`](03-bom-sensor.csv)** is the
   optional custom-satellite BOM, not part of the baseline.)
4. **[`04-fusion-workflow-and-manufacturing.md`](04-fusion-workflow-and-manufacturing.md)**
   — where to get the symbols/footprints, the Fusion Electronics design flow, and
   JLCPCB/PCBWay fab/assembly notes (2-layer, DRC).
5. **[`05-build-plan-checklist.md`](05-build-plan-checklist.md)** — the actual
   step-by-step plan: milestones, what to do in what order, and the risks to retire
   early.
6. **[`fusion/`](fusion/)** — a ready-to-open **EAGLE 9.x `.sch`** (`BrushlessLamp.sch`)
   with all 71 parts, the full netlist wired, and a footprint on every part. The six key
   parts (WROOM-1, DRV8313, USB-C, USBLC6, 2× AO3400) use the **official vendor symbols +
   footprints** from [`components/`](components) (downloaded SnapMagic `.lbr`); the rest
   use standard footprints. Plus the generator script and an import guide. See
   [`fusion/README.md`](fusion/README.md).
7. **[`components/`](components/)** — the official vendor `.lbr` libraries (ESP32-S3-
   WROOM-1, DRV8313, USB-C, USBLC6, AO3400) embedded into the schematic above.
- **[`PCBWay.dru`](PCBWay.dru)** — ready-to-load PCBWay design-rule file for Fusion's DRC
  (6 mil / 0.3 mm drill, within PCBWay's current limits). Load it before routing.
- **[`PCBWay-BOM.xlsx`](PCBWay-BOM.xlsx)** — assembly BOM in PCBWay's required Excel format
  (Designator / Qty / MPN / Package / Type SMD-or-PTH, DNP highlighted). Upload this for PCBA.

8. **[`06-pcbway-assembly.md`](06-pcbway-assembly.md)** + **[`06-bom-assembly.csv`](06-bom-assembly.csv)**
   — what PCBWay (or any PCBA house) needs to fabricate + assemble: Gerbers + centroid
   (auto-generated from your layout) and a finished **assembly BOM** (71 parts, MPN per
   part, DNP marked, reconciled to the schematic). Footprint dimensions are in
   [`fusion/FOOTPRINTS.md`](fusion/FOOTPRINTS.md).

---

## The five design decisions that shape everything

1. **Use the `ESP32-S3-WROOM-1-N8R8` module, not the bare QFN chip.** It is the
   exact silicon in the XIAO (ESP32-S3R8: 8 MB flash + 8 MB octal PSRAM), and it
   carries the crystal, flash/PSRAM, RF balun, tuned antenna **and FCC/IC/CE
   modular approval** inside the shield can. Going chip-down would mean designing
   and tuning your own RF matching network and paying for full intentional-radiator
   certification — not worth it below ~10 k units. *(Caveat: GPIO35/36/37 are owned
   by the octal PSRAM and stay reserved even though the firmware disables PSRAM —
   you cannot reclaim them.)*

2. **Single-stage 24 V → 3.3 V buck (LMR51430), no 5 V rail.** Nothing on the board
   needs 5 V — the motor and LEDs run straight off 24 V, and USB VBUS stays local to
   the connector. The LMR51430 is 4.5–36 V (real margin over a mains-derived 24 V),
   3 A synchronous (≈6× the ESP32's ~500 mA RF-TX burst), in SOT-23-6. The brownout
   history from commissioning is fixed in the **layout** — a 470 µF low-ESR bulk cap
   parked right at the module's 3V3 pins.

3. **Replicate the SimpleFOC Mini's DRV8313 block almost verbatim.** It is
   open-source, and its wiring (EN tied high, nSLEEP clears faults, voltage-mode /
   no current sense) is *exactly* this project's intent. ~7 unique parts.

4. **Reuse the existing MT6701 breakout — just add a connector.** The sensor stays the
   off-the-shelf module already in the lamp (already ABZ mode, 1024 PPR / 4096 CPR), kept at the
   motor shaft and plugged into a `J_SENSOR` connector with a 4–5-wire cable. Nothing to
   place or program on the custom board. (A fully-custom bare-chip satellite is
   documented as optional only — it would add a one-time 5 V EEPROM-programming step.)

5. **2-layer, native-USB-only.** The RF lives **inside the certified WROOM-1 module**
   (not a bare chip), and the board is low-power / low-speed, so a **2-layer** board with a
   solid bottom ground pour + return-aware routing is sufficient — no 4th layer needed
   (cheaper). Native USB-Serial-JTAG (like the XIAO) means **no UART bridge and no
   auto-reset transistors** — just BOOT + RESET buttons and a USB-C connector.

6. **All-SMD board, through-hole connectors.** Every active/passive — including the
   AO3400A LED switches (SOT-23, replacing the breadboard's through-hole MOSFETs) — is
   surface-mount for a single SMT reflow. The off-board **connectors are deliberately
   through-hole** for strength: keyed **JST-XH** for button / knob / LED,
   screw terminals for power / motor / encoder, and the `J_EXP` spare-GPIO pin header —
   seven THT parts in total. They're added to JLCPCB's THT assembly or
   hand-soldered after reflow.

## Programming / flashing the ESP32

**Primary path: USB-C.** The ESP32-S3's built-in USB-Serial-JTAG controller (GPIO19/20)
gives **firmware flash + serial console + JTAG debug over one USB-C cable** — no bridge
chip, exactly like the XIAO. `idf.py -p <PORT> flash monitor` just works; BOOT + RESET
buttons are a manual recovery path only. Full design in
[`02-schematic-blocks.md` → Block B](02-schematic-blocks.md#block-b--usb-c-programmingflashing--native-usb).

**Fallback: a 6-pin UART header (`J_PROG`)** exposing `3V3 / GND / EN / GPIO0 / U0TXD /
U0RXD` as unloaded pads — lets an external USB-UART adapter re-flash the bootloader even
if the native-USB stack is ever bricked. Cheap insurance; populate only if needed.

---

## Status

- [x] Deep research on every subsystem (see commit history / agent briefs).
- [x] Architecture, pin map, schematic-block design, BOM, workflow documented here.
- [ ] Schematic capture in Fusion Electronics.
- [ ] PCB layout (2-layer) + DRC.
- [ ] Fab + assembly (JLCPCB).
- [ ] Bring-up + firmware `BRUSHLESSLAMP_BOARD_CUSTOM` pin branch.

> Nothing here is fabbed yet — this is the design package. The CAD files (`.f3z` /
> schematic / board) will live in this folder once capture begins.
