#!/usr/bin/env python3
"""
Generate a self-contained Autodesk Fusion / EAGLE 9.x schematic (.sch) for the
BrushlessLamp custom PCB -- now with REAL FOOTPRINTS (packages + pad mapping) on
every part, so the design is board-ready (schematic -> board generates pads).

Symbols are generic boxes carrying the real pin names; packages are standard
land patterns with the correct pad-to-pin mapping. Pinouts verified from
datasheets: WROOM-1, DRV8313 (HTSSOP-28), LMR51430 (SOT-23-6: 1GND 2SW 3VIN
4FB 5EN 6CB), USBLC6-2SC6, AO3400 (SOT-23 1G 2S 3D).

The WROOM-1 / USB-C / DRV8313 / USBLC6 / AO3400 parts use the official vendor
.lbr footprints from ../components (see README.md) -- the generic WROOM1/USBC16
devicesets below are dead code kept for reference only. Everything else uses
standard IPC-ish chip/discrete/connector land patterns.

Output: BrushlessLamp.sch  (Fusion: upload into an Electronics project).
"""
import xml.sax.saxutils as su

PITCH = 2.54
def esc(s): return su.escape(str(s), {'"': "&quot;"})

# ----------------------------------------------------------------- SYMBOLS (pins)
SYMS = {
 "RES": ["1","2"], "CAP": ["1","2"], "IND": ["1","2"], "FUSE": ["1","2"],
 "DIODE": ["A","C"], "FET": ["G","D","S"], "SWITCH": ["1","2"],
 "ESP32S3": ["3V3","GND","EN","IO0","IO4","IO5","IO6","IO7","IO8","IO9","IO10",
             "IO11","IO12","IO14","IO15","IO16","IO17","IO18","IO19","IO20","IO21",
             "IO43","IO44"],
 "USBC": ["VBUS","GND","CC1","CC2","DP","DM"],
 "USBLC6": ["DM_CH","GND","DM_CN","DP_CN","VBUS","DP_CH"],
 "DRV8313": ["VM","GND","OUT1","OUT2","OUT3","IN1","IN2","IN3","EN","NSLEEP",
             "NRESET","NFAULT","V3P3","VCP","CPH","CPL"],
 "BUCK": ["VIN","GND","EN","FB","SW","BST"],
 "CONN_PWR": ["VP","VN"], "CONN_MOTOR": ["U","V","W"],
 "CONN_SENSOR": ["VCC","GND","A","B","Z"], "CONN_KNOB": ["A","B","GND"],
 "CONN_BTN": ["SW","GND"], "CONN_LED": ["VP","W","C"],
 "CONN_PROG": ["3V3","GND","EN","IO0","TXD","RXD"],
 "CONN_EXP": ["3V3","GND","IO1","IO2","IO13","IO38","IO39","IO40","IO41","IO42"],
}

# ----------------------------------------------------------------- PACKAGES (pads)
def smd(n,x,y,dx,dy): return (f'<smd name="{esc(n)}" x="{x:.3f}" y="{y:.3f}" '
    f'dx="{dx:.3f}" dy="{dy:.3f}" layer="1" roundness="0" rot="R0"/>', str(n))
def thp(n,x,y,drill=0.9,dia=1.7,sq=False):
    return (f'<pad name="{esc(n)}" x="{x:.3f}" y="{y:.3f}" drill="{drill}" '
            f'diameter="{dia}" shape="{"square" if sq else "round"}"/>', str(n))
def two(x,dx,dy,a="1",b="2"): return [smd(a,-x,0,dx,dy), smd(b,x,0,dx,dy)]

def sot23():   # SOT-23 (TO-236), 0.95mm pitch; pads 0.9x1.0 (IPC-7351 nominal)
    return [smd("1",-0.95,-1.0,0.9,1.0), smd("2",0.95,-1.0,0.9,1.0), smd("3",0.0,1.0,0.9,1.0)]
def sot236():  # SOT-23-6, 0.95mm pitch; pads 0.6x1.0, rows +/-1.1 (IPC-7351 nominal)
    p=[smd(str(i+1),x,-1.1,0.6,1.0) for i,x in enumerate([-0.95,0.0,0.95])]
    p+=[smd(str(i+4),x,1.1,0.6,1.0) for i,x in enumerate([0.95,0.0,-0.95])]
    return p
def htssop28():
    p=[]; pit=0.65; ytop=(14-1)/2*pit
    for i in range(14): p.append(smd(str(i+1),-3.1,ytop-i*pit,1.45,0.4))      # 1..14 left T->B
    for i in range(14): p.append(smd(str(15+i),3.1,-ytop+i*pit,1.45,0.4))     # 15..28 right B->T
    p.append(smd("EP",0,0,3.0,3.0)); return p
