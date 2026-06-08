# 02 — Schematic Blocks

Schematic-level design of every block. Values are from the Espressif ESP32-S3
hardware design guidelines, the TI DRV8313 datasheet (SLVSBA5D) + the open-source
SimpleFOC Mini v1.1 schematic, the TI LMR51430 datasheet, the ST USBLC6-2 datasheet,
and the MagnTek MT6701 datasheet (sources at the bottom). Ref-des prefixes are
namespaced per block to avoid collisions.

---

## Block A — MCU core (ESP32-S3-WROOM-1-N8R8)

**Module:** `ESP32-S3-WROOM-1-N8R8` (exact XIAO ESP32-S3R8 silicon: 8 MB flash + 8 MB
octal PSRAM). Use `-1U` only if you want an external IPEX antenna.

| Ref | Value / Part | Purpose |
|---|---|---|
| U_MCU | ESP32-S3-WROOM-1-N8R8 | MCU module |
| C_MCU1 | 10 µF X7R 0805 | 3V3 bulk at module power entrance |
| C_MCU2 | 1 µF X7R 0603 | mid-frequency decoupling |
| C_MCU3, C_MCU4 | 0.1 µF X7R 0402 | per-pin HF decoupling at each 3V3 pin |
| C_MCU_BULK | **470 µF** low-ESR polymer, 6.3 V | TX-burst reservoir, **at the module**, <5 mm leads |
| D_MCU | TVS/ESD diode, 3.3 V working (SOD-323) | power-entrance ESD |
| R_EN | 10 kΩ | EN pull-up to 3V3 |
| C_EN | 1 µF X7R 0603 | EN RC delay / reset debounce → GND |
| SW_RST | tactile SPST | RESET (EN → GND) |
| R_BOOT | 10 kΩ | GPIO0 pull-up to 3V3 |
| SW_BOOT | tactile SPST | BOOT (GPIO0 → GND) |
| C_BOOT | **100 pF** (or DNP) | GPIO0 debounce — **must stay small**; a ~0.1 µF cap here slows the GPIO0 rise and risks strapping into download mode (Espressif "no large cap on GPIO0"; XIAO uses 10 pF). Do **not** balance it against C_EN. |

**Nets**
```
3V3 ─► U_MCU VDD pins; C_MCU1(10µF)+C_MCU2(1µF) at entrance; C_MCU3/4(0.1µF) at each VDD pin
       C_MCU_BULK(470µF) right at the module 3V3/GND pads; D_MCU ESD at entrance
GND ─► U_MCU GND pins + module thermal pad (via-stitched to ground plane)
EN  ◄─ R_EN 10k to 3V3 ; C_EN 1µF EN→GND ; SW_RST EN→GND
GPIO0 ◄─ R_BOOT 10k to 3V3 ; SW_BOOT GPIO0→GND ; C_BOOT 100pF (small only — never ~0.1µF)
GPIO45 / GPIO46 ─► leave at internal pull-down — NEVER pull high at boot
GPIO3 ─► leave at default (no internal pull); only use as benign-at-reset I/O
GPIO35 / GPIO36 / GPIO37 ─► NO CONNECT (octal PSRAM, reserved even with PSRAM disabled)
GPIO19 (USB_D−), GPIO20 (USB_D+) ─► to USB block
peripheral GPIOs ─► per 01-architecture pin map
```

**Strapping at boot:** normal SPI boot = GPIO0:1, GPIO46:0; download = GPIO0:0,
GPIO46:0. GPIO45/46 must stay low at power-up. Do not put a large cap on GPIO0.

---

## Block B — USB-C programming/flashing + native USB

**This is the primary way to program, flash, and monitor the board — over USB-C.** The
ESP32-S3 has a built-in **USB-Serial-JTAG** controller on GPIO19 (D−) / GPIO20 (D+), so a
single USB-C cable gives you **firmware flash + serial console + JTAG debug** with no
external bridge chip — exactly like the XIAO. The chip also has the **internal 1.5 kΩ D+
pull-up** (do not add an external one).

**How flashing works (no buttons needed in normal use):**
- `idf.py -p <PORT> flash monitor` resets the chip into download mode over USB and
  programs it. The board enumerates as a USB CDC device (`/dev/cu.usbmodem*` on macOS).
- **BOOT + RESET buttons are a manual recovery path** only — if firmware ever wedges the
  USB stack: hold BOOT (GPIO0→GND), tap RESET (EN→GND), release BOOT, then flash. Use
  esptool `--after watchdog-reset` so the chip re-samples strapping and leaves download
  mode cleanly afterward.
