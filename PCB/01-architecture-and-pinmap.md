# 01 — Architecture & Pin Map

## 1. System block diagram

```
                              24 V DC (Mean Well LRS-75-24, mains-derived)
                                        │
              ┌─────────────────────────┼───────────────────────────────┐
              │                         │                                │
        [F1 fuse]                       │                                │
              │                         │                                │
   [Q1 P-FET reverse-protect]           │                                │
              │                         │                                │
   [TVS SMBJ30A] [Cbulk 47–100 µF]      │                                │
              │                         │                                │
        24 V (protected)                │                                │
        ├──────────────► DRV8313 VM ────┴──► OUT1/2/3 ─► 3-phase motor    │
        │                  ▲   ▲ ▲ ▲                                      │
        │            IN1/2/3 nSLEEP/EN/nFAULT                             │
        │                  │   │ │ │                                      │
        ├──────────────► LED strip V+ (common anode) ──┐                 │
        │                                              │                 │
        │                       ┌──[AO3400 WW]── strip W cathode         │
        │                       └──[AO3400 CW]── strip C cathode         │
        │                            ▲   ▲                               │
        │                         gate gate (3.3 V LEDC PWM)             │
        │                                                                │
        └──► LMR51430 buck ──► 3.3 V ──[470 µF + ceramics]──► ESP32-S3-WROOM-1
                                          │                      │ │ │ │
                                          │         USB D±(19/20)┘ │ │ │
                                          │      MT6701 A/B (sat.)──┘ │ │
                                          │      knob A/B/SW ─────────┘ │
                                          │      DRV8313 ctrl ──────────┘
                                          │
                              USB-C ─[USBLC6 ESD]─[5.1k×2 CC]─ VBUS ─[SS14 diode-OR]─┐
                                                                                     │
                                            (USB 5 V only powers logic when no 24 V) ┘
```

The 24 V rail fans out to three loads: the motor driver (VM), the LED strip
(common anode), and the logic buck. USB-C is a **dev/flash port only**; its VBUS is
diode-OR'd so plugging in USB while the lamp is on mains can't back-feed (the exact
lesson the project already learned with the SS14 on the XIAO 5 V path).

## 2. Encoder: existing breakout, plugged in (no custom sensor board)

The MT6701 stays as the **existing breakout module** already in the lamp — already in
**ABZ mode at 1024 CPR**. It lives at the motor shaft and plugs into the main board's
`J_SENSOR` connector with a short **5-wire cable** (`VDD, GND, A, B, Z` — Z wired through
to a spare GPIO just in case, unused by current firmware).
**Nothing to place or program on the custom board** — just the mating connector. This is
deliberate: the sensor die must sit centered on the shaft axis at a sub-mm air gap, which
the main board can't meet anyway, and the breakout already solves that mechanically.

> A fully-custom bare-chip MT6701 satellite is documented as **optional** Block E′ in
> [`02-schematic-blocks.md`](02-schematic-blocks.md) — only relevant if you ever want to
> drop the third-party module (and it carries a one-time 5 V EEPROM-programming step to
> set 1024 PPR). Not part of the baseline design.

## 3. Power tree

```
24 V ─► F1 (1.5–2 A) ─► Q1 P-FET ideal-diode (reverse-polarity) ─► TVS SMBJ30A ─► protected 24 V
  protected 24 V ─► DRV8313 VM            (motor, up to ~0.5 A limited)
  protected 24 V ─► LED strip V+          (two channels, low-side switched)
  protected 24 V ─► LMR51430 buck ─► 3.3 V @ up to 3 A ─► ESP32-S3 + all logic + sensor VDD
```

- **No 5 V board rail.** The only historical 5 V consumer was the XIAO's LDO; the
  buck now makes 3.3 V directly.
- **3.3 V budget:** ESP32-S3 ~500 mA peak (WiFi/BLE TX bursts) + MT6701 ~20 mA +
  DRV8313 logic + pull-ups. The 3 A buck has ~6× headroom so it never current-limits
  mid-commissioning.
- **The 470 µF bulk cap goes next to the *module*, not the buck** — it supplies the
  TX burst locally before the buck loop responds. This is what cured the Matter
  commissioning brownouts.

## 4. ESP32-S3 GPIO assignment (proposed `BRUSHLESSLAMP_BOARD_CUSTOM`)

On a custom board the ESP32-S3 GPIO matrix lets MCPWM / PCNT / LEDC reach almost any
pin, so we can pick a **clean** map that avoids every strapping and reserved pin for
critical signals. Two upgrades over the XIAO build come for free:

- **Software DRV8313 `EN`** (its own GPIO instead of the hard 3V3 jumper).
- **`nFAULT` read-back** (wired to a GPIO — the XIAO build left it unconnected).