def wroom1():
    # APPROX U-shape: 1..15 left(T->B), 16..25 bottom(L->R), 26..40 right(B->T), 41=EP
    p=[]; pit=1.5; n=1
    yt=(15-1)/2*pit
    for i in range(15): p.append(smd(str(n),-8.0,yt-i*pit,1.0,0.9)); n+=1
    xl=(10-1)/2*pit
    for i in range(10): p.append(smd(str(n),-xl+i*pit,-yt-1.5,0.9,1.0)); n+=1
    yb=-yt
    for i in range(15): p.append(smd(str(n),8.0,yb+i*pit,1.0,0.9)); n+=1
    p.append(smd("41",0,0,5.0,5.0)); return p
def usbc16():
    p=[]
    names=["GND1","VBUS1","CC1","DP1","DM1","VBUS2","GND2",
           "GND3","VBUS3","CC2","DP2","DM2","VBUS4","GND4"]  # 14 signal pads two rows
    # map to the connect names used: VBUS1/2, GND1/2, CC1, CC2, DP1/2, DM1/2, SH1/2
    layout=[("GND1",-3.0,1.2),("VBUS1",-2.0,1.2),("CC1",-1.0,1.2),("DP1",0.0,1.2),
            ("DM1",1.0,1.2),("VBUS2",2.0,1.2),("GND2",3.0,1.2),
            ("CC2",-1.0,-1.2),("DP2",0.0,-1.2),("DM2",1.0,-1.2)]
    for nm,x,y in layout: p.append(smd(nm,x,y,0.6,1.0))
    p.append(thp("SH1",-4.4,0,1.0,2.0)); p.append(thp("SH2",4.4,0,1.0,2.0))
    return p
def screw(npos,pitch):   # screw terminal: 1.3mm drill / 2.2mm pad (KF301 5.0mm, KF350 3.5mm; ~1.0mm square pin = 1.4mm diagonal)
    x0=-(npos-1)/2*pitch
    return [thp(str(i+1),x0+i*pitch,0,1.3,2.2,sq=(i==0)) for i in range(npos)]
def xh(npos,pitch=2.5):
    x0=-(npos-1)/2*pitch
    return [thp(str(i+1),x0+i*pitch,0,0.95,1.7,sq=(i==0)) for i in range(npos)]
def hdr(npos,pitch=2.54):
    x0=-(npos-1)/2*pitch
    return [thp(str(i+1),x0+i*pitch,0,0.9,1.6,sq=(i==0)) for i in range(npos)]
def tact():  # 4-leg SMD tactile
    return [smd("1",-3.0,1.0,1.0,0.7),smd("2",3.0,1.0,1.0,0.7),
            smd("3",-3.0,-1.0,1.0,0.7),smd("4",3.0,-1.0,1.0,0.7)]

# Chip land patterns = IPC-7351B density-B nominal: two(pad_center_offset, padX, padY) mm.
# CAP_D/CAP_V/IND43/TACT are PART-SPECIFIC (electrolytic can / polymer / inductor / switch) —
# confirm against the exact component you buy. See FOOTPRINTS.md for the full dimension table.
PKG = {
 "0402":two(0.51,0.59,0.64), "0603":two(0.80,0.90,0.95), "0805":two(0.95,1.00,1.45),
 "1206":two(1.48,1.15,1.80), "1210":two(1.48,1.15,2.65), "FUSE1206":two(1.48,1.15,1.80),
 "FUSENANO2":two(2.60,2.60,2.40),  # Littelfuse NANO2 (6.1x2.7mm body) recommended land -- verify before fab
 "CAP_D":two(2.90,2.10,2.50), "CAP_V":two(1.80,1.20,3.40), "IND43":two(1.50,1.60,3.40),
 "SOD323":two(1.35,0.80,0.90,"C","A"), "SOD123":two(1.85,0.95,1.20,"C","A"),
 "SMA":two(2.30,1.50,1.65,"C","A"), "SMB":two(2.20,2.00,2.20,"C","A"),
 "LED0603":two(0.80,0.90,0.95,"C","A"),
 "SOT23":sot23(), "SOT236":sot236(), "HTSSOP28":htssop28(), "WROOM1":wroom1(),
 "USBC16":usbc16(), "TACT":tact(),
 "SCREW2":screw(2,5.0), "SCREW3":screw(3,5.0), "SCREW5":screw(5,3.5),
 "XH2":xh(2), "XH3":xh(3), "HDR6":hdr(6), "HDR10":hdr(10),
}