- **No UART bridge (CP2102/CH340) and no DTR/RTS auto-reset transistors** — native USB has
  no modem-control pins to drive; the reset is commanded in-band over USB. (BOOT+RESET
  buttons live in Block A.)

| Ref | Value / Part | Purpose |
|---|---|---|
| J_USB | USB-C receptacle, 16-pin USB 2.0 (GCT USB4085 / TYPE-C-31-M-12) | flash/console/JTAG connector |
| R_CC1 | 5.1 kΩ 1% | CC1 pull-down (Rd, sink/UFP) |
| R_CC2 | 5.1 kΩ 1% | CC2 pull-down (Rd) — **separate** from R_CC1 |
| R_DP | 22 Ω | series on D+ (GPIO20) |
| R_DM | 22 Ω | series on D− (GPIO19) |
| D_USB | USBLC6-2SC6 (SOT23-6L) | ESD array on D+/D−/VBUS |
| C_VBUS | 4.7–10 µF | VBUS bulk near J_USB |
| D_OR_USB | SS14 Schottky | VBUS → common logic node (diode-OR) |
| D_OR_5V  | SS14 Schottky | buck-3.3V-feed → common node (diode-OR) — see note |
| C_DP/C_DM | DNP 10–47 pF | optional D± EMI caps (reserve, unpopulated) |
| J_PROG | 6× **SMD test pads** (or SMD 1.27 mm header) (**fallback**) | 3V3, GND, EN, GPIO0, TXD0(43), RXD0(44) — UART recovery if USB ever fails. Bare pads = no part placed |

**Nets**
```
J_USB.VBUS (A4,B4,A9,B9 tied) ─┬─ C_VBUS ─ GND
                               ├─ D_USB.5 (USBLC6 Vcc)
                               └─ D_OR_USB(SS14) anode ─► OR node
J_USB.GND (A1,B1,A12,B12,shield) ─ GND
J_USB.CC1 (A5) ─ R_CC1 5.1k ─ GND
J_USB.CC2 (B5) ─ R_CC2 5.1k ─ GND
J_USB.D+ (A6+B6 shorted) ─ D_USB ── R_DP 22Ω ─ GPIO20
J_USB.D− (A7+B7 shorted) ─ D_USB ── R_DM 22Ω ─ GPIO19

J_PROG: 3V3 · GND · EN · GPIO0 · GPIO43(U0TXD) · GPIO44(U0RXD)   [fallback only]
```
Place D_USB **right at the connector**, order: connector → USBLC6 → 22 Ω → MCU.

> **Fallback programming header (`J_PROG`).** USB-C is the intended path and is enough on
> its own. But a tiny 6-pin header/pad field exposing `3V3 / GND / EN / GPIO0 / U0TXD
> (GPIO43) / U0RXD (GPIO44)` is cheap insurance: it lets an external USB-UART adapter
> (with its own DTR/RTS auto-reset) flash the bootloader even if the native-USB stack is
> bricked. GPIO43/44 are UART0 TX/RX and aren't used by any peripheral in the pin map.
> Mark it DNP-friendly (unloaded pads) — populate only if you need it.
Route D+/D− as a 90 Ω differential pair, short, over solid ground.

> **VBUS / mains coexistence.** USB is dev-only; the board is normally on 24 V. The
> 24 V buck makes the 3.3 V rail. USB VBUS must **never** tie directly to that rail.
> Simplest: power the board off the buck's 3.3 V always, and only let USB VBUS feed a
> tiny separate path for flashing if you want to bring the logic up without 24 V — in
> that case OR VBUS (via D_OR_USB) and a 5 V-derived feed into the buck input, or
> better, just always bench-test with the 24 V supply attached. If you do want
> "USB-only" bring-up, the cleanest production answer is a power-path IC (TI TPS2116
> priority MUX or LM66100 ideal diode) selecting 24 V-buck-3.3V over a USB-VBUS-LDO.
> For a one-off, dual-SS14 OR into the buck-feed node (mirroring the existing SS14
> lesson) is fine. **Keep VBUS routed only to the USBLC6 and the OR diode — nowhere
> else.**
>
> ⚠️ **Review note (USB-only UVLO margin).** As wired, `D_OR_USB` (SS14) feeds USB 5 V into
> the buck input `P24`. After the Schottky drop (~0.35 V at light load) the buck sees
> ~4.6 V, and the LMR51430's UVLO-rising is up to **4.5 V** — only ~0.1 V of margin. It'll
> run for **light loads** (MCU-only flashing/console), but USB-only is **best-effort**: if
> it's flaky, either (a) just bench-test with 24 V attached (USB then data-only, diode
> reverse-blocked — the normal case), or (b) swap `D_OR_USB` for a **P-FET ideal-diode OR**
> (near-zero drop → ~5 V at the buck, comfortable). Not a board bug — only affects the
> optional "power from USB without 24 V" convenience.