Reserved / avoid on WROOM-1-N8R8: `0` (BOOT), `3/45/46` (strapping), `19/20` (USB),
`35/36/37` (octal PSRAM), `43/44` (UART0 — usable but boot flicker), `26–32`
(internal flash, not even broken out).

| Firmware signal | Proposed GPIO | Peripheral | Notes |
|---|---:|---|---|
| `PIN_PWM_A`  (DRV IN1) | 4  | MCPWM | phase A |
| `PIN_PWM_B`  (DRV IN2) | 5  | MCPWM | phase B (swap A/B in firmware to set rotor direction, as today) |
| `PIN_PWM_C`  (DRV IN3) | 6  | MCPWM | phase C |
| `PIN_NSP`    (nSLEEP)  | 7  | GPIO out | pulse low ≥30 µs to clear OCP/TSD; external 10 k pull-up |
| `PIN_DRV_EN` (EN1/2/3) | 15 | GPIO out | **now software-controlled**; external pull-up + 0 Ω-to-3V3 option |
| `PIN_DRV_NFAULT`       | 16 | GPIO in  | **new** — open-drain, 10 k pull-up, read active-low |
| `PIN_ENC_A`  (MT6701 A)| 17 | PCNT | quadrature, from breakout |
| `PIN_ENC_B`  (MT6701 B)| 18 | PCNT | quadrature, from breakout |
| `PIN_ENC_Z`  (MT6701 Z)| 14 | GPIO in | index — wired "just in case", unused by current firmware |
| `PIN_ROT_A`  (knob A)  | 8  | PCNT | hardware glitch-filtered |
| `PIN_ROT_B`  (knob B)  | 9  | PCNT | |
| `PIN_BTN`    (knob SW) | 10 | GPIO in | `INPUT_PULLUP`, polled |
| `PIN_LED_WW` (WW gate) | 11 | LEDC | 25 kHz / 8-bit |
| `PIN_LED_CW` (CW gate) | 12 | LEDC | 25 kHz / 8-bit |
| `PIN_STATUS_LED` (opt) | 21 | GPIO out | optional board status LED |
| USB_D−                 | 19 | USB-Serial-JTAG | fixed by silicon |
| USB_D+                 | 20 | USB-Serial-JTAG | fixed by silicon |
| BOOT button            | 0  | strapping | 10 k pull-up + button |
| RESET button           | EN | — | 10 k pull-up + 1 µF RC + button |
| `J_EXP` spare GPIOs     | 1, 2, 13, 38, 39, 40, 41, 42 | GPIO | broken out to a 1×10 2.54 mm header (+3V3, GND) — see below |

### Spare-GPIO expansion header (`J_EXP`)

A 1×10 0.1″ (2.54 mm) pin header breaks out **8 unused GPIOs + 3V3 + GND** for future
use. The chosen pins are deliberately the *safe* spares — **no strapping pins** (IO0/3/45/46),
**not the octal-PSRAM pins** (IO35/36/37, reserved by the N8R8 module), and not the USB
pins (IO19/20):

| Header pin | GPIO | Note |
|---:|---:|---|
| 1 | 3V3 | board 3.3 V (fused by the buck; keep external draw modest) |
| 2 | GND | |
| 3 | IO1 | ADC1_CH0 |
| 4 | IO2 | ADC1_CH1 |
| 5 | IO13 | general |
| 6 | IO38 | general |
| 7–10 | IO39, IO40, IO41, IO42 | general — also the JTAG pins (MTCK/MTDO/MTDI/MTMS), so this header doubles as an optional external-JTAG port |

