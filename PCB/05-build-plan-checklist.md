# 05 — Build Plan & Checklist

The order to actually do this in, with the risks to retire early.

## Phase 0 — Decisions to lock first (cheap to change now, expensive later)

- [ ] **Confirm WROOM-1-N8R8** (vs N8 non-PSRAM if you ever want GPIO35/36/37 back —
      not needed today, and N8R8 = exact XIAO silicon).
- [x] **MT6701: reuse the existing breakout**, plugged into `J_SENSOR` (already ABZ /
      1024 PPR — no programming). Just match the connector pinout/pitch to your breakout.
      *(The bare-chip satellite with its 5 V EEPROM burn is the optional path you're not
      taking.)*
- [ ] **EN strategy:** software `PIN_DRV_EN` (recommended) **plus** a 0 Ω/jumper to 3V3
      so unmodified firmware brings up with EN hard-enabled.
- [ ] **USB-only bring-up?** If you always bench-test with 24 V attached, the VBUS
      diode-OR is optional. If you want logic-up on USB alone, add the power-path
      (dual-SS14 or TPS2116/LM66100).
- [ ] **Connectors** finalized — THT: JST-XH (button/knob/LED) + screw terminals
      (power/motor/encoder) per [`01`](01-architecture-and-pinmap.md).

## Phase 1 — Library prep

- [ ] Pull Eagle `.lbr` for: WROOM-1-N8R8, DRV8313PWPR, LMR51430, USBLC6-2SC6, USB-C 16P,
      AO3400A (SnapEDA/UltraLibrarian/SamacSys).
- [ ] Author/verify the **MT6701QT-STD** symbol + QFN-16 footprint (likely custom).
- [ ] Cross-check every footprint against the datasheet (pad 1, thermal pad, pin pitch).
- [ ] **SMD for everything except the 7 connectors** — verify no stray THT part (buttons =
      SMD tactile; fuse = NANO2 SMD; bulk caps = SMD polymer/V-chip, not radial cans). The
      only THT footprints allowed: `J_PWR`/`J_MOTOR`/`J_SENSOR` (screw terminals) +
      `J_KNOB`/`J_BTN`/`J_LED` (JST-XH 2.50 mm) + `J_EXP` (2.54 mm pin header).

## Phase 2 — Schematic capture (Fusion Electronics)

- [x] **Draft schematic already generated** — [`fusion/BrushlessLamp.sch`](fusion/BrushlessLamp.sch)
      has all 71 parts + the full netlist wired (named nets). Upload it into a Fusion
      Electronics project as the starting point instead of capturing from scratch.
- [ ] In Fusion, **assign real symbols/footprints** to each part (REPLACE the generic
      no-package devices with library parts; net connections carry over by name). Big
      parts first: WROOM-1, DRV8313, LMR51430, USB-C, USBLC6.
- [ ] (Or capture block-by-block from [`02-schematic-blocks.md`](02-schematic-blocks.md)
      if you prefer a clean sheet.)
- [ ] Confirm the GPIO map from [`01-architecture-and-pinmap.md`](01-architecture-and-pinmap.md).
- [ ] **ERC clean.**
- [ ] Sanity-check against firmware: every `s3/common/pins.h` signal has a net; PWM A/B
      swap preserved; nSLEEP reachable; EN jumper present.

## Phase 3 — PCB layout (2-layer)

- [ ] 2-layer; **solid bottom ground pour**, via-stitched (WROOM-1 has its own RF ground).
- [ ] Place: WROOM-1 antenna off-edge, 24 V/motor/LED cluster on one edge, logic on the
      other.
- [ ] Route power loops (DRV8313 VM, buck hot-loop) tight; wide 24 V/phase copper;
      thermal vias on DRV8313 + buck pads.
- [ ] 470 µF at module; USB 90 Ω pair; charge-pump caps at pins.
- [ ] Load **[`PCBWay.dru`](PCBWay.dru)** (DRC dialog → Load) — PCBWay-safe 6 mil / 0.3 mm
      drill rules (already 2-layer). Run **DRC**.