# ------------------------------------------------- DEVICESETS: (sym, pkg, connects, prefix)
DEVSETS = {
 "RES_0603":("RES","0603",{"1":"1","2":"2"},"R"),
 "CAP_0402":("CAP","0402",{"1":"1","2":"2"},"C"),
 "CAP_0603":("CAP","0603",{"1":"1","2":"2"},"C"),
 "CAP_0805":("CAP","0805",{"1":"1","2":"2"},"C"),
 "CAP_1206":("CAP","1206",{"1":"1","2":"2"},"C"),
 "CAP_1210":("CAP","1210",{"1":"1","2":"2"},"C"),
 "CAP_D":("CAP","CAP_D",{"1":"1","2":"2"},"C"),
 "CAP_V":("CAP","CAP_V",{"1":"1","2":"2"},"C"),
 "IND_SMD":("IND","IND43",{"1":"1","2":"2"},"L"),
 "FUSE_1206":("FUSE","FUSE1206",{"1":"1","2":"2"},"F"),
 "FUSE_NANO2":("FUSE","FUSENANO2",{"1":"1","2":"2"},"F"),
 "DIODE_SOD323":("DIODE","SOD323",{"A":"A","C":"C"},"D"),
 "DIODE_SOD123":("DIODE","SOD123",{"A":"A","C":"C"},"D"),
 "DIODE_SMA":("DIODE","SMA",{"A":"A","C":"C"},"D"),
 "DIODE_SMB":("DIODE","SMB",{"A":"A","C":"C"},"D"),
 "LED_0603":("DIODE","LED0603",{"A":"A","C":"C"},"D"),
 "FET_SOT23":("FET","SOT23",{"G":"1","S":"2","D":"3"},"Q"),
 "SW_TACT":("SWITCH","TACT",{"1":"1 3","2":"2 4"},"SW"),
 "ESP32":("ESP32S3","WROOM1",{
    "3V3":"2","GND":"1 40 41","EN":"3","IO0":"27","IO4":"4","IO5":"5","IO6":"6",
    "IO7":"7","IO8":"12","IO9":"17","IO10":"18","IO11":"19","IO12":"20","IO14":"22",
    "IO15":"8","IO16":"9","IO17":"10","IO18":"11","IO19":"13","IO20":"14","IO21":"23",
    "IO43":"37","IO44":"36"},"U"),
 "USBLC_DS":("USBLC6","SOT236",{"DM_CH":"1","GND":"2","DM_CN":"3","DP_CN":"4",
    "VBUS":"5","DP_CH":"6"},"D"),
 "BUCK_DS":("BUCK","SOT236",{"VIN":"3","GND":"1","EN":"5","FB":"4","SW":"2","BST":"6"},"U"),
 "DRV_DS":("DRV8313","HTSSOP28",{"VM":"4 11","GND":"14 20 28 6 7 10 12 13 19 EP",
    "OUT1":"5","OUT2":"8","OUT3":"9","IN1":"27","IN2":"25","IN3":"23","EN":"26 24 22",
    "NSLEEP":"17","NRESET":"16","NFAULT":"18","V3P3":"15","VCP":"3","CPH":"2","CPL":"1"},"U"),
 "USBC_DS":("USBC","USBC16",{"VBUS":"VBUS1 VBUS2","GND":"GND1 GND2 SH1 SH2",
    "CC1":"CC1","CC2":"CC2","DP":"DP1 DP2","DM":"DM1 DM2"},"J"),
 "CONN_PWR_DS":("CONN_PWR","SCREW2",{"VP":"1","VN":"2"},"J"),
 "CONN_MOTOR_DS":("CONN_MOTOR","SCREW3",{"U":"1","V":"2","W":"3"},"J"),
 "CONN_SENSOR_DS":("CONN_SENSOR","SCREW5",{"VCC":"1","GND":"2","A":"3","B":"4","Z":"5"},"J"),
 "CONN_KNOB_DS":("CONN_KNOB","XH3",{"A":"1","B":"2","GND":"3"},"J"),
 "CONN_BTN_DS":("CONN_BTN","XH2",{"SW":"1","GND":"2"},"J"),
 "CONN_LED_DS":("CONN_LED","XH3",{"VP":"1","W":"2","C":"3"},"J"),
 "CONN_PROG_DS":("CONN_PROG","HDR6",{"3V3":"1","GND":"2","EN":"3","IO0":"4","TXD":"5","RXD":"6"},"J"),
 "CONN_EXP_DS":("CONN_EXP","HDR10",{"3V3":"1","GND":"2","IO1":"3","IO2":"4","IO13":"5",
    "IO38":"6","IO39":"7","IO40":"8","IO41":"9","IO42":"10"},"J"),
}