Unused pins can float (they're inputs). Populate the header only if you want it — or DNP it
and leave bare pads. If you ever need more, IO47/IO48 are also free.

> **GPIO numbers are a layout convenience, not a constraint.** Finalize them during
> routing to minimize trace crossings, then mirror the result into a new
> `#ifdef BRUSHLESSLAMP_BOARD_CUSTOM` branch in `s3/common/pins.h` (the file already
> has `XIAO` and `TEYLETEN` branches — add a third). Keep MT6701 A/B and knob A/B on
> PCNT-reachable pins (any GPIO works via the matrix) and keep the three PWM pins
> together for tidy routing to the DRV8313.

### Proposed `pins.h` branch (drop-in)

```c
#elif defined(BRUSHLESSLAMP_BOARD_CUSTOM)   // custom ESP32-S3-WROOM-1 board

constexpr uint8_t PIN_PWM_A        = 4;   // DRV8313 IN1
constexpr uint8_t PIN_PWM_B        = 5;   // DRV8313 IN2  (A/B swapped in code to set direction)
constexpr uint8_t PIN_PWM_C        = 6;   // DRV8313 IN3
constexpr uint8_t PIN_NSP          = 7;   // DRV8313 nSLEEP
constexpr int     PIN_DRV_EN       = 15;  // DRV8313 EN1/2/3 (software-controlled)
constexpr int     PIN_DRV_NFAULT   = 16;  // DRV8313 nFAULT (read-only, active-low)  [NEW]
constexpr uint8_t PIN_ENC_A        = 17;  // MT6701 A (breakout)
constexpr uint8_t PIN_ENC_B        = 18;  // MT6701 B (breakout)
constexpr uint8_t PIN_ENC_Z        = 14;  // MT6701 Z index (wired but unused by current firmware)
constexpr uint8_t PIN_ROT_A        = 8;   // knob A
constexpr uint8_t PIN_ROT_B        = 9;   // knob B
constexpr uint8_t PIN_BTN          = 10;  // knob push-button
constexpr uint8_t PIN_LED_WW       = 11;  // warm-white LEDC
constexpr uint8_t PIN_LED_CW       = 12;  // cool-white LEDC
constexpr uint8_t PIN_STATUS_LED   = 21;  // optional board status LED
```

`PIN_DRV_EN` becomes a real output (no longer the `-1` sentinel) — gives the firmware
true tri-state, and `PIN_DRV_NFAULT` lets `resetDriver()` confirm the fault actually
cleared instead of blind-pulsing nSLEEP. Both are optional firmware enhancements;
the board should still route EN with a 0 Ω/jumper option to 3V3 so an
unmodified-firmware bring-up works with EN hard-enabled.

## 5. Board-edge connectors (suggested)

**The board is all-SMD except the off-board connectors, which are through-hole** (pin
headers for the UI/LED signals, screw terminals for power/motor/encoder).

| Connector | Type | Pins | Goes to |
|---|---|---|---|
| J_PWR (24 V in) | screw terminal | 2 (V+, V−) | Mean Well LRS-75-24 |
| J_MOTOR | screw terminal | 3 (U, V, W) | BLDC phases |
| J_SENSOR (encoder) | screw terminal | 5 — **VCC, GND, A, B, Z** (confirmed) | MT6701 module; Z → spare GPIO14 |
| J_KNOB | JST-XH (2.54 mm) | 3 (ROT_A, ROT_B, GND) | rotary side of the knob |
| J_BTN | JST-XH (2.54 mm) | 2 (BTN, GND) | push-button side of the knob |
| J_LED | JST-XH (2.54 mm) | 3 (V+, W, C) | CCT strip |
| J_EXP | 1×10 male pin header (2.54 mm) | 10 (3V3, GND, 8 spare GPIO) | future expansion / optional JTAG |
| J_USB | SMD USB-C | — | dev/flash |

> **All-SMD board, through-hole connectors.** Every active/passive — including the AO3400A
> LED switches (SOT-23, replacing the breadboard's through-hole MOSFETs) — is surface-mount
> for a single SMT reflow. The six off-board connectors are deliberately **through-hole**
> for mechanical strength: **keyed JST-XH** (button/knob/LED) and screw terminals
> (power/motor/encoder). They're either added to JLCPCB's THT assembly or hand-soldered
> after reflow.

### Confirmed target motor + encoder (the unit on hand)

Amazon **B0G2LR85GW**, "4015 BLDC Hollow-Shaft External-Rotor, 12/24/36 V — Motor+MT6701":

- **Motor:** 4015, **24-slot / 22-pole = 11 pole pairs** (matches `POLE_PAIRS = 11`),
  hollow shaft, external rotor, 12–36 V (run at 24 V). 3 phase wires → `J_MOTOR`.
  *(Listing KV ~61 is auto-generated and inconsistent — KV/phase-R are firmware tuning
  values, not PCB params; keep your working `KV_RATING`/`PHASE_RESISTANCE`.)*
- **Encoder:** bundled **MT6701 module, ABZ mode / 1024 lines by default** — no
  programming, no 5 V burn. Pads **VCC (3.3–5 V) · GND · A · B · Z**; Z is the
  once-per-rev index/zero pulse. It only leaves ABZ if the `IIC_NC` pads are shorted —
  **leave them open.** We feed it **3.3 V** from the board; its A/B/Z are 3.3 V logic,
  directly ESP32-compatible. The module is **pre-mounted on the motor at the correct air
  gap** over the shaft magnet by the vendor — so `J_SENSOR` really is just plug-in.
- **`J_SENSOR` pin order — CONFIRMED on the physical module: `VCC · GND · A · B · Z`**
  (in that order). Wire the 5-pin connector to match this 1:1.

Keep `J_PWR`, `J_MOTOR`, `J_LED` (the noisy 24 V/high-current side) clustered on one
edge and `J_KNOB`, `J_SENSOR`, `J_USB` (quiet logic) on another, with the RF end of
the WROOM-1 pointed off the opposite board edge with ground clearance.