## Phase 4 — Fab & assembly

- [ ] CAM: Gerbers + drill + BOM + centroid.
- [ ] Order main board (and a few sensor satellites) at JLCPCB; SMT assembly with LCSC
      parts; DNP the optional/clamp parts.
- [ ] Source the Ø6×2.5 mm diametric magnet + a non-magnetic shaft holder.

## Phase 5 — Bring-up (power up in stages — never all at once)

1. [ ] **Bare power, no MCU loaded yet (or hold in reset).** Apply 24 V through a current-
       limited bench supply. Verify reverse-protect (try reversed leads — fuse/FET should
       block), TVS not conducting, and the **buck makes 3.30–3.33 V**. Check ripple.
2. [ ] **MCU alive.** Confirm 3V3 stable, EN RC releases, BOOT/RESET buttons work. Plug
       USB-C → enumerate as USB-Serial-JTAG. Flash a blink/banner. Confirm GPIO0 not stuck
       (no large cap), `--after watchdog-reset` if it hangs in download.
3. [ ] **Flash the real firmware** with a new `BRUSHLESSLAMP_BOARD_CUSTOM` `pins.h` branch
       (snippet in [`01`](01-architecture-and-pinmap.md)). Motor/LED disconnected.
4. [ ] **Encoder.** Plug the existing MT6701 breakout into `J_SENSOR`; verify it reads
       **1024 PPR (4096 CPR in quadrature)** and A/B counts (same module as today — should just work).
5. [ ] **Driver + motor.** Connect motor. EN enabled (jumper or GPIO). Verify `initFOC`,
       direction sweep, smooth spin, nSLEEP fault-clear, and `nFAULT` read-back (new
       capability). Confirm audibility matches the module build (5 kHz loop, center-aligned
       PWM as documented).
6. [ ] **LEDs.** Connect strip; verify both AO3400 channels PWM at 25 kHz, WW/CW mix,
       gate pull-downs keep them dark through boot.
7. [ ] **Matter.** Provision per `s3/README.md §6`; commission; verify no brownout under
       RF burst (the 470 µF + solid buck should have killed it). Confirm 240 MHz, MTU,
       and the CHIP-shell-off config are intact.
8. [ ] **Soak / regression** against the M1–M4 stage acceptance table in `s3/README.md §5`.

## Risk register

| Risk | Where it bites | Mitigation |
|---|---|---|
| MT6701 resolution (only if going bare-chip) | encoder reads garbage at bring-up | N/A for baseline — reusing the pre-configured breakout; gotcha only applies to the optional custom satellite |
| `J_SENSOR` pinout mismatched to breakout | encoder won't plug in / miswired | RESOLVED — pad order confirmed `VCC·GND·A·B·Z`; wire the 5-pin connector to match 1:1 |
| LMR51430 FB/inductor values from secondary sources | wrong Vout / instability | verify against official TI PDF before tape-out (flagged in [`02`](02-schematic-blocks.md)) |
| RF performance on custom board | weak WiFi/BLE range | module + continuous GND plane + antenna keep-out; don't go chip-down |
| 24 V transient kills buck | dead board | SMBJ28A TVS + 50 V-rated input caps + 36 V buck |
| Back-feed when USB + 24 V both present | PC port / board damage | VBUS stays local; diode-OR or power-path |
| GPIO35/36/37 used by mistake | boot/PSRAM faults | marked NO-CONNECT in [`02`](02-schematic-blocks.md) |
| Strapping pins held wrong at boot | won't boot / stuck in download | GPIO0 pull-up only, GPIO45/46 low, no large cap on GPIO0 |

## Definition of done

A single custom board that boots from 24 V, enumerates over USB-C, runs the unmodified
esp-matter firmware on a `BRUSHLESSLAMP_BOARD_CUSTOM` pin branch, drives the gimbal
through closed-loop FOC as quietly as the XIAO+Mini build, fades the CCT LEDs, takes
knob input, and commissions into Matter without brownout — with the MT6701 on its own
satellite at the shaft.