---

## Block C — Power: 24 V → 3.3 V (LMR51430)

Single-stage synchronous buck, 4.5–36 V in, 3 A, SOT-23-6. FB Vref = 0.6 V.
`RFBT/RFBB = (3.3/0.6) − 1 = 4.5` → RFBB = 10 kΩ, RFBT = 45.3 kΩ (E96) → 3.318 V.

### Input protection (24 V)
| Ref | Value / Part | Purpose |
|---|---|---|
| F1 | 1.5–2 A slow-blow **SMD chip fuse, 1206** (≥63 V DC rating) | fault/fire protection |
| Q1 | P-MOSFET ideal-diode, Vds ≥ 40 V, **SOT-23** (DMP3017SFG / AO3401A; DPAK if more current) | reverse-polarity, ~mV drop |
| R_Q1 | 100 kΩ | Q1 gate pulldown |
| D_Q1 | 12–15 V Zener gate→source (SOD-123) | clamp Vgs in reverse fault |
| TVS1 | SMBJ30A (SMB, 30 V standoff, ~48 V clamp, 600 W) | 24 V transient clamp (below buck 40 V abs-max) |
| C_IN_BULK | 47–100 µF / 50 V **SMD** Al-electrolytic (V-chip) or Al-polymer | surge ride-through / input bulk |

> P-FET ideal-diode over a series Schottky: at ~150–300 mA total logic draw the
> Schottky's ~0.4 V drop is wasted heat; the P-FET costs a few mV. Place F1 + Q1 + TVS
> upstream of **all** 24 V loads (motor, LED, buck) so the whole board is protected;
> the DRV8313's own VM bulk stays separate.

### Buck (LMR51430)
| Ref | Value / Part | Purpose |
|---|---|---|
| U_BUCK | LMR51430YDDCR (LCSC C5210749) | 24 V→3.3 V buck |
| C_IN1 | 10 µF / 50 V X7R 1210 | local HF input decoupling (derated 50 V) |
| C_IN2 | 4.7 µF / 50 V X7R 1206 | splits input ripple, tightens hot-loop |
| L1 | 4.7 µH, Isat ≥ 1.5 A, shielded (Würth 74438335047) | buck inductor |
| C_OUT1, C_OUT2 | 22 µF / 10 V X7R | output caps (low ESR) |
| R_FBT | 45.3 kΩ 1% 0402 | FB top (sets 3.318 V) |
| R_FBB | 10 kΩ 1% 0402 | FB bottom |
| C_BOOT_B | 100 nF / 16 V X7R (BOOT↔SW) | high-side gate drive |
| C_FF | 22 pF (optional, across R_FBT) | feed-forward, transient/phase margin |
| R_ENB | 100 kΩ (EN→VIN) | enable (do not float; EN is VIN-tolerant) |

**Nets**
```
protected-24V ─ C_IN1 + C_IN2 + C_IN_BULK ─ U_BUCK.VIN ; R_ENB VIN→EN
U_BUCK.SW ─ L1 ─► 3V3 ;  C_BOOT_B BOOT↔SW
3V3 ─ C_OUT1 + C_OUT2 ─ GND ;  R_FBT 3V3→FB ; R_FBB FB→GND ; (C_FF across R_FBT)
3V3 ─► Block A (MCU) + MT6701 satellite VDD + all logic pull-ups
```
~1.1 MHz switching with these L/C. Keep the input hot-loop (C_IN→VIN/GND) tight and
the SW node small. The 470 µF MCU bulk (Block A) lives at the module, not here.

---

## Block D — DRV8313 3-phase driver (replicates SimpleFOC Mini v1.1)

`DRV8313PWPR`, HTSSOP-28 PowerPAD (LCSC C92482). Voltage-mode, **no current sense**:
PGNDx tie straight to GND, comparator unused. EN1/2/3 tied to one EN node; nSLEEP +
nFAULT (+ optional EN) to the MCU. All INx/ENx/nSLEEP/nRESET have internal 100 kΩ
pulldowns, so pull-ups are what keep the part awake/enabled.