# ============ REAL vendor libraries (downloaded .lbr in ../components) =============
# These parts use the official SnapMagic symbols/footprints; wiring uses the REAL
# pin names found in each .lbr.  Everything else stays on the generic BLLIB library.
import os, xml.etree.ElementTree as ET
COMP_DIR = os.path.join(os.path.dirname(__file__), "..", "components")
REAL_FILES = {
 "ESP32-S3-WROOM-1-N8R8": "ESP32-S3-WROOM-1-N8R8.lbr",
 "DRV8313PWPR":           "DRV8313PWPR.lbr",
 "TYPE-C-31-M-12":        "TYPE-C-31-M-12.lbr",
 "USBLC6-2SC6":           "USBLC6-2SC6.lbr",
 "AO3400A":               "AO3400A.lbr",
}
# part -> (libname, deviceset, {REAL_pin: net})
REPLACED = {
 "U_MCU": ("ESP32-S3-WROOM-1-N8R8","ESP32-S3-WROOM-1-N8R8",{
    "3V3":"P3V3","GND":"GND","EN":"EN_RST","IO0":"BOOT","IO4":"PWMA","IO5":"PWMB",
    "IO6":"PWMC","IO7":"NSLEEP","IO8":"ROTA","IO9":"ROTB","IO10":"BTN","IO11":"LEDWW",
    "IO12":"LEDCW","IO14":"ENCZ","IO15":"DRVEN","IO16":"NFAULT","IO17":"ENCA",
    "IO18":"ENCB","IO19":"USB_DM_MCU","IO20":"USB_DP_MCU","IO21":"STAT",
    "TXD0":"U0TXD","RXD0":"U0RXD",
    # spare GPIOs broken out to the J_EXP expansion header (unused by firmware)
    "IO1":"EXP_IO1","IO2":"EXP_IO2","IO13":"EXP_IO13","IO38":"EXP_IO38",
    "IO39":"EXP_IO39","IO40":"EXP_IO40","IO41":"EXP_IO41","IO42":"EXP_IO42"}),
 "U_DRV": ("DRV8313PWPR","DRV8313PWPR",{
    "VM":"P24","GND":"GND","OUT1":"MOT_U","OUT2":"MOT_V","OUT3":"MOT_W",
    "IN1":"PWMA","IN2":"PWMB","IN3":"PWMC","EN1":"DRVEN","EN2":"DRVEN","EN3":"DRVEN",
    "!SLEEP":"NSLEEP","!RESET":"NRESET","!FAULT":"NFAULT","V3P3":"DRV_V3P3","VCP":"VCP",
    "CPH":"CPH","CPL":"CPL","PGND1":"GND","PGND2":"GND","PGND3":"GND","COMPP":"GND",
    "COMPN":"GND","!COMPO":"GND","EXP":"GND"}),   # NC left unconnected
 "J_USB": ("TYPE-C-31-M-12","TYPE-C-31-M-12",{
    "VBUS":"VBUS","GND":"GND","SHIELD":"GND","CC1":"CC1","CC2":"CC2",
    "DP1":"USBDP_RAW","DP2":"USBDP_RAW","DN1":"USBDM_RAW","DN2":"USBDM_RAW"}),  # SBU NC
 "D_USB": ("USBLC6-2SC6","USBLC6-2SC6",{
    "GND":"GND","VBUS":"VBUS","I/O1_IN":"USBDM_RAW","I/O1_OUT":"USBDM_RAW",
    "I/O2_IN":"USBDP_RAW","I/O2_OUT":"USBDP_RAW"}),
 "Q_WW": ("AO3400A","AO3400A",{"G":"GW_G","D":"LEDW_D","S":"GND"}),
 "Q_CW": ("AO3400A","AO3400A",{"G":"CW_G","D":"LEDC_D","S":"GND"}),
}

