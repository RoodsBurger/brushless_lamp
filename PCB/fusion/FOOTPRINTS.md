# Footprint reference — BrushlessLamp custom PCB

Every footprint on the board, its source, and its dimensions, extracted from the
shipping `BrushlessLamp.brd`. All packages are byte-identical across `.brd`, `.sch`,
and the library files in [`../components/`](../components) (verified by
`check_libraries.py` — drift there makes Fusion flag "inconsistent footprints").

Two classes:

1. **Vendor/verified footprints** — official manufacturer land patterns
   (SnapMagic downloads) or datasheet-rebuilt patterns after the PCBWay
   T-1P9W1088173A manufacturability review.
2. **Generic footprints** — IPC-7351B density-B (nominal) chip lands.

---

## 1. Vendor / datasheet-verified footprints

| Part(s) | Library (`../components/`) | Package | Notes |
|---|---|---|---|
| `U_MCU` | ESP32-S3-WROOM-1-N8R8 | Espressif module land + antenna keep-out | SnapMagic; 3D = user STEP |
| `U_DRV` | DRV8313PWPR | HTSSOP-28 + exposed pad (windowpane paste) | SnapMagic |
| `J_USB` | TYPE-C-31-M-12 | HRO USB-C 2.0 receptacle | shell legs = **plated slots 4× 0.6×1.7 mm**, drawn as slot-isle long pads + layer-46 milling + 9 overlapping Ø0.6 gang drills each (survives Fusion CAM; PCBWay order note required) |
| `D_USB` | USBLC6-2SC6 | SOT-23-6 | SnapMagic |
| `Q_WW`, `Q_CW` | AO3400A | SOT-23 | SnapMagic |
| `SW_RST`, `SW_BOOT` | TS-1187A | XKB TS-1187A-B-A-B 4-pad SMD tact | datasheet-rebuilt: pads 1.0×0.75 at (±3.0, ±1.875); internal contacts bridge pads **pairwise per side** — deviceset connects pin1→pads "1 2", pin2→"3 4" |
| `J_KNOB`, `J_LED` | JST-XH-BL | XH3 (B3B-XH-A top entry) | drill 0.95 / Ø1.7, housing 9.9×5.75 drawn true size |
| `J_BTN` | JST-XH-BL | XH2 (B2B-XH-A) | drill 0.95 / Ø1.7, housing 7.4×5.75 |
| `J_SENSOR` | KF350-SENSOR | SCREW5 — 5-pos 3.5 mm terminal row | populated by gangable KF350-3.5-**3P** (LCSC C474893) + **2P** (C474892); BOM lines `J_SENSOR-A`/`-B` both mount at this one designator |

## 2. Generic footprints (IPC-7351B nominal unless noted)

Chip lands: two pads `padX × padY` mm centered at `±offset` on the pin axis.

| Package | Used by | Pad X | Pad Y | Center ±X | Notes |
|---|---|---:|---:|---:|---|
| 0402 | small C (boot/FF/sense) | 0.59 | 0.64 | 0.51 | |
| 0603 | most R, several C, LED | 0.90 | 0.95 | 0.80 | |
| 0805 | C_MCU1, C_OUT1/2 | 1.00 | 1.45 | 0.95 | |
| 1206 | C_IN2 4.7 µF/50 V | 1.15 | 1.80 | 1.48 | |
| 1210 | C_IN1 10 µF/50 V | 1.15 | 2.65 | 1.48 | |
| FUSENANO2 | F1 | 2.60 | 2.40 | 2.60 | Littelfuse NANO2 land |
| CAP_V | C_IN_BULK, C_VM_BULK (Ø8.3 alu can) | 3.764 | 2.115 | 3.426 | **CAPAE830X1050N** (Fusion managed-library IPC pattern) — replaced the failing pre-rework land; keep an 8.3 mm clear disc, nothing under the can base |
| CAP_D | C_MCU_BULK (Kemet T520D, 7343 D-case) | 2.10 | 2.50 | 2.90 | molded chip, not a can |
| IND43 | L1 shielded inductor | 1.60 | 3.40 | 1.50 | |
| SOD-323 | D_MCU, D_CLW/D_CLC | 0.80 | 0.90 | 1.35 | |
| SOD-123 | D_Q1 | 0.95 | 1.20 | 1.85 | |
| SMA | D_OR_USB | 1.50 | 1.65 | 2.30 | |
| SMB | TVS1 | 2.00 | 2.20 | 2.20 | |
| SOT-23 | Q1 | 0.90 | 1.00 | ±0.95 | 3 pads |
| SOT-23-6 | U_BUCK | 0.60 | 1.00 | ±0.95 | 6 pads, 0.95 pitch |

## 3. Through-hole

| Package | Used by | Drill | Pad Ø | Notes |
|---|---|---:|---:|---|
| SCREW2 / SCREW3 | J_PWR / J_MOTOR (KF301 5.0 mm) | 1.3 | 2.2 | housing drawn true size (P×5.0 body, 7.6 deep) |
| SCREW5 | J_SENSOR (KF350 3.5 mm ×5) | 1.3 | 2.2 | see vendor table |
| XH2 / XH3 | J_BTN / J_KNOB, J_LED | 0.95 | 1.7 | JST-XH, housings drawn true size |
| HDR6 / HDR10 | J_PROG (DNP) / J_EXP | 0.9 | 1.6 | 1×6 and 1×10, 2.54 mm straight rows |
| USB-C shell | J_USB S1–S4 | 0.6 ×9 gang | 1.0 long | plated slots — see below |

## 4. Slot handling (fab-critical)

EAGLE/Fusion has no native slotted-drill object. Each USB-C shell slot is triple-
represented: long copper isle pad + layer-46 milling outline + overlapping-drill gang
(9 hits at 0.1375 mm pitch → 0.6×1.7 mm slot). Fusion CAM carries the gang into the
Excellon automatically (verified in the v4.3 export: 4 clusters × 9 hits). Add to every
PCBWay order: *"Overlapping drills at the USB-C shell legs are plated slots
(4× 0.6×1.7 mm) — follow slots per the Milling layer, do not drill round holes."*
`inject_slots.py` can additionally rewrite the drill file with true G85 slot records.

## Verifying in Fusion

After import, select any pad → the Inspect panel shows size/position, or use the
**Measure** tool between pads to confirm pitch against the tables above.