| Ref | Value / Part | Purpose |
|---|---|---|
| U_DRV | DRV8313PWPR (HTSSOP-28) | 3-phase driver |
| C_CP | 0.01 µF, VM-rated (≥50 V) | charge-pump flying cap CPL↔CPH *(Mini uses 0.1 µF — works; 0.01 µF = datasheet-exact)* |
| C_VCP | 0.1 µF / 16 V (≥50 V ok) | VCP→VM charge-pump reservoir |
| C_V3P3 | 0.47 µF / 6.3 V | V3P3 internal-LDO bypass → GND (required even if unused) |
| C_VM1, C_VM2 | 0.1 µF / 50 V | one at each VM pin (4, 11) |
| C_VM_BULK | 100 µF / **50 V** | VM bulk (Mini ships 35 V — bump to 50 V for 24 V margin) |
| R_NSLEEP | 10 kΩ → 3V3 | nSLEEP pull-up (defaults awake) |
| R_NRESET | 10 kΩ → 3V3 | nRESET pull-up (held high, normal run) |
| R_NFAULT | 10 kΩ → 3V3 | nFAULT open-drain pull-up |
| R_EN_DRV | 10 kΩ → 3V3 + 0 Ω jumper option | EN default-enabled; jumper lets you hard-tie EN=3V3 for unmodified firmware |

**Nets** (pin numbers = HTSSOP-28 PWP)
```
VM (4,11) ── protected-24V ── C_VM_BULK 100µF/50V → GND ; C_VM1(pin4) + C_VM2(pin11) 0.1µF → GND
VCP (3) ── C_VCP 0.1µF ── VM
CPH (2) ─┐
         ├── C_CP 0.01µF (VM-rated)   [flying cap — keep loop tight]
CPL (1) ─┘
V3P3 (15) ── C_V3P3 0.47µF → GND      [internal 3.3 V/10 mA LDO; pull-up reference]

IN1 (27) ── GPIO PIN_PWM_A    EN1 (26) ─┐
IN2 (25) ── GPIO PIN_PWM_B    EN2 (24) ─┼── EN node ── R_EN_DRV 10k→3V3 (+0Ω-to-3V3 jumper) ── GPIO PIN_DRV_EN
IN3 (23) ── GPIO PIN_PWM_C    EN3 (22) ─┘

nSLEEP (17) ── R_NSLEEP 10k→3V3 ── GPIO PIN_NSP        (pulse low ≥30 µs to clear OCP/TSD)
nRESET (16) ── R_NRESET 10k→3V3                         (held high)
nFAULT (18) ── R_NFAULT 10k→3V3 ── GPIO PIN_DRV_NFAULT  (open-drain, active-low read)

OUT1 (5) ── motor phase U     OUT2 (8) ── phase V     OUT3 (9) ── phase W
PGND1 (6), PGND2 (7), PGND3 (10) ── GND      (no current sense)
COMPP (12), COMPN (13) ── GND                (comparator unused)
nCOMPO (19) ── GND or NC
GND (14,20,28) ── GND ;  NC (21) ── open
EP/PPAD ── GND  (copper pour + thermal-via array)
```
**Layout:** bulk cap to minimize the VM→OUTx→PGND→GND loop; OUT1/2/3 on wide copper to
the motor connector; charge-pump caps hard against pins 1/2/3; V3P3 cap at pin 15;
exposed pad to ground pour with a 4–9 via array.

---

## Block E — MT6701 magnetic encoder (existing breakout, plugged in)

**Primary plan: reuse the existing MT6701 breakout** — the module bundled with the motor
(Amazon B0G2LR85GW), which **ships in ABZ mode at 1024 lines by default** and is
**pre-mounted on the motor at the correct air gap**. Pads are **VCC (3.3–5 V) · GND · A ·
B · Z**; leave the module's `IIC_NC` pads **open** to keep it in ABZ. It stays at the
motor shaft and plugs into the main board through a connector. There is **no chip to place, no
magnet to spec, and nothing to program** on the custom board — the main board only needs
the mating connector below. This is the recommended path: identical to today, just
landing on your own board instead of the XIAO.