# ----------------------------------------------------- PARTS: (devset, value, pin->net)
# (generic BLLIB parts; the 6 parts in REPLACED above are NOT listed here)
PARTS = {
 "C_MCU1": ("CAP_0805","10u",{"1":"P3V3","2":"GND"}),
 "C_MCU2": ("CAP_0603","1u",{"1":"P3V3","2":"GND"}),
 "C_MCU3": ("CAP_0402","100n",{"1":"P3V3","2":"GND"}),
 "C_MCU4": ("CAP_0402","100n",{"1":"P3V3","2":"GND"}),
 "C_MCU_BULK": ("CAP_D","470u",{"1":"P3V3","2":"GND"}),
 "D_MCU": ("DIODE_SOD323","ESD",{"C":"P3V3","A":"GND"}),
 "R_EN": ("RES_0603","10k",{"1":"P3V3","2":"EN_RST"}),
 "C_EN": ("CAP_0603","1u",{"1":"EN_RST","2":"GND"}),
 "SW_RST": ("SW_TACT","RESET",{"1":"EN_RST","2":"GND"}),
 "R_BOOT": ("RES_0603","10k",{"1":"P3V3","2":"BOOT"}),
 "SW_BOOT": ("SW_TACT","BOOT",{"1":"BOOT","2":"GND"}),
 "C_BOOT": ("CAP_0402","100p",{"1":"BOOT","2":"GND"}),   # GPIO0: small only - NEVER ~0.1uF (download-mode strap risk)
 "R_CC1": ("RES_0603","5.1k",{"1":"CC1","2":"GND"}),
 "R_CC2": ("RES_0603","5.1k",{"1":"CC2","2":"GND"}),
 "R_DP": ("RES_0603","22R",{"1":"USBDP_RAW","2":"USB_DP_MCU"}),
 "R_DM": ("RES_0603","22R",{"1":"USBDM_RAW","2":"USB_DM_MCU"}),
 "C_VBUS": ("CAP_0603","4.7u",{"1":"VBUS","2":"GND"}),
 "D_OR_USB": ("DIODE_SMA","SS14",{"A":"VBUS","C":"P24"}),   # diode-OR cathode -> buck input
 "J_PROG": ("CONN_PROG_DS","UART-FALLBACK",{"3V3":"P3V3","GND":"GND","EN":"EN_RST",
    "IO0":"BOOT","TXD":"U0TXD","RXD":"U0RXD"}),
 "J_PWR": ("CONN_PWR_DS","24V-IN",{"VP":"P24_RAW","VN":"GND"}),
 "F1": ("FUSE_NANO2","2A",{"1":"P24_RAW","2":"P24_F"}),
 "Q1": ("FET_SOT23","PMOS-RevProt",{"D":"P24_F","S":"P24","G":"Q1G"}),   # D=fused input, S=load: body diode blocks reversed input, channel shorts it in normal polarity
 "R_Q1": ("RES_0603","100k",{"1":"Q1G","2":"GND"}),
 "D_Q1": ("DIODE_SOD123","10V-Zener",{"A":"Q1G","C":"P24"}),   # clamps Q1 gate-to-SOURCE (AO3401A Vgs abs-max ±12V)
 "TVS1": ("DIODE_SMB","SMBJ28A",{"A":"GND","C":"P24"}),
 "C_IN_BULK": ("CAP_V","100u-50V",{"1":"P24","2":"GND"}),
 "U_BUCK": ("BUCK_DS","LMR51430",{"VIN":"P24","GND":"GND","EN":"BUCK_EN","FB":"FB",
    "SW":"SW","BST":"BST"}),
 "C_IN1": ("CAP_1210","10u-50V",{"1":"P24","2":"GND"}),
 "C_IN2": ("CAP_1206","4.7u-50V",{"1":"P24","2":"GND"}),
 "R_ENB": ("RES_0603","100k",{"1":"P24","2":"BUCK_EN"}),
 "L1": ("IND_SMD","4.7uH",{"1":"SW","2":"P3V3"}),
 "C_BOOT_B": ("CAP_0402","100n",{"1":"BST","2":"SW"}),
 "C_OUT1": ("CAP_0805","22u",{"1":"P3V3","2":"GND"}),
 "C_OUT2": ("CAP_0805","22u",{"1":"P3V3","2":"GND"}),
 "R_FBT": ("RES_0603","45.3k",{"1":"P3V3","2":"FB"}),
 "R_FBB": ("RES_0603","10k",{"1":"FB","2":"GND"}),
 "C_FF": ("CAP_0402","22p",{"1":"P3V3","2":"FB"}),
 "C_CP": ("CAP_0603","10n",{"1":"CPH","2":"CPL"}),
 "C_VCP": ("CAP_0603","100n",{"1":"VCP","2":"P24"}),
 "C_V3P3": ("CAP_0603","470n",{"1":"DRV_V3P3","2":"GND"}),
 "C_VM1": ("CAP_0603","100n-50V",{"1":"P24","2":"GND"}),
 "C_VM2": ("CAP_0603","100n-50V",{"1":"P24","2":"GND"}),
 "C_VM_BULK": ("CAP_V","100u-50V",{"1":"P24","2":"GND"}),
 "R_NSLEEP": ("RES_0603","10k",{"1":"P3V3","2":"NSLEEP"}),
 "R_NRESET": ("RES_0603","10k",{"1":"P3V3","2":"NRESET"}),
 "R_NFAULT": ("RES_0603","10k",{"1":"P3V3","2":"NFAULT"}),
 "R_EN_DRV": ("RES_0603","10k",{"1":"P3V3","2":"DRVEN"}),
 "J_MOTOR": ("CONN_MOTOR_DS","MOTOR",{"U":"MOT_U","V":"MOT_V","W":"MOT_W"}),
 "J_SENSOR": ("CONN_SENSOR_DS","MT6701",{"VCC":"P3V3","GND":"GND","A":"ENCA","B":"ENCB","Z":"ENCZ"}),
 "C_SENS": ("CAP_0402","100n",{"1":"P3V3","2":"GND"}),
 # optional encoder A/B/Z pull-up stiffening for a long/noisy cable -- DNP by default
 "R_AZ_A": ("RES_0603","10k-DNP",{"1":"P3V3","2":"ENCA"}),
 "R_AZ_B": ("RES_0603","10k-DNP",{"1":"P3V3","2":"ENCB"}),
 "R_AZ_C": ("RES_0603","10k-DNP",{"1":"P3V3","2":"ENCZ"}),
 "J_LED": ("CONN_LED_DS","CCT-STRIP",{"VP":"P24","W":"LEDW_D","C":"LEDC_D"}),
 "R_GW": ("RES_0603","33R",{"1":"GW_G","2":"LEDWW"}),
 "R_GC": ("RES_0603","33R",{"1":"CW_G","2":"LEDCW"}),
 "R_PDW": ("RES_0603","100k",{"1":"GW_G","2":"GND"}),
 "R_PDC": ("RES_0603","100k",{"1":"CW_G","2":"GND"}),
 "D_CLW": ("DIODE_SOD323","clamp-DNP",{"A":"LEDW_D","C":"P24"}),
 "D_CLC": ("DIODE_SOD323","clamp-DNP",{"A":"LEDC_D","C":"P24"}),
 "J_KNOB": ("CONN_KNOB_DS","KNOB",{"A":"ROTA","B":"ROTB","GND":"GND"}),
 "J_BTN": ("CONN_BTN_DS","BUTTON",{"SW":"BTN","GND":"GND"}),
 "J_EXP": ("CONN_EXP_DS","GPIO-EXP",{"3V3":"P3V3","GND":"GND","IO1":"EXP_IO1",
    "IO2":"EXP_IO2","IO13":"EXP_IO13","IO38":"EXP_IO38","IO39":"EXP_IO39",
    "IO40":"EXP_IO40","IO41":"EXP_IO41","IO42":"EXP_IO42"}),
 "R_STATUS": ("RES_0603","1k",{"1":"STAT","2":"STAT_A"}),
 "LED_STATUS": ("LED_0603","STATUS",{"A":"STAT_A","C":"GND"}),
}

# ----------------------------------------------------------------- builders
def sym_coords(pins):
    n=len(pins); half=(n+1)//2; left,right=pins[:half],pins[half:]; c={}
    for i,p in enumerate(left):  c[p]=(-12.7,((len(left)-1)/2-i)*PITCH)
    for i,p in enumerate(right): c[p]=( 12.7,((len(right)-1)/2-i)*PITCH)
    return c

def build_symbol(name,pins):
    co=sym_coords(pins); ms=max((len(pins)+1)//2,len(pins)//2)
    yt=((ms-1)/2)*PITCH+PITCH
    o=[f'<symbol name="{esc(name)}">']
    o+=[f'<wire x1="-10.16" y1="{-yt:.2f}" x2="10.16" y2="{-yt:.2f}" width="0.254" layer="94"/>',
        f'<wire x1="10.16" y1="{-yt:.2f}" x2="10.16" y2="{yt:.2f}" width="0.254" layer="94"/>',
        f'<wire x1="10.16" y1="{yt:.2f}" x2="-10.16" y2="{yt:.2f}" width="0.254" layer="94"/>',
        f'<wire x1="-10.16" y1="{yt:.2f}" x2="-10.16" y2="{-yt:.2f}" width="0.254" layer="94"/>',
        f'<text x="-10.16" y="{yt+1.0:.2f}" size="1.778" layer="95">&gt;NAME</text>',
        f'<text x="-10.16" y="{-yt-2.8:.2f}" size="1.778" layer="96">&gt;VALUE</text>']
    for p in pins:
        x,y=co[p]; rot="R0" if x<0 else "R180"
        o.append(f'<pin name="{esc(p)}" x="{x:.2f}" y="{y:.2f}" visible="pin" '
                 f'length="point" direction="pas" rot="{rot}"/>')
    o.append('</symbol>'); return "\n".join(o),co

# Silkscreen polarity / pin-1 marks (layer 21 tPlace) so PCBWay can place polarised
# parts correctly. Diode pkgs: cathode bar at the C pad (left). Polarised caps: '+' at
# pad 1 (left). SOT-23-6 buck: pin-1 dot. (Vendor footprints already carry their own.)
def _bar(x,h):  return f'<wire x1="{x:.2f}" y1="{-h:.2f}" x2="{x:.2f}" y2="{h:.2f}" width="0.15" layer="21"/>'
def _plus(x,y): return f'<text x="{x:.2f}" y="{y:.2f}" size="1.0" layer="21">+</text>'
def _dot(x,y):  return f'<circle x="{x:.2f}" y="{y:.2f}" radius="0.1" width="0.3" layer="21"/>'
SILK={
 "SOD323":[_bar(-1.95,0.55)], "SOD123":[_bar(-2.55,0.70)],
 "SMA":[_bar(-3.30,0.95)],    "SMB":[_bar(-3.45,1.25)],
 "LED0603":[_bar(-1.45,0.55)],
 "CAP_D":[_plus(-3.6,1.0)],   "CAP_V":[_plus(-2.6,1.4)],
 "SOT236":[_dot(-1.60,-1.55)],
}
def build_package(name,pad_tuples):
    pads=[t[0] for t in pad_tuples]; names=[t[1] for t in pad_tuples]
    o=[f'<package name="{esc(name)}">',
       f'<text x="0" y="0" size="1.0" layer="25" align="center">&gt;NAME</text>']
    o+=pads; o+=SILK.get(name,[]); o.append('</package>'); return "\n".join(o),set(names)

BASE_LAYERS={90:"Modules",91:"Nets",92:"Busses",93:"Pins",94:"Symbols",
             95:"Names",96:"Values",97:"Info",98:"Guide",
             1:"Top",16:"Bottom",17:"Pads",18:"Vias",19:"Unrouted",20:"Dimension",
             21:"tPlace",22:"bPlace",25:"tNames",27:"tValues",29:"tStop",51:"tDocu"}

def load_real_libs():
    """Parse the downloaded .lbr files: embeddable <library> XML + pin coords."""
    libs={}; layers={}
    for name,fn in REAL_FILES.items():
        root=ET.parse(os.path.join(COMP_DIR,fn)).getroot()
        for L in root.findall(".//layers/layer"): layers[int(L.get("number"))]=L.get("name")
        lib=root.find(".//library"); lib.set("name",name)
        for p3 in lib.findall("packages3d"): lib.remove(p3)          # drop cloud 3D refs
        for d in lib.findall(".//device"):
            for pi in d.findall("package3dinstances"): d.remove(pi)
        symco={sym.get("name"):{p.get("name"):(float(p.get("x")),float(p.get("y")))
                                for p in sym.findall("pin")}
               for sym in lib.findall(".//symbol")}
        ds2sym={ds.get("name"):ds.find(".//gate").get("symbol")
                for ds in lib.findall(".//deviceset")}
        libs[name]=(ET.tostring(lib,encoding="unicode"),symco,ds2sym)
    return libs,layers

def emit_layers(d):
    return "\n".join(
        f'<layer number="{n}" name="{esc(nm)}" color="7" fill="1" visible="yes" active="yes"/>'
        for n,nm in sorted(d.items()))

def main():
    real,real_layers=load_real_libs()

    # ---- validate generic devicesets (pad/pin consistency)
    pkg_pads={n:set(t[1] for t in pads) for n,pads in PKG.items()}; errs=0
    for ds,(sym,pkg,conn,_pref) in DEVSETS.items():
        for pin,padstr in conn.items():
            for pad in padstr.split():
                if pad not in pkg_pads[pkg]: print(f"ERR {ds}: pad {pad} not in {pkg}"); errs+=1
    for nm,(ds,val,pm) in PARTS.items():
        for pin in pm:
            if pin not in SYMS[DEVSETS[ds][0]]:
                print(f"ERR {nm}: pin {pin} not in symbol {DEVSETS[ds][0]}"); errs+=1
    # ---- validate REPLACED parts against the REAL symbol pins
    for nm,(lib,ds,pm) in REPLACED.items():
        sym=real[lib][2][ds]; rp=set(real[lib][1][sym])
        for pin in pm:
            if pin not in rp: print(f"ERR {nm}: real pin {pin} not in {lib}/{sym}"); errs+=1
    if errs: raise SystemExit(f"{errs} errors")

    # ---- generic library content
    pkg_xml=[build_package(n,pads)[0] for n,pads in PKG.items()]
    sym_xml=[]; coords={}
    for s,pins in SYMS.items():
        x,co=build_symbol(s,pins); sym_xml.append(x); coords[s]=co
    dev_xml=[]
    for ds,(sym,pkg,conn,pref) in DEVSETS.items():
        cx="".join(f'<connect gate="G$1" pin="{esc(p)}" pad="{esc(pad)}"/>' for p,pad in conn.items())
        dev_xml.append(
            f'<deviceset name="{esc(ds)}" prefix="{esc(pref)}" uservalue="yes">'
            f'<gates><gate name="G$1" symbol="{esc(sym)}" x="0" y="0"/></gates>'
            f'<devices><device name="" package="{esc(pkg)}"><connects>{cx}</connects>'
            f'<technologies><technology name=""/></technologies></device></devices></deviceset>')

    # ---- unified part list: (name, libref, deviceset, value_or_None, pin->net, coords)
    items=[]
    for nm,(ds,val,pm) in PARTS.items():
        items.append((nm,"BLLIB",ds,val,pm,coords[DEVSETS[ds][0]]))
    for nm,(lib,ds,pm) in REPLACED.items():
        items.append((nm,lib,ds,None,pm,real[lib][1][real[lib][2][ds]]))

    # ---- place on a wide grid (avoid pin-coordinate collisions for big symbols)
    parts_xml=[]; inst=[]; placed={}
    for i,(nm,libref,ds,val,pm,co) in enumerate(items):
        col,row=i%6,i//6; x,y=col*88.9,-row*76.2; placed[nm]=(x,y)
        vattr=f' value="{esc(val)}"' if val is not None else ""
        parts_xml.append(f'<part name="{esc(nm)}" library="{esc(libref)}" '
                         f'deviceset="{esc(ds)}" device=""{vattr}/>')
        inst.append(f'<instance part="{esc(nm)}" gate="G$1" x="{x:.2f}" y="{y:.2f}" rot="R0"/>')

    # ---- nets: per-pin labelled stub segments grouped by net name
    nets={}
    for nm,libref,ds,val,pm,co in items:
        ix,iy=placed[nm]
        for pin,net in pm.items():
            px,py=co[pin]; cx,cy=ix+px,iy+py
            if px<0: ex,ey=cx-PITCH,cy
            elif px>0: ex,ey=cx+PITCH,cy
            elif py>=0: ex,ey=cx,cy+PITCH
            else: ex,ey=cx,cy-PITCH
            nets.setdefault(net,[]).append(
                f'<segment><pinref part="{esc(nm)}" gate="G$1" pin="{esc(pin)}"/>'
                f'<wire x1="{cx:.2f}" y1="{cy:.2f}" x2="{ex:.2f}" y2="{ey:.2f}" width="0.1524" layer="91"/>'
                f'<label x="{ex:.2f}" y="{ey:.2f}" size="1.27" layer="91"/></segment>')
    NET_CLASS={"P24":"1","P24_RAW":"1","P24_F":"1","P3V3":"2","VBUS":"2"}   # wider power copper
    nets_xml=[f'<net name="{esc(n)}" class="{NET_CLASS.get(n,"0")}">'+"".join(nets[n])+'</net>' for n in sorted(nets)]

    all_layers={**real_layers,**BASE_LAYERS}
    real_lib_xml="\n".join(real[n][0] for n in REAL_FILES)

    doc=f'''<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE eagle SYSTEM "eagle.dtd">
<eagle version="9.6.2">
<drawing>
<settings>
<setting alwaysvectorfont="no"/>
<setting verticaltext="up"/>
</settings>
<grid distance="0.1" unitdist="inch" unit="inch" style="lines" multiple="1" display="no" altdistance="0.01" altunitdist="inch" altunit="inch"/>
<layers>
{emit_layers(all_layers)}
</layers>
<schematic xreflabel="%F%N/%S.%C%R" xrefpart="/%S.%C%R">
<libraries>
<library name="BLLIB">
<description>BrushlessLamp generic symbols + standard footprints (passives, connectors, buck, P-FET). The MCU/DRV8313/USB-C/USBLC6/AO3400 use the official vendor libraries below.</description>
<packages>
{chr(10).join(pkg_xml)}
</packages>
<symbols>
{chr(10).join(sym_xml)}
</symbols>
<devicesets>
{chr(10).join(dev_xml)}
</devicesets>
</library>
{real_lib_xml}
</libraries>
<attributes/>
<variantdefs/>
<classes>
<class number="0" name="default" width="0" drill="0"/>
<class number="1" name="pwr24" width="0.5" drill="0"/>
<class number="2" name="pwr3v3" width="0.4" drill="0"/>
</classes>
<parts>
{chr(10).join(parts_xml)}
</parts>
<sheets>
<sheet>
<plain/>
<instances>
{chr(10).join(inst)}
</instances>
<busses/>
<nets>
{chr(10).join(nets_xml)}
</nets>
</sheet>
</sheets>
</schematic>
</drawing>
</eagle>
'''
    open("BrushlessLamp.sch","w").write(doc)
    print(f"parts={len(items)} (generic={len(PARTS)}, vendor={len(REPLACED)}) "
          f"vendor_libs={len(REAL_FILES)} nets={len(nets)}")

if __name__=="__main__":
    main()