**Main-board side (all you need):**
| Ref | Value / Part | Purpose |
|---|---|---|
| J_SENSOR | **5-pos screw terminal (THT)** | wires from the existing MT6701 module |
| C_SENS | 0.1 µF 0402 (optional) | extra decoupling at the connector for the cable run |
| R_AZ (×3) | 10 kΩ (DNP) | optional A/B/Z pull-up stiffening for a long/noisy cable |

**Nets (connector only)**
```
J_SENSOR.VDD ── 3V3 ── C_SENS 0.1µF → GND        (powers the breakout)
J_SENSOR.GND ── GND
J_SENSOR.A   ── PIN_ENC_A    (breakout's A / "DO" pad; internal 200 kΩ pull-up on the MT6701)
J_SENSOR.B   ── PIN_ENC_B    (breakout's B / "CLK" pad)
J_SENSOR.Z   ── PIN_ENC_Z    (breakout's Z / "CS" pad — routed to a spare GPIO; unused by
                              current firmware but available for a future index/abs-zero)
```
Z is wired through to a free GPIO **just in case** — the firmware ignores it today, but
single-turn absolute indexing or an encoder zero-reference becomes a firmware-only change
later, no board respin. **`J_SENSOR` pad order is confirmed: `VCC · GND · A · B · Z`** —
wire the 5-pin connector to match 1:1. Keep the cable short and route A/B as a quasi-pair
with a GND return. The 50 ms ABZ power-up blanking is handled in firmware, not hardware.

---

### Optional Block E′ — custom bare-chip MT6701 satellite (only if you ever drop the breakout)

Not needed for your plan. Documented only so the option exists if you later want to
eliminate the third-party module and integrate the sensor yourself. **This is the path
where the resolution gotcha applies:** a bare `MT6701QT-STD` ships at a default ABZ
resolution that is **not** 1024 PPR; setting 1024 means writing the EEPROM `ABZ_RES`
register over I²C and **burning it at VDD > 4.5 V** (won't program at 3.3 V). To go this
route you'd source a pre-programmed part **or** add a one-time 5 V-during-burn provision
(test point/jumper to sensor VDD) with A=SDA/B=SCL brought to GPIO.

| Ref | Value / Part | Purpose |
|---|---|---|
| U_ENC | MT6701QT-STD (QFN-16, 3×3 — die centered in package) | magnetic angle sensor |
| C_ENC1 | 0.1 µF 0402 | VDD decoupling at the chip |
| C_ENC2 | 1 µF 0603 | bulk for the cable run |
| D_ENC | 6 V TVS (DNP-optional) | VDD reliability (datasheet-recommended) |
| R_AZ (×3) | 10 kΩ (DNP) | optional A/B/Z stiffening |
| J_ENC | JST-SH/GH 4–5 pin | cable to main board |

Wiring: VDD(13)→3V3 + caps; A(6)/B(7)/Z(8)→connector; MODE(14) internal 200 kΩ pull-up
selects ABZ; PUSH(5)/OUT(15)/UVW pins NC. **Magnet:** Ø6 mm × 2.5 mm diametric on the
hollow shaft, die centered ±0.3 mm, air gap 0.5–2.0 mm (~1 mm). Non-magnetic holder.

---

## Block F — LED low-side switches (2× AO3400A)

Common-anode 24 V CCT strip; each channel switched low-side, gate from a 3.3 V LEDC
GPIO at 25 kHz. AO3400A (SOT-23, logic-level, Vth 1.2–2.2 V, ~33 mΩ) fully enhances
from 3.3 V. **No flyback diode** (resistive + LED load, essentially non-inductive).

Per channel (×2 — warm-white, cool-white):
| Ref | Value / Part | Purpose |
|---|---|---|
| Q_WW / Q_CW | AO3400A (SOT-23, LCSC C20917) | low-side switch |
| R_GW / R_GC | 33 Ω (22–100 Ω) | gate series — tames edges/EMI at 25 kHz |
| R_PDW / R_PDC | 100 kΩ | gate pull-down — keeps FET off during boot/hi-Z |
| D_CLW / D_CLC | TVS/Schottky (DNP) | drain clamp — only if drain ringing >30 V seen |

**Nets**
```
LED strip V+ (common anode) ── protected-24V
strip W cathode ── Q_WW drain ;  Q_WW source ── GND ;  Q_WW gate ── R_GW 33Ω ── PIN_LED_WW ; R_PDW 100k gate→GND
strip C cathode ── Q_CW drain ;  Q_CW source ── GND ;  Q_CW gate ── R_GC 33Ω ── PIN_LED_CW ; R_PDC 100k gate→GND
```
Route the 24 V LED return on wide copper back to the supply ground; star-ground at the
power connector so motor + LED return current doesn't share the ESP32/RF reference.
**AO3400A is SOT-23 (SMD)** — these replace the through-hole MOSFETs from the breadboard
build.

---

## Block G — Connectors

**The board is all-SMD *except the off-board connectors*, which are through-hole** for
mechanical strength (cable tension would peel SMD pads). Two connector families:

- **Keyed JST-XH (2.54 mm pitch)** for the low-current UI/LED signals — polarized so a
  connector can't go in backwards.
- **Through-hole screw-terminal blocks** for the high-current / field-wired power, motor,
  and encoder lines.

| Connector | Type | Pins | Signal (order) |
|---|---|---|---|
| J_PWR | screw terminal (THT) | 2 | 24 V `V+`, `V−` |
| J_MOTOR | screw terminal (THT) | 3 | `U`, `V`, `W` |
| J_SENSOR (encoder) | screw terminal (THT) | 5 | `VCC`, `GND`, `A`, `B`, `Z` |
| J_KNOB | JST-XH 2.54 mm (THT) | 3 | `ROT_A`, `ROT_B`, `GND` (common) |
| J_BTN | JST-XH 2.54 mm (THT) | 2 | `BTN` (SW), `GND` |
| J_LED | JST-XH 2.54 mm (THT) | 3 | `V+`, `W`, `C` |
| J_EXP | 1×10 male pin header (2.54 mm THT) | 10 | spare-GPIO breakout: 3V3, GND, IO1, IO2, IO13, IO38–42 |
| J_USB | SMD USB-C receptacle (Block B) | — | dev/flash |
| J_PROG | SMD test pads (Block B) | 6 | UART fallback — no part placed |

The rotary encoder's two physical sides map to two JST-XH connectors: **J_KNOB** (the 3-pin
A/B/common rotary side) and **J_BTN** (the 2-pin push-button side) — wire the encoder's
common and the button's return both to `GND`. Pick the screw-terminal pitch (3.5 / 5.0 mm)
to suit your wire gauge; put `J_PWR` / `J_MOTOR` / `J_SENSOR` on the 24 V/high-current
board edge per the layout plan. JST-XH mating shells: B3B-XH-A (3-pin) / B2B-XH-A (2-pin).

> **Assembly note:** these THT connectors are the board's only through-hole parts — either
> add them to JLCPCB's through-hole assembly or hand-solder them after the SMT reflow
> (six connectors, a few minutes). Everything else is SMD, one reflow pass.

---

## Sources

- ESP32-S3-WROOM-1/1U datasheet & ESP32-S3 Hardware Design Guidelines (schematic
  checklist: decoupling 10/1/0.1 µF, EN RC 10 k+1 µF, GPIO0 "no large cap",
  USB 22 Ω + internal 1.5 k pull-up, strapping tables).
  <https://docs.espressif.com/projects/esp-hardware-design-guidelines/en/latest/esp32s3/schematic-checklist.html>
- TI DRV8313 datasheet SLVSBA5D — <https://www.ti.com/lit/ds/symlink/drv8313.pdf>
- SimpleFOC Mini v1.1 schematic + BOM —
  <https://github.com/simplefoc/SimpleFOCMini> ·
  <https://github.com/simplefoc/SimpleFOCMini/blob/main/Schematic_simplefocmini.pdf>
- TI LMR51430 — <https://www.ti.com/product/LMR51430> (verify FB eq + inductor table
  against the official PDF before tape-out; values here cross-checked from TI product
  page + Mouser-hosted datasheet).
- ST USBLC6-2 — <https://www.st.com/resource/en/datasheet/usblc6-2.pdf>
- MagnTek MT6701 datasheet Rev 1.8/1.9 — pinout, MODE 200 kΩ pull-up, ABZ_RES
  register, EEPROM burn VDD>4.5 V, magnet Ø6×2.5 mm / AG 0.5–2.0 mm / DISP 0.3 mm.
- AO3400A datasheet (AOS) — <https://www.aosmd.com/sites/default/files/res/datasheets/AO3400A.pdf>
- Seeed — Schottky to prevent USB/external power back-feed —
  <https://www.seeedstudio.com/blog/2025/11/24/designing-a-xiao-expansion-board-using-a-schottky-diode-to-prevent-power-backfeeding/>
