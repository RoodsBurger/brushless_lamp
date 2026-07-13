<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE eagle SYSTEM "eagle.dtd">
<eagle version="9.7.0">
<drawing>
<settings>
<setting alwaysvectorfont="no"/>
<setting verticaltext="up"/>
</settings>
<grid distance="0.1" unitdist="inch" unit="inch" style="lines" multiple="1" display="no" altdistance="0.01" altunitdist="inch" altunit="inch"/>
<layers>
<layer number="1" name="Top" color="4" fill="1" visible="no" active="no"/>
<layer number="16" name="Bottom" color="1" fill="1" visible="no" active="no"/>
<layer number="17" name="Pads" color="2" fill="1" visible="no" active="no"/>
<layer number="18" name="Vias" color="2" fill="1" visible="no" active="no"/>
<layer number="19" name="Unrouted" color="6" fill="1" visible="no" active="no"/>
<layer number="20" name="Dimension" color="24" fill="1" visible="no" active="no"/>
<layer number="21" name="tPlace" color="7" fill="1" visible="no" active="no"/>
<layer number="22" name="bPlace" color="7" fill="1" visible="no" active="no"/>
<layer number="23" name="tOrigins" color="15" fill="1" visible="no" active="no"/>
<layer number="24" name="bOrigins" color="15" fill="1" visible="no" active="no"/>
<layer number="25" name="tNames" color="7" fill="1" visible="no" active="no"/>
<layer number="26" name="bNames" color="7" fill="1" visible="no" active="no"/>
<layer number="27" name="tValues" color="7" fill="1" visible="no" active="no"/>
<layer number="28" name="bValues" color="7" fill="1" visible="no" active="no"/>
<layer number="29" name="tStop" color="7" fill="3" visible="no" active="no"/>
<layer number="30" name="bStop" color="7" fill="6" visible="no" active="no"/>
<layer number="31" name="tCream" color="7" fill="4" visible="no" active="no"/>
<layer number="32" name="bCream" color="7" fill="5" visible="no" active="no"/>
<layer number="33" name="tFinish" color="6" fill="3" visible="no" active="no"/>
<layer number="34" name="bFinish" color="6" fill="6" visible="no" active="no"/>
<layer number="35" name="tGlue" color="7" fill="4" visible="no" active="no"/>
<layer number="36" name="bGlue" color="7" fill="5" visible="no" active="no"/>
<layer number="37" name="tTest" color="7" fill="1" visible="no" active="no"/>
<layer number="38" name="bTest" color="7" fill="1" visible="no" active="no"/>
<layer number="39" name="tKeepout" color="4" fill="11" visible="no" active="no"/>
<layer number="40" name="bKeepout" color="1" fill="11" visible="no" active="no"/>
<layer number="41" name="tRestrict" color="4" fill="10" visible="no" active="no"/>
<layer number="42" name="bRestrict" color="1" fill="10" visible="no" active="no"/>
<layer number="43" name="vRestrict" color="2" fill="10" visible="no" active="no"/>
<layer number="44" name="Drills" color="7" fill="1" visible="no" active="no"/>
<layer number="45" name="Holes" color="7" fill="1" visible="no" active="no"/>
<layer number="46" name="Milling" color="3" fill="1" visible="no" active="no"/>
<layer number="47" name="Measures" color="7" fill="1" visible="no" active="no"/>
<layer number="48" name="Document" color="7" fill="1" visible="no" active="no"/>
<layer number="49" name="Reference" color="7" fill="1" visible="no" active="no"/>
<layer number="51" name="tDocu" color="7" fill="1" visible="no" active="no"/>
<layer number="52" name="bDocu" color="7" fill="1" visible="no" active="no"/>
<layer number="90" name="Modules" color="5" fill="1" visible="yes" active="yes"/>
<layer number="91" name="Nets" color="2" fill="1" visible="yes" active="yes"/>
<layer number="92" name="Busses" color="1" fill="1" visible="yes" active="yes"/>
<layer number="93" name="Pins" color="2" fill="1" visible="yes" active="yes"/>
<layer number="94" name="Symbols" color="4" fill="1" visible="yes" active="yes"/>
<layer number="95" name="Names" color="7" fill="1" visible="yes" active="yes"/>
<layer number="96" name="Values" color="7" fill="1" visible="yes" active="yes"/>
<layer number="97" name="Info" color="7" fill="1" visible="yes" active="yes"/>
<layer number="98" name="Guide" color="6" fill="1" visible="yes" active="yes"/>
</layers>
<schematic xreflabel="%F%N/%S.%C%R" xrefpart="/%S.%C%R">
<libraries>
<library name="BLLIB">
<description>BrushlessLamp generic symbols + standard footprints (passives, connectors, buck, P-FET). The MCU/DRV8313/USB-C/USBLC6/AO3400 use the official vendor libraries below.</description>
<packages>
<package name="0402">
<text x="0" y="0" size="1" layer="25" align="center">&gt;NAME</text>
<smd name="1" x="-0.51" y="0" dx="0.59" dy="0.64" layer="1"/>
<smd name="2" x="0.51" y="0" dx="0.59" dy="0.64" layer="1"/>
</package>
<package name="0603">
<text x="0" y="0" size="1" layer="25" align="center">&gt;NAME</text>
<smd name="1" x="-0.8" y="0" dx="0.9" dy="0.95" layer="1"/>
<smd name="2" x="0.8" y="0" dx="0.9" dy="0.95" layer="1"/>
</package>
<package name="0805">
<text x="0" y="0" size="1" layer="25" align="center">&gt;NAME</text>
<smd name="1" x="-0.95" y="0" dx="1" dy="1.45" layer="1"/>
<smd name="2" x="0.95" y="0" dx="1" dy="1.45" layer="1"/>
</package>
<package name="1206">
<text x="0" y="0" size="1" layer="25" align="center">&gt;NAME</text>
<smd name="1" x="-1.48" y="0" dx="1.15" dy="1.8" layer="1"/>
<smd name="2" x="1.48" y="0" dx="1.15" dy="1.8" layer="1"/>
</package>
<package name="1210">
<text x="0" y="0" size="1" layer="25" align="center">&gt;NAME</text>
<smd name="1" x="-1.48" y="0" dx="1.15" dy="2.65" layer="1"/>
<smd name="2" x="1.48" y="0" dx="1.15" dy="2.65" layer="1"/>
</package>
<package name="FUSE1206">
<text x="0" y="0" size="1" layer="25" align="center">&gt;NAME</text>
<smd name="1" x="-1.48" y="0" dx="1.15" dy="1.8" layer="1"/>
<smd name="2" x="1.48" y="0" dx="1.15" dy="1.8" layer="1"/>
</package>
<package name="FUSENANO2">
<text x="0" y="0" size="1" layer="25" align="center">&gt;NAME</text>
<smd name="1" x="-2.6" y="0" dx="2.6" dy="2.4" layer="1"/>
<smd name="2" x="2.6" y="0" dx="2.6" dy="2.4" layer="1"/>
</package>
<package name="CAP_D">
<text x="0" y="0" size="1" layer="25" align="center">&gt;NAME</text>
<smd name="1" x="-2.9" y="0" dx="2.1" dy="2.5" layer="1"/>
<smd name="2" x="2.9" y="0" dx="2.1" dy="2.5" layer="1"/>
<text x="-3.6" y="1" size="1" layer="21">+</text>
</package>
<package name="CAP_V">
<text x="0" y="0" size="1" layer="25" align="center">&gt;NAME</text>
<smd name="1" x="-3.426" y="0" dx="3.764" dy="2.115" layer="1"/>
<smd name="2" x="3.426" y="0" dx="3.764" dy="2.115" layer="1"/>
<wire x1="-4.25" y1="1.312" x2="-4.25" y2="2.654" width="0.12" layer="21"/>
<wire x1="-4.25" y1="2.654" x2="-2.654" y2="4.25" width="0.12" layer="21"/>
<wire x1="-2.654" y1="4.25" x2="4.25" y2="4.25" width="0.12" layer="21"/>
<wire x1="4.25" y1="4.25" x2="4.25" y2="1.312" width="0.12" layer="21"/>
<wire x1="-4.25" y1="-1.312" x2="-4.25" y2="-2.654" width="0.12" layer="21"/>
<wire x1="-4.25" y1="-2.654" x2="-2.654" y2="-4.25" width="0.12" layer="21"/>
<wire x1="-2.654" y1="-4.25" x2="4.25" y2="-4.25" width="0.12" layer="21"/>
<wire x1="4.25" y1="-4.25" x2="4.25" y2="-1.312" width="0.12" layer="21"/>
<wire x1="-4.25" y1="-4.25" x2="4.25" y2="-4.25" width="0.12" layer="51"/>
<wire x1="4.25" y1="-4.25" x2="4.25" y2="4.25" width="0.12" layer="51"/>
<wire x1="4.25" y1="4.25" x2="-4.25" y2="4.25" width="0.12" layer="51"/>
<wire x1="-4.25" y1="4.25" x2="-4.25" y2="-4.25" width="0.12" layer="51"/>
<wire x1="-3.4" y1="2.95" x2="-3.4" y2="3.65" width="0.2" layer="21"/>
<wire x1="-3.75" y1="3.3" x2="-3.05" y2="3.3" width="0.2" layer="21"/>
</package>
<package name="IND43">
<text x="0" y="0" size="1" layer="25" align="center">&gt;NAME</text>
<smd name="1" x="-1.5" y="0" dx="1.6" dy="3.4" layer="1"/>
<smd name="2" x="1.5" y="0" dx="1.6" dy="3.4" layer="1"/>
</package>
<package name="SOD323">
<text x="0" y="0" size="1" layer="25" align="center">&gt;NAME</text>
<smd name="C" x="-1.35" y="0" dx="0.8" dy="0.9" layer="1"/>
<smd name="A" x="1.35" y="0" dx="0.8" dy="0.9" layer="1"/>
<wire x1="-1.95" y1="-0.55" x2="-1.95" y2="0.55" width="0.15" layer="21"/>
</package>
<package name="SOD123">
<text x="0" y="0" size="1" layer="25" align="center">&gt;NAME</text>
<smd name="C" x="-1.85" y="0" dx="0.95" dy="1.2" layer="1"/>
<smd name="A" x="1.85" y="0" dx="0.95" dy="1.2" layer="1"/>
<wire x1="-2.55" y1="-0.7" x2="-2.55" y2="0.7" width="0.15" layer="21"/>
</package>
<package name="SMA">
<text x="0" y="0" size="1" layer="25" align="center">&gt;NAME</text>
<smd name="C" x="-2.3" y="0" dx="1.5" dy="1.65" layer="1"/>
<smd name="A" x="2.3" y="0" dx="1.5" dy="1.65" layer="1"/>
<wire x1="-3.3" y1="-0.95" x2="-3.3" y2="0.95" width="0.15" layer="21"/>
</package>
<package name="SMB">
<text x="0" y="0" size="1" layer="25" align="center">&gt;NAME</text>
<smd name="C" x="-2.2" y="0" dx="2" dy="2.2" layer="1"/>
<smd name="A" x="2.2" y="0" dx="2" dy="2.2" layer="1"/>
<wire x1="-3.45" y1="-1.25" x2="-3.45" y2="1.25" width="0.15" layer="21"/>
</package>
<package name="LED0603">
<text x="0" y="0" size="1" layer="25" align="center">&gt;NAME</text>
<smd name="C" x="-0.8" y="0" dx="0.9" dy="0.95" layer="1"/>
<smd name="A" x="0.8" y="0" dx="0.9" dy="0.95" layer="1"/>
<wire x1="-1.45" y1="-0.55" x2="-1.45" y2="0.55" width="0.15" layer="21"/>
</package>
<package name="SOT23">
<text x="0" y="0" size="1" layer="25" align="center">&gt;NAME</text>
<smd name="1" x="-0.95" y="-1" dx="0.9" dy="1" layer="1"/>
<smd name="2" x="0.95" y="-1" dx="0.9" dy="1" layer="1"/>
<smd name="3" x="0" y="1" dx="0.9" dy="1" layer="1"/>
</package>
<package name="SOT236">
<text x="0" y="0" size="1" layer="25" align="center">&gt;NAME</text>
<smd name="1" x="-0.95" y="-1.1" dx="0.6" dy="1" layer="1"/>
<smd name="2" x="0" y="-1.1" dx="0.6" dy="1" layer="1"/>
<smd name="3" x="0.95" y="-1.1" dx="0.6" dy="1" layer="1"/>
<smd name="4" x="0.95" y="1.1" dx="0.6" dy="1" layer="1"/>
<smd name="5" x="0" y="1.1" dx="0.6" dy="1" layer="1"/>
<smd name="6" x="-0.95" y="1.1" dx="0.6" dy="1" layer="1"/>
<circle x="-1.6" y="-1.55" radius="0.1" width="0.3" layer="21"/>
</package>
<package name="HTSSOP28">
<text x="0" y="0" size="1" layer="25" align="center">&gt;NAME</text>
<smd name="1" x="-3.1" y="4.225" dx="1.45" dy="0.4" layer="1"/>
<smd name="2" x="-3.1" y="3.575" dx="1.45" dy="0.4" layer="1"/>
<smd name="3" x="-3.1" y="2.925" dx="1.45" dy="0.4" layer="1"/>
<smd name="4" x="-3.1" y="2.275" dx="1.45" dy="0.4" layer="1"/>
<smd name="5" x="-3.1" y="1.625" dx="1.45" dy="0.4" layer="1"/>
<smd name="6" x="-3.1" y="0.975" dx="1.45" dy="0.4" layer="1"/>
<smd name="7" x="-3.1" y="0.325" dx="1.45" dy="0.4" layer="1"/>
<smd name="8" x="-3.1" y="-0.325" dx="1.45" dy="0.4" layer="1"/>
<smd name="9" x="-3.1" y="-0.975" dx="1.45" dy="0.4" layer="1"/>
<smd name="10" x="-3.1" y="-1.625" dx="1.45" dy="0.4" layer="1"/>
<smd name="11" x="-3.1" y="-2.275" dx="1.45" dy="0.4" layer="1"/>
<smd name="12" x="-3.1" y="-2.925" dx="1.45" dy="0.4" layer="1"/>
<smd name="13" x="-3.1" y="-3.575" dx="1.45" dy="0.4" layer="1"/>
<smd name="14" x="-3.1" y="-4.225" dx="1.45" dy="0.4" layer="1"/>
<smd name="15" x="3.1" y="-4.225" dx="1.45" dy="0.4" layer="1"/>
<smd name="16" x="3.1" y="-3.575" dx="1.45" dy="0.4" layer="1"/>
<smd name="17" x="3.1" y="-2.925" dx="1.45" dy="0.4" layer="1"/>
<smd name="18" x="3.1" y="-2.275" dx="1.45" dy="0.4" layer="1"/>
<smd name="19" x="3.1" y="-1.625" dx="1.45" dy="0.4" layer="1"/>
<smd name="20" x="3.1" y="-0.975" dx="1.45" dy="0.4" layer="1"/>
<smd name="21" x="3.1" y="-0.325" dx="1.45" dy="0.4" layer="1"/>
<smd name="22" x="3.1" y="0.325" dx="1.45" dy="0.4" layer="1"/>
<smd name="23" x="3.1" y="0.975" dx="1.45" dy="0.4" layer="1"/>
<smd name="24" x="3.1" y="1.625" dx="1.45" dy="0.4" layer="1"/>
<smd name="25" x="3.1" y="2.275" dx="1.45" dy="0.4" layer="1"/>
<smd name="26" x="3.1" y="2.925" dx="1.45" dy="0.4" layer="1"/>
<smd name="27" x="3.1" y="3.575" dx="1.45" dy="0.4" layer="1"/>
<smd name="28" x="3.1" y="4.225" dx="1.45" dy="0.4" layer="1"/>
<smd name="EP" x="0" y="0" dx="3" dy="3" layer="1"/>
</package>
<package name="WROOM1">
<text x="0" y="0" size="1" layer="25" align="center">&gt;NAME</text>
<smd name="1" x="-8" y="10.5" dx="1" dy="0.9" layer="1"/>
<smd name="2" x="-8" y="9" dx="1" dy="0.9" layer="1"/>
<smd name="3" x="-8" y="7.5" dx="1" dy="0.9" layer="1"/>
<smd name="4" x="-8" y="6" dx="1" dy="0.9" layer="1"/>
<smd name="5" x="-8" y="4.5" dx="1" dy="0.9" layer="1"/>
<smd name="6" x="-8" y="3" dx="1" dy="0.9" layer="1"/>
<smd name="7" x="-8" y="1.5" dx="1" dy="0.9" layer="1"/>
<smd name="8" x="-8" y="0" dx="1" dy="0.9" layer="1"/>
<smd name="9" x="-8" y="-1.5" dx="1" dy="0.9" layer="1"/>
<smd name="10" x="-8" y="-3" dx="1" dy="0.9" layer="1"/>
<smd name="11" x="-8" y="-4.5" dx="1" dy="0.9" layer="1"/>
<smd name="12" x="-8" y="-6" dx="1" dy="0.9" layer="1"/>
<smd name="13" x="-8" y="-7.5" dx="1" dy="0.9" layer="1"/>
<smd name="14" x="-8" y="-9" dx="1" dy="0.9" layer="1"/>
<smd name="15" x="-8" y="-10.5" dx="1" dy="0.9" layer="1"/>
<smd name="16" x="-6.75" y="-12" dx="0.9" dy="1" layer="1"/>
<smd name="17" x="-5.25" y="-12" dx="0.9" dy="1" layer="1"/>
<smd name="18" x="-3.75" y="-12" dx="0.9" dy="1" layer="1"/>
<smd name="19" x="-2.25" y="-12" dx="0.9" dy="1" layer="1"/>
<smd name="20" x="-0.75" y="-12" dx="0.9" dy="1" layer="1"/>
<smd name="21" x="0.75" y="-12" dx="0.9" dy="1" layer="1"/>
<smd name="22" x="2.25" y="-12" dx="0.9" dy="1" layer="1"/>
<smd name="23" x="3.75" y="-12" dx="0.9" dy="1" layer="1"/>
<smd name="24" x="5.25" y="-12" dx="0.9" dy="1" layer="1"/>
<smd name="25" x="6.75" y="-12" dx="0.9" dy="1" layer="1"/>
<smd name="26" x="8" y="-10.5" dx="1" dy="0.9" layer="1"/>
<smd name="27" x="8" y="-9" dx="1" dy="0.9" layer="1"/>
<smd name="28" x="8" y="-7.5" dx="1" dy="0.9" layer="1"/>
<smd name="29" x="8" y="-6" dx="1" dy="0.9" layer="1"/>
<smd name="30" x="8" y="-4.5" dx="1" dy="0.9" layer="1"/>
<smd name="31" x="8" y="-3" dx="1" dy="0.9" layer="1"/>
<smd name="32" x="8" y="-1.5" dx="1" dy="0.9" layer="1"/>
<smd name="33" x="8" y="0" dx="1" dy="0.9" layer="1"/>
<smd name="34" x="8" y="1.5" dx="1" dy="0.9" layer="1"/>
<smd name="35" x="8" y="3" dx="1" dy="0.9" layer="1"/>
<smd name="36" x="8" y="4.5" dx="1" dy="0.9" layer="1"/>
<smd name="37" x="8" y="6" dx="1" dy="0.9" layer="1"/>
<smd name="38" x="8" y="7.5" dx="1" dy="0.9" layer="1"/>
<smd name="39" x="8" y="9" dx="1" dy="0.9" layer="1"/>
<smd name="40" x="8" y="10.5" dx="1" dy="0.9" layer="1"/>
<smd name="41" x="0" y="0" dx="5" dy="5" layer="1"/>
</package>
<package name="USBC16">
<text x="0" y="0" size="1" layer="25" align="center">&gt;NAME</text>
<smd name="GND1" x="-3" y="1.2" dx="0.6" dy="1" layer="1"/>
<smd name="VBUS1" x="-2" y="1.2" dx="0.6" dy="1" layer="1"/>
<smd name="CC1" x="-1" y="1.2" dx="0.6" dy="1" layer="1"/>
<smd name="DP1" x="0" y="1.2" dx="0.6" dy="1" layer="1"/>
<smd name="DM1" x="1" y="1.2" dx="0.6" dy="1" layer="1"/>
<smd name="VBUS2" x="2" y="1.2" dx="0.6" dy="1" layer="1"/>
<smd name="GND2" x="3" y="1.2" dx="0.6" dy="1" layer="1"/>
<smd name="CC2" x="-1" y="-1.2" dx="0.6" dy="1" layer="1"/>
<smd name="DP2" x="0" y="-1.2" dx="0.6" dy="1" layer="1"/>
<smd name="DM2" x="1" y="-1.2" dx="0.6" dy="1" layer="1"/>
<pad name="SH1" x="-4.4" y="0" drill="1" diameter="2"/>
<pad name="SH2" x="4.4" y="0" drill="1" diameter="2"/>
</package>
<package name="SCREW2">
<text x="0" y="0" size="1" layer="25" align="center">&gt;NAME</text>
<pad name="1" x="-2.5" y="0" drill="1.3" diameter="2.2" shape="square"/>
<pad name="2" x="2.5" y="0" drill="1.3" diameter="2.2"/>
<wire x1="-5" y1="-3.6" x2="5" y2="-3.6" width="0.127" layer="21"/>
<wire x1="5" y1="-3.6" x2="5" y2="4" width="0.127" layer="21"/>
<wire x1="5" y1="4" x2="-5" y2="4" width="0.127" layer="21"/>
<wire x1="-5" y1="4" x2="-5" y2="-3.6" width="0.127" layer="21"/>
<wire x1="-5" y1="-3.6" x2="5" y2="-3.6" width="0.12" layer="51"/>
<wire x1="5" y1="-3.6" x2="5" y2="4" width="0.12" layer="51"/>
<wire x1="5" y1="4" x2="-5" y2="4" width="0.12" layer="51"/>
<wire x1="-5" y1="4" x2="-5" y2="-3.6" width="0.12" layer="51"/>
</package>
<package name="SCREW3">
<text x="0" y="0" size="1" layer="25" align="center">&gt;NAME</text>
<pad name="1" x="-5" y="0" drill="1.3" diameter="2.2" shape="square"/>
<pad name="2" x="0" y="0" drill="1.3" diameter="2.2"/>
<pad name="3" x="5" y="0" drill="1.3" diameter="2.2"/>
<wire x1="-7.5" y1="-3.6" x2="7.5" y2="-3.6" width="0.127" layer="21"/>
<wire x1="7.5" y1="-3.6" x2="7.5" y2="4" width="0.127" layer="21"/>
<wire x1="7.5" y1="4" x2="-7.5" y2="4" width="0.127" layer="21"/>
<wire x1="-7.5" y1="4" x2="-7.5" y2="-3.6" width="0.127" layer="21"/>
<wire x1="-7.5" y1="-3.6" x2="7.5" y2="-3.6" width="0.12" layer="51"/>
<wire x1="7.5" y1="-3.6" x2="7.5" y2="4" width="0.12" layer="51"/>
<wire x1="7.5" y1="4" x2="-7.5" y2="4" width="0.12" layer="51"/>
<wire x1="-7.5" y1="4" x2="-7.5" y2="-3.6" width="0.12" layer="51"/>
</package>
<package name="HDR6">
<text x="0" y="0" size="1" layer="25" align="center">&gt;NAME</text>
<pad name="1" x="-6.35" y="0" drill="0.9" diameter="1.6" shape="square"/>
<pad name="2" x="-3.81" y="0" drill="0.9" diameter="1.6"/>
<pad name="3" x="-1.27" y="0" drill="0.9" diameter="1.6"/>
<pad name="4" x="1.27" y="0" drill="0.9" diameter="1.6"/>
<pad name="5" x="3.81" y="0" drill="0.9" diameter="1.6"/>
<pad name="6" x="6.35" y="0" drill="0.9" diameter="1.6"/>
</package>
<package name="HDR10">
<text x="0" y="0" size="1" layer="25" align="center">&gt;NAME</text>
<pad name="1" x="-11.43" y="0" drill="0.9" diameter="1.6" shape="square"/>
<pad name="2" x="-8.89" y="0" drill="0.9" diameter="1.6"/>
<pad name="3" x="-6.35" y="0" drill="0.9" diameter="1.6"/>
<pad name="4" x="-3.81" y="0" drill="0.9" diameter="1.6"/>
<pad name="5" x="-1.27" y="0" drill="0.9" diameter="1.6"/>
<pad name="6" x="1.27" y="0" drill="0.9" diameter="1.6"/>
<pad name="7" x="3.81" y="0" drill="0.9" diameter="1.6"/>
<pad name="8" x="6.35" y="0" drill="0.9" diameter="1.6"/>
<pad name="9" x="8.89" y="0" drill="0.9" diameter="1.6"/>
<pad name="10" x="11.43" y="0" drill="0.9" diameter="1.6"/>
</package>
</packages>
<packages3d>
<package3d name="CAPC1608X85" urn="urn:adsk.eagle:package:16290898/6" type="model">
<packageinstances>
<packageinstance name="0603"/>
</packageinstances>
</package3d>
<package3d name="CAPC1005X60" urn="urn:adsk.eagle:package:16290895/6" type="model">
<packageinstances>
<packageinstance name="0402"/>
</packageinstances>
</package3d>
<package3d name="RESC1608X60" urn="urn:adsk.eagle:package:16378565/7" type="model">
<packageinstances>
<packageinstance name="0603"/>
</packageinstances>
</package3d>
<package3d name="CAPC2012X110" urn="urn:adsk.eagle:package:16290897/6" type="model">
<packageinstances>
<packageinstance name="0805"/>
</packageinstances>
</package3d>
<package3d name="CAPC3216X135" urn="urn:adsk.eagle:package:16290893/6" type="model">
<packageinstances>
<packageinstance name="1206"/>
</packageinstances>
</package3d>
<package3d name="CAPC3225X135" urn="urn:adsk.eagle:package:16290903/6" type="model">
<packageinstances>
<packageinstance name="1210"/>
</packageinstances>
</package3d>
<package3d name="CAPAE830X1050N" urn="urn:adsk.eagle:package:16290889/4" type="model">
<packageinstances>
<packageinstance name="CAP_V"/>
</packageinstances>
</package3d>
<package3d name="CAPMP7343X310N" urn="urn:adsk.eagle:package:16290885/4" type="model">
<packageinstances>
<packageinstance name="CAP_D"/>
</packageinstances>
</package3d>
<package3d name="FUSC3215X82N" urn="urn:adsk.eagle:package:16378157/3" type="model">
<packageinstances>
<packageinstance name="FUSENANO2"/>
</packageinstances>
</package3d>
<package3d name="1X10" urn="urn:adsk.eagle:package:51802615/2" type="model">
<packageinstances>
<packageinstance name="HDR10"/>
</packageinstances>
</package3d>
<package3d name="1X06" urn="urn:adsk.eagle:package:51802562/2" type="model">
<packageinstances>
<packageinstance name="HDR6"/>
</packageinstances>
</package3d>
<package3d name="INDM4532X340" urn="urn:adsk.eagle:package:16378474/4" type="model">
<packageinstances>
<packageinstance name="IND43"/>
</packageinstances>
</package3d>
<package3d name="LEDC1608X35N_FLAT-G" urn="urn:adsk.eagle:package:24294786/3" type="model">
<packageinstances>
<packageinstance name="LED0603"/>
</packageinstances>
</package3d>
<package3d name="TERMBLK_508-2N" urn="urn:adsk.eagle:package:51802550/2" type="model">
<packageinstances>
<packageinstance name="SCREW2"/>
</packageinstances>
</package3d>
<package3d name="TERMBLK_508-3N" urn="urn:adsk.eagle:package:51802625/2" type="model">
<packageinstances>
<packageinstance name="SCREW3"/>
</packageinstances>
</package3d>
<package3d name="SODFL4725X110" urn="urn:adsk.eagle:package:48498326/1" type="model">
<packageinstances>
<packageinstance name="SMA"/>
</packageinstances>
</package3d>
<package3d name="SODFL5336X110" urn="urn:adsk.eagle:package:48498325/1" type="model">
<packageinstances>
<packageinstance name="SMB"/>
</packageinstances>
</package3d>
<package3d name="SOD-123" urn="urn:adsk.eagle:package:10898395/2" type="model">
<packageinstances>
<packageinstance name="SOD123"/>
</packageinstances>
</package3d>
<package3d name="SOD-323" urn="urn:adsk.eagle:package:10898396/3" type="model">
<packageinstances>
<packageinstance name="SOD323"/>
</packageinstances>
</package3d>
<package3d name="SOT23" urn="urn:adsk.eagle:package:46660660/4" type="model">
<packageinstances>
<packageinstance name="SOT23"/>
</packageinstances>
</package3d>
<package3d name="SOT95P280X145-6N" urn="urn:adsk.eagle:package:18604126/4" type="model">
<packageinstances>
<packageinstance name="SOT236"/>
</packageinstances>
</package3d>
</packages3d>
<symbols>
<symbol name="RES">
<wire x1="-10.16" y1="-2.54" x2="10.16" y2="-2.54" width="0.254" layer="94"/>
<wire x1="10.16" y1="-2.54" x2="10.16" y2="2.54" width="0.254" layer="94"/>
<wire x1="10.16" y1="2.54" x2="-10.16" y2="2.54" width="0.254" layer="94"/>
<wire x1="-10.16" y1="2.54" x2="-10.16" y2="-2.54" width="0.254" layer="94"/>
<text x="-10.16" y="3.54" size="1.778" layer="95">&gt;NAME</text>
<text x="-10.16" y="-5.34" size="1.778" layer="96">&gt;VALUE</text>
<pin name="1" x="-12.7" y="0" visible="pin" length="point" direction="pas"/>
<pin name="2" x="12.7" y="0" visible="pin" length="point" direction="pas" rot="R180"/>
</symbol>
<symbol name="CAP">
<wire x1="-10.16" y1="-2.54" x2="10.16" y2="-2.54" width="0.254" layer="94"/>
<wire x1="10.16" y1="-2.54" x2="10.16" y2="2.54" width="0.254" layer="94"/>
<wire x1="10.16" y1="2.54" x2="-10.16" y2="2.54" width="0.254" layer="94"/>
<wire x1="-10.16" y1="2.54" x2="-10.16" y2="-2.54" width="0.254" layer="94"/>
<text x="-10.16" y="3.54" size="1.778" layer="95">&gt;NAME</text>
<text x="-10.16" y="-5.34" size="1.778" layer="96">&gt;VALUE</text>
<pin name="1" x="-12.7" y="0" visible="pin" length="point" direction="pas"/>
<pin name="2" x="12.7" y="0" visible="pin" length="point" direction="pas" rot="R180"/>
</symbol>
<symbol name="IND">
<wire x1="-10.16" y1="-2.54" x2="10.16" y2="-2.54" width="0.254" layer="94"/>
<wire x1="10.16" y1="-2.54" x2="10.16" y2="2.54" width="0.254" layer="94"/>
<wire x1="10.16" y1="2.54" x2="-10.16" y2="2.54" width="0.254" layer="94"/>
<wire x1="-10.16" y1="2.54" x2="-10.16" y2="-2.54" width="0.254" layer="94"/>
<text x="-10.16" y="3.54" size="1.778" layer="95">&gt;NAME</text>
<text x="-10.16" y="-5.34" size="1.778" layer="96">&gt;VALUE</text>
<pin name="1" x="-12.7" y="0" visible="pin" length="point" direction="pas"/>
<pin name="2" x="12.7" y="0" visible="pin" length="point" direction="pas" rot="R180"/>
</symbol>
<symbol name="FUSE">
<wire x1="-10.16" y1="-2.54" x2="10.16" y2="-2.54" width="0.254" layer="94"/>
<wire x1="10.16" y1="-2.54" x2="10.16" y2="2.54" width="0.254" layer="94"/>
<wire x1="10.16" y1="2.54" x2="-10.16" y2="2.54" width="0.254" layer="94"/>
<wire x1="-10.16" y1="2.54" x2="-10.16" y2="-2.54" width="0.254" layer="94"/>
<text x="-10.16" y="3.54" size="1.778" layer="95">&gt;NAME</text>
<text x="-10.16" y="-5.34" size="1.778" layer="96">&gt;VALUE</text>
<pin name="1" x="-12.7" y="0" visible="pin" length="point" direction="pas"/>
<pin name="2" x="12.7" y="0" visible="pin" length="point" direction="pas" rot="R180"/>
</symbol>
<symbol name="DIODE">
<wire x1="-10.16" y1="-2.54" x2="10.16" y2="-2.54" width="0.254" layer="94"/>
<wire x1="10.16" y1="-2.54" x2="10.16" y2="2.54" width="0.254" layer="94"/>
<wire x1="10.16" y1="2.54" x2="-10.16" y2="2.54" width="0.254" layer="94"/>
<wire x1="-10.16" y1="2.54" x2="-10.16" y2="-2.54" width="0.254" layer="94"/>
<text x="-10.16" y="3.54" size="1.778" layer="95">&gt;NAME</text>
<text x="-10.16" y="-5.34" size="1.778" layer="96">&gt;VALUE</text>
<pin name="A" x="-12.7" y="0" visible="pin" length="point" direction="pas"/>
<pin name="C" x="12.7" y="0" visible="pin" length="point" direction="pas" rot="R180"/>
</symbol>
<symbol name="FET">
<wire x1="-10.16" y1="-3.81" x2="10.16" y2="-3.81" width="0.254" layer="94"/>
<wire x1="10.16" y1="-3.81" x2="10.16" y2="3.81" width="0.254" layer="94"/>
<wire x1="10.16" y1="3.81" x2="-10.16" y2="3.81" width="0.254" layer="94"/>
<wire x1="-10.16" y1="3.81" x2="-10.16" y2="-3.81" width="0.254" layer="94"/>
<text x="-10.16" y="4.81" size="1.778" layer="95">&gt;NAME</text>
<text x="-10.16" y="-6.61" size="1.778" layer="96">&gt;VALUE</text>
<pin name="G" x="-12.7" y="1.27" visible="pin" length="point" direction="pas"/>
<pin name="D" x="-12.7" y="-1.27" visible="pin" length="point" direction="pas"/>
<pin name="S" x="12.7" y="0" visible="pin" length="point" direction="pas" rot="R180"/>
</symbol>
<symbol name="ESP32S3">
<wire x1="-10.16" y1="-16.51" x2="10.16" y2="-16.51" width="0.254" layer="94"/>
<wire x1="10.16" y1="-16.51" x2="10.16" y2="16.51" width="0.254" layer="94"/>
<wire x1="10.16" y1="16.51" x2="-10.16" y2="16.51" width="0.254" layer="94"/>
<wire x1="-10.16" y1="16.51" x2="-10.16" y2="-16.51" width="0.254" layer="94"/>
<text x="-10.16" y="17.51" size="1.778" layer="95">&gt;NAME</text>
<text x="-10.16" y="-19.31" size="1.778" layer="96">&gt;VALUE</text>
<pin name="3V3" x="-12.7" y="13.97" visible="pin" length="point" direction="pas"/>
<pin name="GND" x="-12.7" y="11.43" visible="pin" length="point" direction="pas"/>
<pin name="EN" x="-12.7" y="8.89" visible="pin" length="point" direction="pas"/>
<pin name="IO0" x="-12.7" y="6.35" visible="pin" length="point" direction="pas"/>
<pin name="IO4" x="-12.7" y="3.81" visible="pin" length="point" direction="pas"/>
<pin name="IO5" x="-12.7" y="1.27" visible="pin" length="point" direction="pas"/>
<pin name="IO6" x="-12.7" y="-1.27" visible="pin" length="point" direction="pas"/>
<pin name="IO7" x="-12.7" y="-3.81" visible="pin" length="point" direction="pas"/>
<pin name="IO8" x="-12.7" y="-6.35" visible="pin" length="point" direction="pas"/>
<pin name="IO9" x="-12.7" y="-8.89" visible="pin" length="point" direction="pas"/>
<pin name="IO10" x="-12.7" y="-11.43" visible="pin" length="point" direction="pas"/>
<pin name="IO11" x="-12.7" y="-13.97" visible="pin" length="point" direction="pas"/>
<pin name="IO12" x="12.7" y="12.7" visible="pin" length="point" direction="pas" rot="R180"/>
<pin name="IO14" x="12.7" y="10.16" visible="pin" length="point" direction="pas" rot="R180"/>
<pin name="IO15" x="12.7" y="7.62" visible="pin" length="point" direction="pas" rot="R180"/>
<pin name="IO16" x="12.7" y="5.08" visible="pin" length="point" direction="pas" rot="R180"/>
<pin name="IO17" x="12.7" y="2.54" visible="pin" length="point" direction="pas" rot="R180"/>
<pin name="IO18" x="12.7" y="0" visible="pin" length="point" direction="pas" rot="R180"/>
<pin name="IO19" x="12.7" y="-2.54" visible="pin" length="point" direction="pas" rot="R180"/>
<pin name="IO20" x="12.7" y="-5.08" visible="pin" length="point" direction="pas" rot="R180"/>
<pin name="IO21" x="12.7" y="-7.62" visible="pin" length="point" direction="pas" rot="R180"/>
<pin name="IO43" x="12.7" y="-10.16" visible="pin" length="point" direction="pas" rot="R180"/>
<pin name="IO44" x="12.7" y="-12.7" visible="pin" length="point" direction="pas" rot="R180"/>
</symbol>
<symbol name="USBC">
<wire x1="-10.16" y1="-5.08" x2="10.16" y2="-5.08" width="0.254" layer="94"/>
<wire x1="10.16" y1="-5.08" x2="10.16" y2="5.08" width="0.254" layer="94"/>
<wire x1="10.16" y1="5.08" x2="-10.16" y2="5.08" width="0.254" layer="94"/>
<wire x1="-10.16" y1="5.08" x2="-10.16" y2="-5.08" width="0.254" layer="94"/>
<text x="-10.16" y="6.08" size="1.778" layer="95">&gt;NAME</text>
<text x="-10.16" y="-7.88" size="1.778" layer="96">&gt;VALUE</text>
<pin name="VBUS" x="-12.7" y="2.54" visible="pin" length="point" direction="pas"/>
<pin name="GND" x="-12.7" y="0" visible="pin" length="point" direction="pas"/>
<pin name="CC1" x="-12.7" y="-2.54" visible="pin" length="point" direction="pas"/>
<pin name="CC2" x="12.7" y="2.54" visible="pin" length="point" direction="pas" rot="R180"/>
<pin name="DP" x="12.7" y="0" visible="pin" length="point" direction="pas" rot="R180"/>
<pin name="DM" x="12.7" y="-2.54" visible="pin" length="point" direction="pas" rot="R180"/>
</symbol>
<symbol name="USBLC6">
<wire x1="-10.16" y1="-5.08" x2="10.16" y2="-5.08" width="0.254" layer="94"/>
<wire x1="10.16" y1="-5.08" x2="10.16" y2="5.08" width="0.254" layer="94"/>
<wire x1="10.16" y1="5.08" x2="-10.16" y2="5.08" width="0.254" layer="94"/>
<wire x1="-10.16" y1="5.08" x2="-10.16" y2="-5.08" width="0.254" layer="94"/>
<text x="-10.16" y="6.08" size="1.778" layer="95">&gt;NAME</text>
<text x="-10.16" y="-7.88" size="1.778" layer="96">&gt;VALUE</text>
<pin name="DM_CH" x="-12.7" y="2.54" visible="pin" length="point" direction="pas"/>
<pin name="GND" x="-12.7" y="0" visible="pin" length="point" direction="pas"/>
<pin name="DM_CN" x="-12.7" y="-2.54" visible="pin" length="point" direction="pas"/>
<pin name="DP_CN" x="12.7" y="2.54" visible="pin" length="point" direction="pas" rot="R180"/>
<pin name="VBUS" x="12.7" y="0" visible="pin" length="point" direction="pas" rot="R180"/>
<pin name="DP_CH" x="12.7" y="-2.54" visible="pin" length="point" direction="pas" rot="R180"/>
</symbol>
<symbol name="DRV8313">
<wire x1="-10.16" y1="-11.43" x2="10.16" y2="-11.43" width="0.254" layer="94"/>
<wire x1="10.16" y1="-11.43" x2="10.16" y2="11.43" width="0.254" layer="94"/>
<wire x1="10.16" y1="11.43" x2="-10.16" y2="11.43" width="0.254" layer="94"/>
<wire x1="-10.16" y1="11.43" x2="-10.16" y2="-11.43" width="0.254" layer="94"/>
<text x="-10.16" y="12.43" size="1.778" layer="95">&gt;NAME</text>
<text x="-10.16" y="-14.23" size="1.778" layer="96">&gt;VALUE</text>
<pin name="VM" x="-12.7" y="8.89" visible="pin" length="point" direction="pas"/>
<pin name="GND" x="-12.7" y="6.35" visible="pin" length="point" direction="pas"/>
<pin name="OUT1" x="-12.7" y="3.81" visible="pin" length="point" direction="pas"/>
<pin name="OUT2" x="-12.7" y="1.27" visible="pin" length="point" direction="pas"/>
<pin name="OUT3" x="-12.7" y="-1.27" visible="pin" length="point" direction="pas"/>
<pin name="IN1" x="-12.7" y="-3.81" visible="pin" length="point" direction="pas"/>
<pin name="IN2" x="-12.7" y="-6.35" visible="pin" length="point" direction="pas"/>
<pin name="IN3" x="-12.7" y="-8.89" visible="pin" length="point" direction="pas"/>
<pin name="EN" x="12.7" y="8.89" visible="pin" length="point" direction="pas" rot="R180"/>
<pin name="NSLEEP" x="12.7" y="6.35" visible="pin" length="point" direction="pas" rot="R180"/>
<pin name="NRESET" x="12.7" y="3.81" visible="pin" length="point" direction="pas" rot="R180"/>
<pin name="NFAULT" x="12.7" y="1.27" visible="pin" length="point" direction="pas" rot="R180"/>
<pin name="V3P3" x="12.7" y="-1.27" visible="pin" length="point" direction="pas" rot="R180"/>
<pin name="VCP" x="12.7" y="-3.81" visible="pin" length="point" direction="pas" rot="R180"/>
<pin name="CPH" x="12.7" y="-6.35" visible="pin" length="point" direction="pas" rot="R180"/>
<pin name="CPL" x="12.7" y="-8.89" visible="pin" length="point" direction="pas" rot="R180"/>
</symbol>
<symbol name="BUCK">
<wire x1="-10.16" y1="-5.08" x2="10.16" y2="-5.08" width="0.254" layer="94"/>
<wire x1="10.16" y1="-5.08" x2="10.16" y2="5.08" width="0.254" layer="94"/>
<wire x1="10.16" y1="5.08" x2="-10.16" y2="5.08" width="0.254" layer="94"/>
<wire x1="-10.16" y1="5.08" x2="-10.16" y2="-5.08" width="0.254" layer="94"/>
<text x="-10.16" y="6.08" size="1.778" layer="95">&gt;NAME</text>
<text x="-10.16" y="-7.88" size="1.778" layer="96">&gt;VALUE</text>
<pin name="VIN" x="-12.7" y="2.54" visible="pin" length="point" direction="pas"/>
<pin name="GND" x="-12.7" y="0" visible="pin" length="point" direction="pas"/>
<pin name="EN" x="-12.7" y="-2.54" visible="pin" length="point" direction="pas"/>
<pin name="FB" x="12.7" y="2.54" visible="pin" length="point" direction="pas" rot="R180"/>
<pin name="SW" x="12.7" y="0" visible="pin" length="point" direction="pas" rot="R180"/>
<pin name="BST" x="12.7" y="-2.54" visible="pin" length="point" direction="pas" rot="R180"/>
</symbol>
<symbol name="CONN_PWR">
<wire x1="-10.16" y1="-2.54" x2="10.16" y2="-2.54" width="0.254" layer="94"/>
<wire x1="10.16" y1="-2.54" x2="10.16" y2="2.54" width="0.254" layer="94"/>
<wire x1="10.16" y1="2.54" x2="-10.16" y2="2.54" width="0.254" layer="94"/>
<wire x1="-10.16" y1="2.54" x2="-10.16" y2="-2.54" width="0.254" layer="94"/>
<text x="-10.16" y="3.54" size="1.778" layer="95">&gt;NAME</text>
<text x="-10.16" y="-5.34" size="1.778" layer="96">&gt;VALUE</text>
<pin name="VP" x="-12.7" y="0" visible="pin" length="point" direction="pas"/>
<pin name="VN" x="12.7" y="0" visible="pin" length="point" direction="pas" rot="R180"/>
</symbol>
<symbol name="CONN_MOTOR">
<wire x1="-10.16" y1="-3.81" x2="10.16" y2="-3.81" width="0.254" layer="94"/>
<wire x1="10.16" y1="-3.81" x2="10.16" y2="3.81" width="0.254" layer="94"/>
<wire x1="10.16" y1="3.81" x2="-10.16" y2="3.81" width="0.254" layer="94"/>
<wire x1="-10.16" y1="3.81" x2="-10.16" y2="-3.81" width="0.254" layer="94"/>
<text x="-10.16" y="4.81" size="1.778" layer="95">&gt;NAME</text>
<text x="-10.16" y="-6.61" size="1.778" layer="96">&gt;VALUE</text>
<pin name="U" x="-12.7" y="1.27" visible="pin" length="point" direction="pas"/>
<pin name="V" x="-12.7" y="-1.27" visible="pin" length="point" direction="pas"/>
<pin name="W" x="12.7" y="0" visible="pin" length="point" direction="pas" rot="R180"/>
</symbol>
<symbol name="CONN_PROG">
<wire x1="-10.16" y1="-5.08" x2="10.16" y2="-5.08" width="0.254" layer="94"/>
<wire x1="10.16" y1="-5.08" x2="10.16" y2="5.08" width="0.254" layer="94"/>
<wire x1="10.16" y1="5.08" x2="-10.16" y2="5.08" width="0.254" layer="94"/>
<wire x1="-10.16" y1="5.08" x2="-10.16" y2="-5.08" width="0.254" layer="94"/>
<text x="-10.16" y="6.08" size="1.778" layer="95">&gt;NAME</text>
<text x="-10.16" y="-7.88" size="1.778" layer="96">&gt;VALUE</text>
<pin name="3V3" x="-12.7" y="2.54" visible="pin" length="point" direction="pas"/>
<pin name="GND" x="-12.7" y="0" visible="pin" length="point" direction="pas"/>
<pin name="EN" x="-12.7" y="-2.54" visible="pin" length="point" direction="pas"/>
<pin name="IO0" x="12.7" y="2.54" visible="pin" length="point" direction="pas" rot="R180"/>
<pin name="TXD" x="12.7" y="0" visible="pin" length="point" direction="pas" rot="R180"/>
<pin name="RXD" x="12.7" y="-2.54" visible="pin" length="point" direction="pas" rot="R180"/>
</symbol>
<symbol name="CONN_EXP">
<wire x1="-10.16" y1="-7.62" x2="10.16" y2="-7.62" width="0.254" layer="94"/>
<wire x1="10.16" y1="-7.62" x2="10.16" y2="7.62" width="0.254" layer="94"/>
<wire x1="10.16" y1="7.62" x2="-10.16" y2="7.62" width="0.254" layer="94"/>
<wire x1="-10.16" y1="7.62" x2="-10.16" y2="-7.62" width="0.254" layer="94"/>
<text x="-10.16" y="8.62" size="1.778" layer="95">&gt;NAME</text>
<text x="-10.16" y="-10.42" size="1.778" layer="96">&gt;VALUE</text>
<pin name="3V3" x="-12.7" y="5.08" visible="pin" length="point" direction="pas"/>
<pin name="GND" x="-12.7" y="2.54" visible="pin" length="point" direction="pas"/>
<pin name="IO1" x="-12.7" y="0" visible="pin" length="point" direction="pas"/>
<pin name="IO2" x="-12.7" y="-2.54" visible="pin" length="point" direction="pas"/>
<pin name="IO13" x="-12.7" y="-5.08" visible="pin" length="point" direction="pas"/>
<pin name="IO38" x="12.7" y="5.08" visible="pin" length="point" direction="pas" rot="R180"/>
<pin name="IO39" x="12.7" y="2.54" visible="pin" length="point" direction="pas" rot="R180"/>
<pin name="IO40" x="12.7" y="0" visible="pin" length="point" direction="pas" rot="R180"/>
<pin name="IO41" x="12.7" y="-2.54" visible="pin" length="point" direction="pas" rot="R180"/>
<pin name="IO42" x="12.7" y="-5.08" visible="pin" length="point" direction="pas" rot="R180"/>
</symbol>
</symbols>
<devicesets>
<deviceset name="RES_0603" prefix="R" uservalue="yes">
<gates>
<gate name="G$1" symbol="RES" x="0" y="0"/>
</gates>
<devices>
<device name="" package="0603">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
</connects>

<package3dinstances>
<package3dinstance package3d_urn="urn:adsk.eagle:package:16378565/7"/>
</package3dinstances>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="CAP_0402" prefix="C" uservalue="yes">
<gates>
<gate name="G$1" symbol="CAP" x="0" y="0"/>
</gates>
<devices>
<device name="" package="0402">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
</connects>

<package3dinstances>
<package3dinstance package3d_urn="urn:adsk.eagle:package:16290895/6"/>
</package3dinstances>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="CAP_0603" prefix="C" uservalue="yes">
<gates>
<gate name="G$1" symbol="CAP" x="0" y="0"/>
</gates>
<devices>
<device name="" package="0603">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
</connects>

<package3dinstances>
<package3dinstance package3d_urn="urn:adsk.eagle:package:16290898/6"/>
</package3dinstances>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="CAP_0805" prefix="C" uservalue="yes">
<gates>
<gate name="G$1" symbol="CAP" x="0" y="0"/>
</gates>
<devices>
<device name="" package="0805">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
</connects>

<package3dinstances>
<package3dinstance package3d_urn="urn:adsk.eagle:package:16290897/6"/>
</package3dinstances>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="CAP_1206" prefix="C" uservalue="yes">
<gates>
<gate name="G$1" symbol="CAP" x="0" y="0"/>
</gates>
<devices>
<device name="" package="1206">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
</connects>

<package3dinstances>
<package3dinstance package3d_urn="urn:adsk.eagle:package:16290893/6"/>
</package3dinstances>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="CAP_1210" prefix="C" uservalue="yes">
<gates>
<gate name="G$1" symbol="CAP" x="0" y="0"/>
</gates>
<devices>
<device name="" package="1210">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
</connects>

<package3dinstances>
<package3dinstance package3d_urn="urn:adsk.eagle:package:16290903/6"/>
</package3dinstances>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="CAP_D" prefix="C" uservalue="yes">
<gates>
<gate name="G$1" symbol="CAP" x="0" y="0"/>
</gates>
<devices>
<device name="" package="CAP_D">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
</connects>

<package3dinstances>
<package3dinstance package3d_urn="urn:adsk.eagle:package:16290885/4"/>
</package3dinstances>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="CAP_V" prefix="C" uservalue="yes">
<gates>
<gate name="G$1" symbol="CAP" x="0" y="0"/>
</gates>
<devices>
<device name="" package="CAP_V">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
</connects>

<package3dinstances>
<package3dinstance package3d_urn="urn:adsk.eagle:package:16290889/4"/>
</package3dinstances>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="IND_SMD" prefix="L" uservalue="yes">
<gates>
<gate name="G$1" symbol="IND" x="0" y="0"/>
</gates>
<devices>
<device name="" package="IND43">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
</connects>

<package3dinstances>
<package3dinstance package3d_urn="urn:adsk.eagle:package:16378474/4"/>
</package3dinstances>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="FUSE_1206" prefix="F" uservalue="yes">
<gates>
<gate name="G$1" symbol="FUSE" x="0" y="0"/>
</gates>
<devices>
<device name="" package="FUSE1206">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="FUSE_NANO2" prefix="F" uservalue="yes">
<gates>
<gate name="G$1" symbol="FUSE" x="0" y="0"/>
</gates>
<devices>
<device name="" package="FUSENANO2">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
</connects>

<package3dinstances>
<package3dinstance package3d_urn="urn:adsk.eagle:package:16378157/3"/>
</package3dinstances>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="DIODE_SOD323" prefix="D" uservalue="yes">
<gates>
<gate name="G$1" symbol="DIODE" x="0" y="0"/>
</gates>
<devices>
<device name="" package="SOD323">
<connects>
<connect gate="G$1" pin="A" pad="A"/>
<connect gate="G$1" pin="C" pad="C"/>
</connects>

<package3dinstances>
<package3dinstance package3d_urn="urn:adsk.eagle:package:10898396/3"/>
</package3dinstances>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="DIODE_SOD123" prefix="D" uservalue="yes">
<gates>
<gate name="G$1" symbol="DIODE" x="0" y="0"/>
</gates>
<devices>
<device name="" package="SOD123">
<connects>
<connect gate="G$1" pin="A" pad="A"/>
<connect gate="G$1" pin="C" pad="C"/>
</connects>

<package3dinstances>
<package3dinstance package3d_urn="urn:adsk.eagle:package:10898395/2"/>
</package3dinstances>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="DIODE_SMA" prefix="D" uservalue="yes">
<gates>
<gate name="G$1" symbol="DIODE" x="0" y="0"/>
</gates>
<devices>
<device name="" package="SMA">
<connects>
<connect gate="G$1" pin="A" pad="A"/>
<connect gate="G$1" pin="C" pad="C"/>
</connects>

<package3dinstances>
<package3dinstance package3d_urn="urn:adsk.eagle:package:48498326/1"/>
</package3dinstances>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="DIODE_SMB" prefix="D" uservalue="yes">
<gates>
<gate name="G$1" symbol="DIODE" x="0" y="0"/>
</gates>
<devices>
<device name="" package="SMB">
<connects>
<connect gate="G$1" pin="A" pad="A"/>
<connect gate="G$1" pin="C" pad="C"/>
</connects>

<package3dinstances>
<package3dinstance package3d_urn="urn:adsk.eagle:package:48498325/1"/>
</package3dinstances>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="LED_0603" prefix="D" uservalue="yes">
<gates>
<gate name="G$1" symbol="DIODE" x="0" y="0"/>
</gates>
<devices>
<device name="" package="LED0603">
<connects>
<connect gate="G$1" pin="A" pad="A"/>
<connect gate="G$1" pin="C" pad="C"/>
</connects>

<package3dinstances>
<package3dinstance package3d_urn="urn:adsk.eagle:package:24294786/3"/>
</package3dinstances>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="FET_SOT23" prefix="Q" uservalue="yes">
<gates>
<gate name="G$1" symbol="FET" x="0" y="0"/>
</gates>
<devices>
<device name="" package="SOT23">
<connects>
<connect gate="G$1" pin="D" pad="3"/>
<connect gate="G$1" pin="G" pad="1"/>
<connect gate="G$1" pin="S" pad="2"/>
</connects>

<package3dinstances>
<package3dinstance package3d_urn="urn:adsk.eagle:package:46660660/4"/>
</package3dinstances>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="ESP32" prefix="U" uservalue="yes">
<gates>
<gate name="G$1" symbol="ESP32S3" x="0" y="0"/>
</gates>
<devices>
<device name="" package="WROOM1">
<connects>
<connect gate="G$1" pin="3V3" pad="2"/>
<connect gate="G$1" pin="EN" pad="3"/>
<connect gate="G$1" pin="GND" pad="1 40 41"/>
<connect gate="G$1" pin="IO0" pad="27"/>
<connect gate="G$1" pin="IO10" pad="18"/>
<connect gate="G$1" pin="IO11" pad="19"/>
<connect gate="G$1" pin="IO12" pad="20"/>
<connect gate="G$1" pin="IO14" pad="22"/>
<connect gate="G$1" pin="IO15" pad="8"/>
<connect gate="G$1" pin="IO16" pad="9"/>
<connect gate="G$1" pin="IO17" pad="10"/>
<connect gate="G$1" pin="IO18" pad="11"/>
<connect gate="G$1" pin="IO19" pad="13"/>
<connect gate="G$1" pin="IO20" pad="14"/>
<connect gate="G$1" pin="IO21" pad="23"/>
<connect gate="G$1" pin="IO4" pad="4"/>
<connect gate="G$1" pin="IO43" pad="37"/>
<connect gate="G$1" pin="IO44" pad="36"/>
<connect gate="G$1" pin="IO5" pad="5"/>
<connect gate="G$1" pin="IO6" pad="6"/>
<connect gate="G$1" pin="IO7" pad="7"/>
<connect gate="G$1" pin="IO8" pad="12"/>
<connect gate="G$1" pin="IO9" pad="17"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="USBLC_DS" prefix="D" uservalue="yes">
<gates>
<gate name="G$1" symbol="USBLC6" x="0" y="0"/>
</gates>
<devices>
<device name="" package="SOT236">
<connects>
<connect gate="G$1" pin="DM_CH" pad="1"/>
<connect gate="G$1" pin="DM_CN" pad="3"/>
<connect gate="G$1" pin="DP_CH" pad="6"/>
<connect gate="G$1" pin="DP_CN" pad="4"/>
<connect gate="G$1" pin="GND" pad="2"/>
<connect gate="G$1" pin="VBUS" pad="5"/>
</connects>

<package3dinstances>
<package3dinstance package3d_urn="urn:adsk.eagle:package:18604126/4"/>
</package3dinstances>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="BUCK_DS" prefix="U" uservalue="yes">
<gates>
<gate name="G$1" symbol="BUCK" x="0" y="0"/>
</gates>
<devices>
<device name="" package="SOT236">
<connects>
<connect gate="G$1" pin="BST" pad="6"/>
<connect gate="G$1" pin="EN" pad="5"/>
<connect gate="G$1" pin="FB" pad="4"/>
<connect gate="G$1" pin="GND" pad="1"/>
<connect gate="G$1" pin="SW" pad="2"/>
<connect gate="G$1" pin="VIN" pad="3"/>
</connects>

<package3dinstances>
<package3dinstance package3d_urn="urn:adsk.eagle:package:18604126/4"/>
</package3dinstances>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="DRV_DS" prefix="U" uservalue="yes">
<gates>
<gate name="G$1" symbol="DRV8313" x="0" y="0"/>
</gates>
<devices>
<device name="" package="HTSSOP28">
<connects>
<connect gate="G$1" pin="CPH" pad="2"/>
<connect gate="G$1" pin="CPL" pad="1"/>
<connect gate="G$1" pin="EN" pad="22 24 26"/>
<connect gate="G$1" pin="GND" pad="6 7 10 12 13 14 19 20 28 EP"/>
<connect gate="G$1" pin="IN1" pad="27"/>
<connect gate="G$1" pin="IN2" pad="25"/>
<connect gate="G$1" pin="IN3" pad="23"/>
<connect gate="G$1" pin="NFAULT" pad="18"/>
<connect gate="G$1" pin="NRESET" pad="16"/>
<connect gate="G$1" pin="NSLEEP" pad="17"/>
<connect gate="G$1" pin="OUT1" pad="5"/>
<connect gate="G$1" pin="OUT2" pad="8"/>
<connect gate="G$1" pin="OUT3" pad="9"/>
<connect gate="G$1" pin="V3P3" pad="15"/>
<connect gate="G$1" pin="VCP" pad="3"/>
<connect gate="G$1" pin="VM" pad="4 11"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="USBC_DS" prefix="J" uservalue="yes">
<gates>
<gate name="G$1" symbol="USBC" x="0" y="0"/>
</gates>
<devices>
<device name="" package="USBC16">
<connects>
<connect gate="G$1" pin="CC1" pad="CC1"/>
<connect gate="G$1" pin="CC2" pad="CC2"/>
<connect gate="G$1" pin="DM" pad="DM1 DM2"/>
<connect gate="G$1" pin="DP" pad="DP1 DP2"/>
<connect gate="G$1" pin="GND" pad="GND1 GND2 SH1 SH2"/>
<connect gate="G$1" pin="VBUS" pad="VBUS1 VBUS2"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="CONN_PWR_DS" prefix="J" uservalue="yes">
<gates>
<gate name="G$1" symbol="CONN_PWR" x="0" y="0"/>
</gates>
<devices>
<device name="" package="SCREW2">
<connects>
<connect gate="G$1" pin="VN" pad="2"/>
<connect gate="G$1" pin="VP" pad="1"/>
</connects>

<package3dinstances>
<package3dinstance package3d_urn="urn:adsk.eagle:package:51802550/2"/>
</package3dinstances>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="CONN_MOTOR_DS" prefix="J" uservalue="yes">
<gates>
<gate name="G$1" symbol="CONN_MOTOR" x="0" y="0"/>
</gates>
<devices>
<device name="" package="SCREW3">
<connects>
<connect gate="G$1" pin="U" pad="1"/>
<connect gate="G$1" pin="V" pad="2"/>
<connect gate="G$1" pin="W" pad="3"/>
</connects>

<package3dinstances>
<package3dinstance package3d_urn="urn:adsk.eagle:package:51802625/2"/>
</package3dinstances>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="CONN_PROG_DS" prefix="J" uservalue="yes">
<gates>
<gate name="G$1" symbol="CONN_PROG" x="0" y="0"/>
</gates>
<devices>
<device name="" package="HDR6">
<connects>
<connect gate="G$1" pin="3V3" pad="1"/>
<connect gate="G$1" pin="EN" pad="3"/>
<connect gate="G$1" pin="GND" pad="2"/>
<connect gate="G$1" pin="IO0" pad="4"/>
<connect gate="G$1" pin="RXD" pad="6"/>
<connect gate="G$1" pin="TXD" pad="5"/>
</connects>

<package3dinstances>
<package3dinstance package3d_urn="urn:adsk.eagle:package:51802562/2"/>
</package3dinstances>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="CONN_EXP_DS" prefix="J" uservalue="yes">
<gates>
<gate name="G$1" symbol="CONN_EXP" x="0" y="0"/>
</gates>
<devices>
<device name="" package="HDR10">
<connects>
<connect gate="G$1" pin="3V3" pad="1"/>
<connect gate="G$1" pin="GND" pad="2"/>
<connect gate="G$1" pin="IO1" pad="3"/>
<connect gate="G$1" pin="IO13" pad="5"/>
<connect gate="G$1" pin="IO2" pad="4"/>
<connect gate="G$1" pin="IO38" pad="6"/>
<connect gate="G$1" pin="IO39" pad="7"/>
<connect gate="G$1" pin="IO40" pad="8"/>
<connect gate="G$1" pin="IO41" pad="9"/>
<connect gate="G$1" pin="IO42" pad="10"/>
</connects>

<package3dinstances>
<package3dinstance package3d_urn="urn:adsk.eagle:package:51802615/2"/>
</package3dinstances>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
<library name="ESP32-S3-WROOM-1-N8R8" urn="urn:adsk.wipprod:fs.file:vf.hVdIrY1CQSyaVlBVWFbtZQ">
<packages>
<package name="XCVR_ESP32S3WROOM1N8R8">
<text x="-9.75" y="13.5" size="1.27" layer="25">&gt;NAME</text>
<text x="-9.75" y="-15" size="1.27" layer="27">&gt;VALUE</text>
<wire x1="-9" y1="12.75" x2="9" y2="12.75" width="0.127" layer="51"/>
<wire x1="9" y1="12.75" x2="9" y2="6.75" width="0.127" layer="51"/>
<wire x1="9" y1="6.75" x2="9" y2="-12.75" width="0.127" layer="51"/>
<wire x1="9" y1="-12.75" x2="-9" y2="-12.75" width="0.127" layer="51"/>
<wire x1="-9" y1="-12.75" x2="-9" y2="6.75" width="0.127" layer="51"/>
<wire x1="-9" y1="6.75" x2="-9" y2="12.75" width="0.127" layer="51"/>
<wire x1="-9" y1="6.75" x2="9" y2="6.75" width="0.127" layer="51"/>
<text x="-4.5" y="9.5" size="1.27" layer="51">ANTENNA</text>
<wire x1="-9.75" y1="-13.5" x2="-9.75" y2="13" width="0.05" layer="39"/>
<wire x1="-9.75" y1="13" x2="9.75" y2="13" width="0.05" layer="39"/>
<wire x1="9.75" y1="13" x2="9.75" y2="-13.5" width="0.05" layer="39"/>
<wire x1="-9.75" y1="-13.5" x2="9.75" y2="-13.5" width="0.05" layer="39"/>
<circle x="-10.2" y="5.26" radius="0.1" width="0.2" layer="51"/>
<wire x1="-9" y1="12.75" x2="9" y2="12.75" width="0.127" layer="21"/>
<wire x1="9" y1="12.75" x2="9" y2="6.03" width="0.127" layer="21"/>
<wire x1="-9" y1="6.03" x2="-9" y2="12.75" width="0.127" layer="21"/>
<circle x="-10.2" y="5.26" radius="0.1" width="0.2" layer="21"/>
<rectangle x1="-9" y1="6.75" x2="9" y2="12.75" layer="41"/>
<rectangle x1="-9" y1="6.75" x2="9" y2="12.75" layer="42"/>
<rectangle x1="-9" y1="6.75" x2="9" y2="12.75" layer="43"/>
<wire x1="-9" y1="-12.02" x2="-9" y2="-12.75" width="0.127" layer="21"/>
<wire x1="-9" y1="-12.75" x2="-7.755" y2="-12.75" width="0.127" layer="21"/>
<wire x1="9" y1="-12.02" x2="9" y2="-12.75" width="0.127" layer="21"/>
<wire x1="9" y1="-12.75" x2="7.755" y2="-12.75" width="0.127" layer="21"/>
<smd name="41_1" x="-1.5" y="-2.46" dx="0.9" dy="0.9" layer="1"/>
<smd name="1" x="-8.75" y="5.26" dx="1.5" dy="0.9" layer="1"/>
<smd name="41_2" x="-2.9" y="-2.46" dx="0.9" dy="0.9" layer="1"/>
<smd name="41_3" x="-0.1" y="-2.46" dx="0.9" dy="0.9" layer="1"/>
<smd name="41_4" x="-2.9" y="-3.86" dx="0.9" dy="0.9" layer="1"/>
<smd name="41_5" x="-1.5" y="-3.86" dx="0.9" dy="0.9" layer="1"/>
<smd name="41_6" x="-0.1" y="-3.86" dx="0.9" dy="0.9" layer="1"/>
<smd name="41_7" x="-2.9" y="-1.06" dx="0.9" dy="0.9" layer="1"/>
<smd name="41_8" x="-1.5" y="-1.06" dx="0.9" dy="0.9" layer="1"/>
<smd name="41_9" x="-0.1" y="-1.06" dx="0.9" dy="0.9" layer="1"/>
<pad name="41_10" x="-1.5" y="-1.76" drill="0.2" diameter="0.4" stop="no"/>
<pad name="41_11" x="-2.9" y="-1.76" drill="0.2" diameter="0.4" stop="no"/>
<pad name="41_12" x="-0.1" y="-1.76" drill="0.2" diameter="0.4" stop="no"/>
<pad name="41_13" x="-2.9" y="-3.16" drill="0.2" diameter="0.4" stop="no"/>
<pad name="41_14" x="-1.5" y="-3.16" drill="0.2" diameter="0.4" stop="no"/>
<pad name="41_15" x="-0.1" y="-3.16" drill="0.2" diameter="0.4" stop="no"/>
<pad name="41_16" x="-2.2" y="-1.06" drill="0.2" diameter="0.4" stop="no"/>
<pad name="41_17" x="-0.8" y="-1.06" drill="0.2" diameter="0.4" stop="no"/>
<pad name="41_18" x="-2.2" y="-2.46" drill="0.2" diameter="0.4" stop="no"/>
<pad name="41_19" x="-0.8" y="-2.46" drill="0.2" diameter="0.4" stop="no"/>
<pad name="41_20" x="-2.2" y="-3.86" drill="0.2" diameter="0.4" stop="no"/>
<pad name="41_21" x="-0.8" y="-3.86" drill="0.2" diameter="0.4" stop="no"/>
<smd name="2" x="-8.75" y="3.99" dx="1.5" dy="0.9" layer="1"/>
<smd name="3" x="-8.75" y="2.72" dx="1.5" dy="0.9" layer="1"/>
<smd name="4" x="-8.75" y="1.45" dx="1.5" dy="0.9" layer="1"/>
<smd name="5" x="-8.75" y="0.18" dx="1.5" dy="0.9" layer="1"/>
<smd name="6" x="-8.75" y="-1.09" dx="1.5" dy="0.9" layer="1"/>
<smd name="7" x="-8.75" y="-2.36" dx="1.5" dy="0.9" layer="1"/>
<smd name="8" x="-8.75" y="-3.63" dx="1.5" dy="0.9" layer="1"/>
<smd name="9" x="-8.75" y="-4.9" dx="1.5" dy="0.9" layer="1"/>
<smd name="10" x="-8.75" y="-6.17" dx="1.5" dy="0.9" layer="1"/>
<smd name="11" x="-8.75" y="-7.44" dx="1.5" dy="0.9" layer="1"/>
<smd name="12" x="-8.75" y="-8.71" dx="1.5" dy="0.9" layer="1"/>
<smd name="13" x="-8.75" y="-9.98" dx="1.5" dy="0.9" layer="1"/>
<smd name="14" x="-8.75" y="-11.25" dx="1.5" dy="0.9" layer="1"/>
<smd name="15" x="-6.985" y="-12.5" dx="0.9" dy="1.5" layer="1"/>
<smd name="27" x="8.75" y="-11.25" dx="1.5" dy="0.9" layer="1"/>
<smd name="28" x="8.75" y="-9.98" dx="1.5" dy="0.9" layer="1"/>
<smd name="29" x="8.75" y="-8.71" dx="1.5" dy="0.9" layer="1"/>
<smd name="30" x="8.75" y="-7.44" dx="1.5" dy="0.9" layer="1"/>
<smd name="31" x="8.75" y="-6.17" dx="1.5" dy="0.9" layer="1"/>
<smd name="32" x="8.75" y="-4.9" dx="1.5" dy="0.9" layer="1"/>
<smd name="33" x="8.75" y="-3.63" dx="1.5" dy="0.9" layer="1"/>
<smd name="34" x="8.75" y="-2.36" dx="1.5" dy="0.9" layer="1"/>
<smd name="35" x="8.75" y="-1.09" dx="1.5" dy="0.9" layer="1"/>
<smd name="36" x="8.75" y="0.18" dx="1.5" dy="0.9" layer="1"/>
<smd name="37" x="8.75" y="1.45" dx="1.5" dy="0.9" layer="1"/>
<smd name="38" x="8.75" y="2.72" dx="1.5" dy="0.9" layer="1"/>
<smd name="39" x="8.75" y="3.99" dx="1.5" dy="0.9" layer="1"/>
<smd name="40" x="8.75" y="5.26" dx="1.5" dy="0.9" layer="1"/>
<smd name="16" x="-5.715" y="-12.5" dx="0.9" dy="1.5" layer="1"/>
<smd name="17" x="-4.445" y="-12.5" dx="0.9" dy="1.5" layer="1"/>
<smd name="18" x="-3.175" y="-12.5" dx="0.9" dy="1.5" layer="1"/>
<smd name="19" x="-1.905" y="-12.5" dx="0.9" dy="1.5" layer="1"/>
<smd name="20" x="-0.635" y="-12.5" dx="0.9" dy="1.5" layer="1"/>
<smd name="21" x="0.635" y="-12.5" dx="0.9" dy="1.5" layer="1"/>
<smd name="22" x="1.905" y="-12.5" dx="0.9" dy="1.5" layer="1"/>
<smd name="23" x="3.175" y="-12.5" dx="0.9" dy="1.5" layer="1"/>
<smd name="24" x="4.445" y="-12.5" dx="0.9" dy="1.5" layer="1"/>
<smd name="25" x="5.715" y="-12.5" dx="0.9" dy="1.5" layer="1"/>
<smd name="26" x="6.985" y="-12.5" dx="0.9" dy="1.5" layer="1"/>
</package>
</packages>
<packages3d>
<package3d name="ESP32-S3-WROOM-1-N8R8" urn="" wip_urn="urn:adsk.wipprod:fs.file:vf.dXc3RDZRTwmGOH6vqNPrOw?version=2" locally_modified="yes" type="model">
<packageinstances>
<packageinstance name="XCVR_ESP32S3WROOM1N8R8"/>
</packageinstances>
</package3d>
</packages3d>
<symbols>
<symbol name="ESP32-S3-WROOM-1-N8R8">
<wire x1="-10.16" y1="33.02" x2="10.16" y2="33.02" width="0.254" layer="94"/>
<wire x1="10.16" y1="33.02" x2="10.16" y2="-33.02" width="0.254" layer="94"/>
<wire x1="10.16" y1="-33.02" x2="-10.16" y2="-33.02" width="0.254" layer="94"/>
<wire x1="-10.16" y1="-33.02" x2="-10.16" y2="33.02" width="0.254" layer="94"/>
<text x="-10.16" y="34.1122" size="1.778" layer="95">&gt;NAME</text>
<text x="-10.16" y="-35.56" size="1.778" layer="96">&gt;VALUE</text>
<pin name="GND" x="15.24" y="-30.48" length="middle" direction="pwr" rot="R180"/>
<pin name="3V3" x="15.24" y="30.48" length="middle" direction="pwr" rot="R180"/>
<pin name="EN" x="-15.24" y="27.94" length="middle" direction="in"/>
<pin name="IO35" x="15.24" y="2.54" length="middle" rot="R180"/>
<pin name="IO41" x="15.24" y="-12.7" length="middle" rot="R180"/>
<pin name="IO39" x="15.24" y="-7.62" length="middle" rot="R180"/>
<pin name="IO40" x="15.24" y="-10.16" length="middle" rot="R180"/>
<pin name="IO14" x="-15.24" y="-20.32" length="middle"/>
<pin name="IO12" x="-15.24" y="-15.24" length="middle"/>
<pin name="IO13" x="-15.24" y="-17.78" length="middle"/>
<pin name="IO15" x="-15.24" y="-22.86" length="middle"/>
<pin name="IO2" x="-15.24" y="10.16" length="middle"/>
<pin name="IO0" x="-15.24" y="15.24" length="middle"/>
<pin name="IO4" x="-15.24" y="5.08" length="middle"/>
<pin name="IO16" x="-15.24" y="-25.4" length="middle"/>
<pin name="IO17" x="15.24" y="15.24" length="middle" rot="R180"/>
<pin name="IO5" x="-15.24" y="2.54" length="middle"/>
<pin name="IO18" x="15.24" y="12.7" length="middle" rot="R180"/>
<pin name="IO19" x="15.24" y="10.16" length="middle" rot="R180"/>
<pin name="IO21" x="15.24" y="5.08" length="middle" rot="R180"/>
<pin name="IO37" x="15.24" y="-2.54" length="middle" rot="R180"/>
<pin name="IO38" x="15.24" y="-5.08" length="middle" rot="R180"/>
<pin name="IO1" x="-15.24" y="12.7" length="middle"/>
<pin name="IO3" x="-15.24" y="7.62" length="middle"/>
<pin name="IO6" x="-15.24" y="0" length="middle"/>
<pin name="IO7" x="-15.24" y="-2.54" length="middle"/>
<pin name="IO8" x="-15.24" y="-5.08" length="middle"/>
<pin name="IO9" x="-15.24" y="-7.62" length="middle"/>
<pin name="IO10" x="-15.24" y="-10.16" length="middle"/>
<pin name="IO11" x="-15.24" y="-12.7" length="middle"/>
<pin name="IO36" x="15.24" y="0" length="middle" rot="R180"/>
<pin name="IO42" x="15.24" y="-15.24" length="middle" rot="R180"/>
<pin name="IO20" x="15.24" y="7.62" length="middle" rot="R180"/>
<pin name="TXD0" x="-15.24" y="20.32" length="middle"/>
<pin name="RXD0" x="-15.24" y="22.86" length="middle"/>
<pin name="IO45" x="15.24" y="-17.78" length="middle" rot="R180"/>
<pin name="IO46" x="15.24" y="-20.32" length="middle" rot="R180"/>
<pin name="IO47" x="15.24" y="-22.86" length="middle" rot="R180"/>
<pin name="IO48" x="15.24" y="-25.4" length="middle" rot="R180"/>
</symbol>
</symbols>
<devicesets>
<deviceset name="ESP32-S3-WROOM-1-N8R8" prefix="U">
<description> &lt;a href="https://pricing.snapeda.com/parts/ESP32-S3-WROOM-1-N8R8/Espressif%20Systems/view-part?ref=eda"&gt;Check availability&lt;/a&gt;</description>
<gates>
<gate name="G$1" symbol="ESP32-S3-WROOM-1-N8R8" x="0" y="0"/>
</gates>
<devices>
<device name="" package="XCVR_ESP32S3WROOM1N8R8">
<connects>
<connect gate="G$1" pin="3V3" pad="2"/>
<connect gate="G$1" pin="EN" pad="3"/>
<connect gate="G$1" pin="GND" pad="1 40 41_1 41_2 41_3 41_4 41_5 41_6 41_7 41_8 41_9"/>
<connect gate="G$1" pin="IO0" pad="27"/>
<connect gate="G$1" pin="IO1" pad="39"/>
<connect gate="G$1" pin="IO10" pad="18"/>
<connect gate="G$1" pin="IO11" pad="19"/>
<connect gate="G$1" pin="IO12" pad="20"/>
<connect gate="G$1" pin="IO13" pad="21"/>
<connect gate="G$1" pin="IO14" pad="22"/>
<connect gate="G$1" pin="IO15" pad="8"/>
<connect gate="G$1" pin="IO16" pad="9"/>
<connect gate="G$1" pin="IO17" pad="10"/>
<connect gate="G$1" pin="IO18" pad="11"/>
<connect gate="G$1" pin="IO19" pad="13"/>
<connect gate="G$1" pin="IO2" pad="38"/>
<connect gate="G$1" pin="IO20" pad="14"/>
<connect gate="G$1" pin="IO21" pad="23"/>
<connect gate="G$1" pin="IO3" pad="15"/>
<connect gate="G$1" pin="IO35" pad="28"/>
<connect gate="G$1" pin="IO36" pad="29"/>
<connect gate="G$1" pin="IO37" pad="30"/>
<connect gate="G$1" pin="IO38" pad="31"/>
<connect gate="G$1" pin="IO39" pad="32"/>
<connect gate="G$1" pin="IO4" pad="4"/>
<connect gate="G$1" pin="IO40" pad="33"/>
<connect gate="G$1" pin="IO41" pad="34"/>
<connect gate="G$1" pin="IO42" pad="35"/>
<connect gate="G$1" pin="IO45" pad="26"/>
<connect gate="G$1" pin="IO46" pad="16"/>
<connect gate="G$1" pin="IO47" pad="24"/>
<connect gate="G$1" pin="IO48" pad="25"/>
<connect gate="G$1" pin="IO5" pad="5"/>
<connect gate="G$1" pin="IO6" pad="6"/>
<connect gate="G$1" pin="IO7" pad="7"/>
<connect gate="G$1" pin="IO8" pad="12"/>
<connect gate="G$1" pin="IO9" pad="17"/>
<connect gate="G$1" pin="RXD0" pad="36"/>
<connect gate="G$1" pin="TXD0" pad="37"/>
</connects>

<package3dinstances>
<package3dinstance package3d_urn="urn:adsk.wipprod:fs.file:vf.dXc3RDZRTwmGOH6vqNPrOw?version=2"/>
</package3dinstances>
<technologies>
<technology name="">
<attribute name="MF" value="Espressif Systems"/>
<attribute name="DESCRIPTION" value="                                                      Bluetooth, WiFi 802.11b/g/n, Bluetooth v5.0 Transceiver Module 2.4GHz PCB Trace Surface Mount                                              "/>
<attribute name="PACKAGE" value="NON STANDARD Espressif Systems"/>
<attribute name="PRICE" value="None"/>
<attribute name="CHECK_PRICES" value="https://www.snapeda.com/parts/ESP32-S3-WROOM-1-N8R8/Espressif+Systems/view-part/?ref=eda"/>
<attribute name="SNAPEDA_LINK" value="https://www.snapeda.com/parts/ESP32-S3-WROOM-1-N8R8/Espressif+Systems/view-part/?ref=snap"/>
<attribute name="MP" value="ESP32-S3-WROOM-1-N8R8"/>
<attribute name="AVAILABILITY" value="In Stock"/>
<attribute name="PURCHASE-URL" value="https://www.snapeda.com/api/url_track_click_mouser/?unipart_id=8936581&amp;manufacturer=Espressif Systems&amp;part_name=ESP32-S3-WROOM-1-N8R8&amp;search_term= esp32-s3-wroom-1-n8r8"/>
</technology>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
<library name="DRV8313PWPR" urn="urn:adsk.wipprod:fs.file:vf.Qd50y5P0SAeBl-SYBauaqQ">
<packages>
<package name="SOP65P640X120-29N">
<wire x1="-2.2" y1="4.85" x2="2.2" y2="4.85" width="0.127" layer="51"/>
<wire x1="-2.2" y1="-4.85" x2="2.2" y2="-4.85" width="0.127" layer="51"/>
<wire x1="-2.2" y1="4.85" x2="-2.2" y2="-4.85" width="0.127" layer="51"/>
<wire x1="2.2" y1="4.85" x2="2.2" y2="-4.85" width="0.127" layer="51"/>
<text x="-16.435" y="-8.555" size="0.8128" layer="51">Note: Paste mask of Power Pad is based on 0.125 mm stencil thickness</text>
<polygon width="0.01" layer="29" pour="solid">
<vertex x="-3.7" y="4.025"/>
<vertex x="-2.1" y="4.025"/>
<vertex x="-2.1" y="4.425"/>
<vertex x="-3.7" y="4.425"/>
</polygon>
<polygon width="0.01" layer="29" pour="solid">
<vertex x="-3.7" y="3.375"/>
<vertex x="-2.1" y="3.375"/>
<vertex x="-2.1" y="3.775"/>
<vertex x="-3.7" y="3.775"/>
</polygon>
<polygon width="0.01" layer="29" pour="solid">
<vertex x="-3.7" y="2.725"/>
<vertex x="-2.1" y="2.725"/>
<vertex x="-2.1" y="3.125"/>
<vertex x="-3.7" y="3.125"/>
</polygon>
<polygon width="0.01" layer="29" pour="solid">
<vertex x="-3.7" y="2.075"/>
<vertex x="-2.1" y="2.075"/>
<vertex x="-2.1" y="2.475"/>
<vertex x="-3.7" y="2.475"/>
</polygon>
<polygon width="0.01" layer="29" pour="solid">
<vertex x="-3.7" y="1.425"/>
<vertex x="-2.1" y="1.425"/>
<vertex x="-2.1" y="1.825"/>
<vertex x="-3.7" y="1.825"/>
</polygon>
<polygon width="0.01" layer="29" pour="solid">
<vertex x="-3.7" y="0.775"/>
<vertex x="-2.1" y="0.775"/>
<vertex x="-2.1" y="1.175"/>
<vertex x="-3.7" y="1.175"/>
</polygon>
<polygon width="0.01" layer="29" pour="solid">
<vertex x="-3.7" y="0.125"/>
<vertex x="-2.1" y="0.125"/>
<vertex x="-2.1" y="0.525"/>
<vertex x="-3.7" y="0.525"/>
</polygon>
<polygon width="0.01" layer="29" pour="solid">
<vertex x="-3.7" y="-0.525"/>
<vertex x="-2.1" y="-0.525"/>
<vertex x="-2.1" y="-0.125"/>
<vertex x="-3.7" y="-0.125"/>
</polygon>
<polygon width="0.01" layer="29" pour="solid">
<vertex x="-3.7" y="-1.175"/>
<vertex x="-2.1" y="-1.175"/>
<vertex x="-2.1" y="-0.775"/>
<vertex x="-3.7" y="-0.775"/>
</polygon>
<polygon width="0.01" layer="29" pour="solid">
<vertex x="-3.7" y="-1.825"/>
<vertex x="-2.1" y="-1.825"/>
<vertex x="-2.1" y="-1.425"/>
<vertex x="-3.7" y="-1.425"/>
</polygon>
<polygon width="0.01" layer="29" pour="solid">
<vertex x="-3.7" y="-2.475"/>
<vertex x="-2.1" y="-2.475"/>
<vertex x="-2.1" y="-2.075"/>
<vertex x="-3.7" y="-2.075"/>
</polygon>
<polygon width="0.01" layer="29" pour="solid">
<vertex x="-3.7" y="-3.125"/>
<vertex x="-2.1" y="-3.125"/>
<vertex x="-2.1" y="-2.725"/>
<vertex x="-3.7" y="-2.725"/>
</polygon>
<polygon width="0.01" layer="29" pour="solid">
<vertex x="-3.7" y="-3.775"/>
<vertex x="-2.1" y="-3.775"/>
<vertex x="-2.1" y="-3.375"/>
<vertex x="-3.7" y="-3.375"/>
</polygon>
<polygon width="0.01" layer="29" pour="solid">
<vertex x="-3.7" y="-4.425"/>
<vertex x="-2.1" y="-4.425"/>
<vertex x="-2.1" y="-4.025"/>
<vertex x="-3.7" y="-4.025"/>
</polygon>
<polygon width="0.01" layer="29" pour="solid">
<vertex x="2.1" y="4.025"/>
<vertex x="3.7" y="4.025"/>
<vertex x="3.7" y="4.425"/>
<vertex x="2.1" y="4.425"/>
</polygon>
<polygon width="0.01" layer="29" pour="solid">
<vertex x="2.1" y="3.375"/>
<vertex x="3.7" y="3.375"/>
<vertex x="3.7" y="3.775"/>
<vertex x="2.1" y="3.775"/>
</polygon>
<polygon width="0.01" layer="29" pour="solid">
<vertex x="2.1" y="2.725"/>
<vertex x="3.7" y="2.725"/>
<vertex x="3.7" y="3.125"/>
<vertex x="2.1" y="3.125"/>
</polygon>
<polygon width="0.01" layer="29" pour="solid">
<vertex x="2.1" y="2.075"/>
<vertex x="3.7" y="2.075"/>
<vertex x="3.7" y="2.475"/>
<vertex x="2.1" y="2.475"/>
</polygon>
<polygon width="0.01" layer="29" pour="solid">
<vertex x="2.1" y="1.425"/>
<vertex x="3.7" y="1.425"/>
<vertex x="3.7" y="1.825"/>
<vertex x="2.1" y="1.825"/>
</polygon>
<polygon width="0.01" layer="29" pour="solid">
<vertex x="2.1" y="0.775"/>
<vertex x="3.7" y="0.775"/>
<vertex x="3.7" y="1.175"/>
<vertex x="2.1" y="1.175"/>
</polygon>
<polygon width="0.01" layer="29" pour="solid">
<vertex x="2.1" y="0.125"/>
<vertex x="3.7" y="0.125"/>
<vertex x="3.7" y="0.525"/>
<vertex x="2.1" y="0.525"/>
</polygon>
<polygon width="0.01" layer="29" pour="solid">
<vertex x="2.1" y="-0.525"/>
<vertex x="3.7" y="-0.525"/>
<vertex x="3.7" y="-0.125"/>
<vertex x="2.1" y="-0.125"/>
</polygon>
<polygon width="0.01" layer="29" pour="solid">
<vertex x="2.1" y="-1.175"/>
<vertex x="3.7" y="-1.175"/>
<vertex x="3.7" y="-0.775"/>
<vertex x="2.1" y="-0.775"/>
</polygon>
<polygon width="0.01" layer="29" pour="solid">
<vertex x="2.1" y="-1.825"/>
<vertex x="3.7" y="-1.825"/>
<vertex x="3.7" y="-1.425"/>
<vertex x="2.1" y="-1.425"/>
</polygon>
<polygon width="0.01" layer="29" pour="solid">
<vertex x="2.1" y="-2.475"/>
<vertex x="3.7" y="-2.475"/>
<vertex x="3.7" y="-2.075"/>
<vertex x="2.1" y="-2.075"/>
</polygon>
<polygon width="0.01" layer="29" pour="solid">
<vertex x="2.1" y="-3.125"/>
<vertex x="3.7" y="-3.125"/>
<vertex x="3.7" y="-2.725"/>
<vertex x="2.1" y="-2.725"/>
</polygon>
<polygon width="0.01" layer="29" pour="solid">
<vertex x="2.1" y="-3.775"/>
<vertex x="3.7" y="-3.775"/>
<vertex x="3.7" y="-3.375"/>
<vertex x="2.1" y="-3.375"/>
</polygon>
<polygon width="0.01" layer="29" pour="solid">
<vertex x="2.1" y="-4.425"/>
<vertex x="3.7" y="-4.425"/>
<vertex x="3.7" y="-4.025"/>
<vertex x="2.1" y="-4.025"/>
</polygon>
<polygon width="0.01" layer="31" pour="solid">
<vertex x="-1.4999875" y="-2.59"/>
<vertex x="1.4999875" y="-2.59"/>
<vertex x="1.51915" y="-2.586175"/>
<vertex x="1.53535625" y="-2.57535625"/>
<vertex x="1.546175" y="-2.55915"/>
<vertex x="1.55" y="-2.5399875"/>
<vertex x="1.55" y="2.5399875"/>
<vertex x="1.546175" y="2.55915"/>
<vertex x="1.53535625" y="2.57535625"/>
<vertex x="1.51915" y="2.586175"/>
<vertex x="1.4999875" y="2.59"/>
<vertex x="-1.4999875" y="2.59"/>
<vertex x="-1.51915" y="2.586175"/>
<vertex x="-1.53535625" y="2.57535625"/>
<vertex x="-1.546175" y="2.55915"/>
<vertex x="-1.55" y="2.5399875"/>
<vertex x="-1.55" y="-2.5399875"/>
<vertex x="-1.546175" y="-2.55915"/>
<vertex x="-1.53535625" y="-2.57535625"/>
<vertex x="-1.51915" y="-2.586175"/>
</polygon>
<polygon width="0.01" layer="29" pour="solid">
<vertex x="-1.6999875" y="-4.9"/>
<vertex x="1.6999875" y="-4.9"/>
<vertex x="1.71915" y="-4.896175"/>
<vertex x="1.73535625" y="-4.88535625"/>
<vertex x="1.746175" y="-4.86915"/>
<vertex x="1.75" y="-4.8499875"/>
<vertex x="1.75" y="4.8499875"/>
<vertex x="1.746175" y="4.86915"/>
<vertex x="1.73535625" y="4.88535625"/>
<vertex x="1.71915" y="4.896175"/>
<vertex x="1.6999875" y="4.9"/>
<vertex x="-1.6999875" y="4.9"/>
<vertex x="-1.71915" y="4.896175"/>
<vertex x="-1.73535625" y="4.88535625"/>
<vertex x="-1.746175" y="4.86915"/>
<vertex x="-1.75" y="4.8499875"/>
<vertex x="-1.75" y="-4.8499875"/>
<vertex x="-1.746175" y="-4.86915"/>
<vertex x="-1.73535625" y="-4.88535625"/>
<vertex x="-1.71915" y="-4.896175"/>
</polygon>
<polygon width="0.01" layer="1" pour="solid">
<vertex x="-1.6499875" y="-4.85"/>
<vertex x="1.6499875" y="-4.85"/>
<vertex x="1.66915" y="-4.846175"/>
<vertex x="1.68535625" y="-4.83535625"/>
<vertex x="1.696175" y="-4.81915"/>
<vertex x="1.7" y="-4.7999875"/>
<vertex x="1.7" y="4.7999875"/>
<vertex x="1.696175" y="4.81915"/>
<vertex x="1.68535625" y="4.83535625"/>
<vertex x="1.66915" y="4.846175"/>
<vertex x="1.6499875" y="4.85"/>
<vertex x="-1.6499875" y="4.85"/>
<vertex x="-1.66915" y="4.846175"/>
<vertex x="-1.68535625" y="4.83535625"/>
<vertex x="-1.696175" y="4.81915"/>
<vertex x="-1.7" y="4.7999875"/>
<vertex x="-1.7" y="-4.7999875"/>
<vertex x="-1.696175" y="-4.81915"/>
<vertex x="-1.68535625" y="-4.83535625"/>
<vertex x="-1.66915" y="-4.846175"/>
</polygon>
<wire x1="-2.2" y1="4.85" x2="-2.2" y2="4.77" width="0.127" layer="21"/>
<wire x1="-2.2" y1="-4.85" x2="-2.2" y2="-4.77" width="0.127" layer="21"/>
<wire x1="2.2" y1="4.85" x2="2.2" y2="4.77" width="0.127" layer="21"/>
<wire x1="2.2" y1="-4.85" x2="2.2" y2="-4.77" width="0.127" layer="21"/>
<wire x1="-2.2" y1="4.85" x2="-2.02" y2="4.85" width="0.127" layer="21"/>
<wire x1="2.02" y1="4.85" x2="2.2" y2="4.85" width="0.127" layer="21"/>
<wire x1="-2.2" y1="-4.85" x2="-2.02" y2="-4.85" width="0.127" layer="21"/>
<wire x1="2.02" y1="-4.85" x2="2.2" y2="-4.85" width="0.127" layer="21"/>
<wire x1="-3.9" y1="5.1" x2="-3.9" y2="-5.1" width="0.05" layer="39"/>
<wire x1="-3.9" y1="-5.1" x2="3.9" y2="-5.1" width="0.05" layer="39"/>
<wire x1="3.9" y1="-5.1" x2="3.9" y2="5.1" width="0.05" layer="39"/>
<wire x1="3.9" y1="5.1" x2="-3.9" y2="5.1" width="0.05" layer="39"/>
<circle x="-4.65" y="4.225" radius="0.1" width="0.2" layer="21"/>
<circle x="-4.65" y="4.225" radius="0.1" width="0.2" layer="51"/>
<text x="-3.9" y="6.1" size="1.27" layer="25">&gt;NAME</text>
<text x="-3.9" y="-6.1" size="1.27" layer="27" align="top-left">&gt;VALUE</text>
<smd name="1" x="-2.9" y="4.225" dx="1.5" dy="0.45" layer="1" roundness="20" stop="no"/>
<smd name="2" x="-2.9" y="3.575" dx="1.5" dy="0.45" layer="1" roundness="20" stop="no"/>
<smd name="3" x="-2.9" y="2.925" dx="1.5" dy="0.45" layer="1" roundness="20" stop="no"/>
<smd name="4" x="-2.9" y="2.275" dx="1.5" dy="0.45" layer="1" roundness="20" stop="no"/>
<smd name="5" x="-2.9" y="1.625" dx="1.5" dy="0.45" layer="1" roundness="20" stop="no"/>
<smd name="6" x="-2.9" y="0.975" dx="1.5" dy="0.45" layer="1" roundness="20" stop="no"/>
<smd name="7" x="-2.9" y="0.325" dx="1.5" dy="0.45" layer="1" roundness="20" stop="no"/>
<smd name="8" x="-2.9" y="-0.325" dx="1.5" dy="0.45" layer="1" roundness="20" stop="no"/>
<smd name="9" x="-2.9" y="-0.975" dx="1.5" dy="0.45" layer="1" roundness="20" stop="no"/>
<smd name="10" x="-2.9" y="-1.625" dx="1.5" dy="0.45" layer="1" roundness="20" stop="no"/>
<smd name="11" x="-2.9" y="-2.275" dx="1.5" dy="0.45" layer="1" roundness="20" stop="no"/>
<smd name="12" x="-2.9" y="-2.925" dx="1.5" dy="0.45" layer="1" roundness="20" stop="no"/>
<smd name="13" x="-2.9" y="-3.575" dx="1.5" dy="0.45" layer="1" roundness="20" stop="no"/>
<smd name="14" x="-2.9" y="-4.225" dx="1.5" dy="0.45" layer="1" roundness="20" stop="no"/>
<smd name="15" x="2.9" y="-4.225" dx="1.5" dy="0.45" layer="1" roundness="20" stop="no"/>
<smd name="16" x="2.9" y="-3.575" dx="1.5" dy="0.45" layer="1" roundness="20" stop="no"/>
<smd name="17" x="2.9" y="-2.925" dx="1.5" dy="0.45" layer="1" roundness="20" stop="no"/>
<smd name="18" x="2.9" y="-2.275" dx="1.5" dy="0.45" layer="1" roundness="20" stop="no"/>
<smd name="19" x="2.9" y="-1.625" dx="1.5" dy="0.45" layer="1" roundness="20" stop="no"/>
<smd name="20" x="2.9" y="-0.975" dx="1.5" dy="0.45" layer="1" roundness="20" stop="no"/>
<smd name="21" x="2.9" y="-0.325" dx="1.5" dy="0.45" layer="1" roundness="20" stop="no"/>
<smd name="22" x="2.9" y="0.325" dx="1.5" dy="0.45" layer="1" roundness="20" stop="no"/>
<smd name="23" x="2.9" y="0.975" dx="1.5" dy="0.45" layer="1" roundness="20" stop="no"/>
<smd name="24" x="2.9" y="1.625" dx="1.5" dy="0.45" layer="1" roundness="20" stop="no"/>
<smd name="25" x="2.9" y="2.275" dx="1.5" dy="0.45" layer="1" roundness="20" stop="no"/>
<smd name="26" x="2.9" y="2.925" dx="1.5" dy="0.45" layer="1" roundness="20" stop="no"/>
<smd name="27" x="2.9" y="3.575" dx="1.5" dy="0.45" layer="1" roundness="20" stop="no"/>
<smd name="28" x="2.9" y="4.225" dx="1.5" dy="0.45" layer="1" roundness="20" stop="no"/>
<smd name="29" x="0" y="0" dx="0.1" dy="0.1" layer="1" stop="no" cream="no"/>
<pad name="43" x="0" y="-0.6" drill="0.2" diameter="0.3" stop="no"/>
<pad name="40" x="0" y="0.6" drill="0.2" diameter="0.3" stop="no"/>
<pad name="39" x="-1.2" y="0.6" drill="0.2" diameter="0.3" stop="no"/>
<pad name="42" x="-1.2" y="-0.6" drill="0.2" diameter="0.3" stop="no"/>
<pad name="41" x="1.2" y="0.6" drill="0.2" diameter="0.3" stop="no"/>
<pad name="44" x="1.2" y="-0.6" drill="0.2" diameter="0.3" stop="no"/>
<pad name="49" x="0" y="-3" drill="0.2" diameter="0.3" stop="no"/>
<pad name="46" x="0" y="-1.8" drill="0.2" diameter="0.3" stop="no"/>
<pad name="45" x="-1.2" y="-1.8" drill="0.2" diameter="0.3" stop="no"/>
<pad name="48" x="-1.2" y="-3" drill="0.2" diameter="0.3" stop="no"/>
<pad name="47" x="1.2" y="-1.8" drill="0.2" diameter="0.3" stop="no"/>
<pad name="50" x="1.2" y="-3" drill="0.2" diameter="0.3" stop="no"/>
<pad name="51" x="-1.2" y="-4.2" drill="0.2" diameter="0.3" stop="no"/>
<pad name="52" x="0" y="-4.2" drill="0.2" diameter="0.3" stop="no"/>
<pad name="53" x="1.2" y="-4.2" drill="0.2" diameter="0.3" stop="no"/>
<pad name="37" x="0" y="1.8" drill="0.2" diameter="0.3" stop="no"/>
<pad name="36" x="-1.2" y="1.8" drill="0.2" diameter="0.3" stop="no"/>
<pad name="38" x="1.2" y="1.8" drill="0.2" diameter="0.3" stop="no"/>
<pad name="34" x="0" y="3" drill="0.2" diameter="0.3" stop="no"/>
<pad name="33" x="-1.2" y="3" drill="0.2" diameter="0.3" stop="no"/>
<pad name="35" x="1.2" y="3" drill="0.2" diameter="0.3" stop="no"/>
<pad name="31" x="0" y="4.2" drill="0.2" diameter="0.3" stop="no"/>
<pad name="30" x="-1.2" y="4.2" drill="0.2" diameter="0.3" stop="no"/>
<pad name="32" x="1.2" y="4.2" drill="0.2" diameter="0.3" stop="no"/>
</package>
</packages>
<packages3d>
<package3d name="DRV8313PWPR" urn="" wip_urn="urn:adsk.wipprod:fs.file:vf.8ar484oXSRiencqLNEvKVw?version=2" locally_modified="yes" type="model">
<packageinstances>
<packageinstance name="SOP65P640X120-29N"/>
</packageinstances>
</package3d>
</packages3d>
<symbols>
<symbol name="DRV8313PWPR">
<wire x1="-12.7" y1="30.48" x2="12.7" y2="30.48" width="0.254" layer="94"/>
<wire x1="12.7" y1="30.48" x2="12.7" y2="-30.48" width="0.254" layer="94"/>
<wire x1="12.7" y1="-30.48" x2="-12.7" y2="-30.48" width="0.254" layer="94"/>
<wire x1="-12.7" y1="-30.48" x2="-12.7" y2="30.48" width="0.254" layer="94"/>
<text x="-12.7" y="31.48" size="2.0828" layer="95" ratio="10" rot="SR0">&gt;NAME</text>
<text x="-12.7" y="-33.02" size="2.0828" layer="96" ratio="10" rot="SR0">&gt;VALUE</text>
<pin name="COMPN" x="-17.78" y="-7.62" length="middle" direction="in"/>
<pin name="COMPP" x="-17.78" y="-10.16" length="middle" direction="in"/>
<pin name="EN1" x="-17.78" y="12.7" length="middle" direction="in"/>
<pin name="EN2" x="-17.78" y="10.16" length="middle" direction="in"/>
<pin name="EN3" x="-17.78" y="7.62" length="middle" direction="in"/>
<pin name="IN1" x="-17.78" y="2.54" length="middle" direction="in"/>
<pin name="IN2" x="-17.78" y="0" length="middle" direction="in"/>
<pin name="IN3" x="-17.78" y="-2.54" length="middle" direction="in"/>
<pin name="!RESET" x="-17.78" y="20.32" length="middle" direction="in"/>
<pin name="!SLEEP" x="-17.78" y="17.78" length="middle" direction="in"/>
<pin name="!FAULT" x="17.78" y="0" length="middle" direction="out" rot="R180"/>
<pin name="CPL" x="17.78" y="17.78" length="middle" direction="pwr" rot="R180"/>
<pin name="CPH" x="17.78" y="15.24" length="middle" direction="pwr" rot="R180"/>
<pin name="VCP" x="17.78" y="27.94" length="middle" direction="pwr" rot="R180"/>
<pin name="VM" x="17.78" y="22.86" length="middle" direction="pwr" rot="R180"/>
<pin name="V3P3" x="17.78" y="25.4" length="middle" direction="pwr" rot="R180"/>
<pin name="EXP" x="17.78" y="-15.24" length="middle" direction="pwr" rot="R180"/>
<pin name="OUT1" x="17.78" y="10.16" length="middle" direction="out" rot="R180"/>
<pin name="OUT2" x="17.78" y="7.62" length="middle" direction="out" rot="R180"/>
<pin name="OUT3" x="17.78" y="5.08" length="middle" direction="out" rot="R180"/>
<pin name="PGND1" x="17.78" y="-20.32" length="middle" direction="pwr" rot="R180"/>
<pin name="PGND2" x="17.78" y="-22.86" length="middle" direction="pwr" rot="R180"/>
<pin name="PGND3" x="17.78" y="-25.4" length="middle" direction="pwr" rot="R180"/>
<pin name="GND" x="17.78" y="-27.94" length="middle" direction="pwr" rot="R180"/>
<pin name="NC" x="-17.78" y="-15.24" length="middle" direction="nc"/>
<pin name="!COMPO" x="17.78" y="-7.62" length="middle" direction="out" rot="R180"/>
</symbol>
</symbols>
<devicesets>
<deviceset name="DRV8313PWPR" prefix="U">
<description> &lt;a href="https://pricing.snapeda.com/parts/DRV8313PWPR/Texas%20Instruments/view-part?ref=eda"&gt;Check availability&lt;/a&gt;</description>
<gates>
<gate name="G$1" symbol="DRV8313PWPR" x="0" y="0"/>
</gates>
<devices>
<device name="" package="SOP65P640X120-29N">
<connects>
<connect gate="G$1" pin="!COMPO" pad="19"/>
<connect gate="G$1" pin="!FAULT" pad="18"/>
<connect gate="G$1" pin="!RESET" pad="16"/>
<connect gate="G$1" pin="!SLEEP" pad="17"/>
<connect gate="G$1" pin="COMPN" pad="13"/>
<connect gate="G$1" pin="COMPP" pad="12"/>
<connect gate="G$1" pin="CPH" pad="2"/>
<connect gate="G$1" pin="CPL" pad="1"/>
<connect gate="G$1" pin="EN1" pad="26"/>
<connect gate="G$1" pin="EN2" pad="24"/>
<connect gate="G$1" pin="EN3" pad="22"/>
<connect gate="G$1" pin="EXP" pad="29"/>
<connect gate="G$1" pin="GND" pad="14 20 28"/>
<connect gate="G$1" pin="IN1" pad="27"/>
<connect gate="G$1" pin="IN2" pad="25"/>
<connect gate="G$1" pin="IN3" pad="23"/>
<connect gate="G$1" pin="NC" pad="21"/>
<connect gate="G$1" pin="OUT1" pad="5"/>
<connect gate="G$1" pin="OUT2" pad="8"/>
<connect gate="G$1" pin="OUT3" pad="9"/>
<connect gate="G$1" pin="PGND1" pad="6"/>
<connect gate="G$1" pin="PGND2" pad="7"/>
<connect gate="G$1" pin="PGND3" pad="10"/>
<connect gate="G$1" pin="V3P3" pad="15"/>
<connect gate="G$1" pin="VCP" pad="3"/>
<connect gate="G$1" pin="VM" pad="4 11"/>
</connects>

<package3dinstances>
<package3dinstance package3d_urn="urn:adsk.wipprod:fs.file:vf.8ar484oXSRiencqLNEvKVw?version=2"/>
</package3dinstances>
<technologies>
<technology name="">
<attribute name="MF" value="Texas Instruments"/>
<attribute name="DESCRIPTION" value="                                                      65-V max 3-A peak 3-phase motor driver with integrated FETs                                              "/>
<attribute name="PACKAGE" value="HTSSOP-28 Texas Instruments"/>
<attribute name="PRICE" value="None"/>
<attribute name="SNAPEDA_LINK" value="https://www.snapeda.com/parts/DRV8313PWPR/Texas+Instruments/view-part/?ref=snap"/>
<attribute name="MP" value="DRV8313PWPR"/>
<attribute name="AVAILABILITY" value="In Stock"/>
<attribute name="CHECK_PRICES" value="https://www.snapeda.com/parts/DRV8313PWPR/Texas+Instruments/view-part/?ref=eda"/>
</technology>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
<library name="TYPE-C-31-M-12" urn="urn:adsk.wipprod:fs.file:vf.lrnFXZjHR86kpQ-OMBs1Ug">
<packages>
<package name="HRO_TYPE-C-31-M-12">
<wire x1="-4.47" y1="-2.6" x2="4.47" y2="-2.6" width="0.127" layer="51"/>
<wire x1="4.47" y1="-2.6" x2="4.47" y2="4.75" width="0.127" layer="51"/>
<wire x1="4.47" y1="4.75" x2="-4.47" y2="4.75" width="0.127" layer="51"/>
<wire x1="-4.47" y1="4.75" x2="-4.47" y2="-2.6" width="0.127" layer="51"/>
<polygon width="0.01" layer="1" pour="solid">
<vertex x="-4.425165625" y="3.18128125"/>
<vertex x="-4.325" y="3.17"/>
<vertex x="-4.224834375" y="3.18128125"/>
<vertex x="-4.129709375" y="3.21455625"/>
<vertex x="-4.044490625" y="3.2681125"/>
<vertex x="-3.9731125" y="3.339490625"/>
<vertex x="-3.91955625" y="3.424709375"/>
<vertex x="-3.88628125" y="3.519834375"/>
<vertex x="-3.875" y="3.6200625"/>
<vertex x="-3.875" y="4.7199375"/>
<vertex x="-3.88628125" y="4.820165625"/>
<vertex x="-3.91955625" y="4.915290625"/>
<vertex x="-3.9731125" y="5.000509375"/>
<vertex x="-4.044490625" y="5.0718875"/>
<vertex x="-4.129709375" y="5.12544375"/>
<vertex x="-4.224834375" y="5.15871875"/>
<vertex x="-4.325" y="5.17"/>
<vertex x="-4.425165625" y="5.15871875"/>
<vertex x="-4.520290625" y="5.12544375"/>
<vertex x="-4.605509375" y="5.0718875"/>
<vertex x="-4.6768875" y="5.000509375"/>
<vertex x="-4.73044375" y="4.915290625"/>
<vertex x="-4.76371875" y="4.820165625"/>
<vertex x="-4.775" y="4.7199375"/>
<vertex x="-4.775" y="3.6200625"/>
<vertex x="-4.76371875" y="3.519834375"/>
<vertex x="-4.73044375" y="3.424709375"/>
<vertex x="-4.6768875" y="3.339490625"/>
<vertex x="-4.605509375" y="3.2681125"/>
<vertex x="-4.520290625" y="3.21455625"/>
</polygon>
<polygon width="0.01" layer="16" pour="solid">
<vertex x="-4.425165625" y="3.18128125"/>
<vertex x="-4.325" y="3.17"/>
<vertex x="-4.224834375" y="3.18128125"/>
<vertex x="-4.129709375" y="3.21455625"/>
<vertex x="-4.044490625" y="3.2681125"/>
<vertex x="-3.9731125" y="3.339490625"/>
<vertex x="-3.91955625" y="3.424709375"/>
<vertex x="-3.88628125" y="3.519834375"/>
<vertex x="-3.875" y="3.6200625"/>
<vertex x="-3.875" y="4.7199375"/>
<vertex x="-3.88628125" y="4.820165625"/>
<vertex x="-3.91955625" y="4.915290625"/>
<vertex x="-3.9731125" y="5.000509375"/>
<vertex x="-4.044490625" y="5.0718875"/>
<vertex x="-4.129709375" y="5.12544375"/>
<vertex x="-4.224834375" y="5.15871875"/>
<vertex x="-4.325" y="5.17"/>
<vertex x="-4.425165625" y="5.15871875"/>
<vertex x="-4.520290625" y="5.12544375"/>
<vertex x="-4.605509375" y="5.0718875"/>
<vertex x="-4.6768875" y="5.000509375"/>
<vertex x="-4.73044375" y="4.915290625"/>
<vertex x="-4.76371875" y="4.820165625"/>
<vertex x="-4.775" y="4.7199375"/>
<vertex x="-4.775" y="3.6200625"/>
<vertex x="-4.76371875" y="3.519834375"/>
<vertex x="-4.73044375" y="3.424709375"/>
<vertex x="-4.6768875" y="3.339490625"/>
<vertex x="-4.605509375" y="3.2681125"/>
<vertex x="-4.520290625" y="3.21455625"/>
</polygon>
<polygon width="0.01" layer="29" pour="solid">
<vertex x="-4.436290625" y="3.1325375"/>
<vertex x="-4.325" y="3.12"/>
<vertex x="-4.213709375" y="3.1325375"/>
<vertex x="-4.108015625" y="3.169509375"/>
<vertex x="-4.01331875" y="3.229021875"/>
<vertex x="-3.934021875" y="3.30831875"/>
<vertex x="-3.874509375" y="3.403015625"/>
<vertex x="-3.8375375" y="3.508709375"/>
<vertex x="-3.825" y="3.6200625"/>
<vertex x="-3.825" y="4.7199375"/>
<vertex x="-3.8375375" y="4.831290625"/>
<vertex x="-3.874509375" y="4.936984375"/>
<vertex x="-3.934021875" y="5.03168125"/>
<vertex x="-4.01331875" y="5.110978125"/>
<vertex x="-4.108015625" y="5.170490625"/>
<vertex x="-4.213709375" y="5.2074625"/>
<vertex x="-4.325" y="5.22"/>
<vertex x="-4.436290625" y="5.2074625"/>
<vertex x="-4.541984375" y="5.170490625"/>
<vertex x="-4.63668125" y="5.110978125"/>
<vertex x="-4.715978125" y="5.03168125"/>
<vertex x="-4.775490625" y="4.936984375"/>
<vertex x="-4.8124625" y="4.831290625"/>
<vertex x="-4.825" y="4.7199375"/>
<vertex x="-4.825" y="3.6200625"/>
<vertex x="-4.8124625" y="3.508709375"/>
<vertex x="-4.775490625" y="3.403015625"/>
<vertex x="-4.715978125" y="3.30831875"/>
<vertex x="-4.63668125" y="3.229021875"/>
<vertex x="-4.541984375" y="3.169509375"/>
</polygon>
<polygon width="0.01" layer="30" pour="solid">
<vertex x="-4.436290625" y="3.1325375"/>
<vertex x="-4.325" y="3.12"/>
<vertex x="-4.213709375" y="3.1325375"/>
<vertex x="-4.108015625" y="3.169509375"/>
<vertex x="-4.01331875" y="3.229021875"/>
<vertex x="-3.934021875" y="3.30831875"/>
<vertex x="-3.874509375" y="3.403015625"/>
<vertex x="-3.8375375" y="3.508709375"/>
<vertex x="-3.825" y="3.6200625"/>
<vertex x="-3.825" y="4.7199375"/>
<vertex x="-3.8375375" y="4.831290625"/>
<vertex x="-3.874509375" y="4.936984375"/>
<vertex x="-3.934021875" y="5.03168125"/>
<vertex x="-4.01331875" y="5.110978125"/>
<vertex x="-4.108015625" y="5.170490625"/>
<vertex x="-4.213709375" y="5.2074625"/>
<vertex x="-4.325" y="5.22"/>
<vertex x="-4.436290625" y="5.2074625"/>
<vertex x="-4.541984375" y="5.170490625"/>
<vertex x="-4.63668125" y="5.110978125"/>
<vertex x="-4.715978125" y="5.03168125"/>
<vertex x="-4.775490625" y="4.936984375"/>
<vertex x="-4.8124625" y="4.831290625"/>
<vertex x="-4.825" y="4.7199375"/>
<vertex x="-4.825" y="3.6200625"/>
<vertex x="-4.8124625" y="3.508709375"/>
<vertex x="-4.775490625" y="3.403015625"/>
<vertex x="-4.715978125" y="3.30831875"/>
<vertex x="-4.63668125" y="3.229021875"/>
<vertex x="-4.541984375" y="3.169509375"/>
</polygon>
<polygon width="0.01" layer="1" pour="solid">
<vertex x="4.224834375" y="3.18128125"/>
<vertex x="4.325" y="3.17"/>
<vertex x="4.425165625" y="3.18128125"/>
<vertex x="4.520290625" y="3.21455625"/>
<vertex x="4.605509375" y="3.2681125"/>
<vertex x="4.6768875" y="3.339490625"/>
<vertex x="4.73044375" y="3.424709375"/>
<vertex x="4.76371875" y="3.519834375"/>
<vertex x="4.775" y="3.6200625"/>
<vertex x="4.775" y="4.7199375"/>
<vertex x="4.76371875" y="4.820165625"/>
<vertex x="4.73044375" y="4.915290625"/>
<vertex x="4.6768875" y="5.000509375"/>
<vertex x="4.605509375" y="5.0718875"/>
<vertex x="4.520290625" y="5.12544375"/>
<vertex x="4.425165625" y="5.15871875"/>
<vertex x="4.325" y="5.17"/>
<vertex x="4.224834375" y="5.15871875"/>
<vertex x="4.129709375" y="5.12544375"/>
<vertex x="4.044490625" y="5.0718875"/>
<vertex x="3.9731125" y="5.000509375"/>
<vertex x="3.91955625" y="4.915290625"/>
<vertex x="3.88628125" y="4.820165625"/>
<vertex x="3.875" y="4.7199375"/>
<vertex x="3.875" y="3.6200625"/>
<vertex x="3.88628125" y="3.519834375"/>
<vertex x="3.91955625" y="3.424709375"/>
<vertex x="3.9731125" y="3.339490625"/>
<vertex x="4.044490625" y="3.2681125"/>
<vertex x="4.129709375" y="3.21455625"/>
</polygon>
<polygon width="0.01" layer="16" pour="solid">
<vertex x="4.224834375" y="3.18128125"/>
<vertex x="4.325" y="3.17"/>
<vertex x="4.425165625" y="3.18128125"/>
<vertex x="4.520290625" y="3.21455625"/>
<vertex x="4.605509375" y="3.2681125"/>
<vertex x="4.6768875" y="3.339490625"/>
<vertex x="4.73044375" y="3.424709375"/>
<vertex x="4.76371875" y="3.519834375"/>
<vertex x="4.775" y="3.6200625"/>
<vertex x="4.775" y="4.7199375"/>
<vertex x="4.76371875" y="4.820165625"/>
<vertex x="4.73044375" y="4.915290625"/>
<vertex x="4.6768875" y="5.000509375"/>
<vertex x="4.605509375" y="5.0718875"/>
<vertex x="4.520290625" y="5.12544375"/>
<vertex x="4.425165625" y="5.15871875"/>
<vertex x="4.325" y="5.17"/>
<vertex x="4.224834375" y="5.15871875"/>
<vertex x="4.129709375" y="5.12544375"/>
<vertex x="4.044490625" y="5.0718875"/>
<vertex x="3.9731125" y="5.000509375"/>
<vertex x="3.91955625" y="4.915290625"/>
<vertex x="3.88628125" y="4.820165625"/>
<vertex x="3.875" y="4.7199375"/>
<vertex x="3.875" y="3.6200625"/>
<vertex x="3.88628125" y="3.519834375"/>
<vertex x="3.91955625" y="3.424709375"/>
<vertex x="3.9731125" y="3.339490625"/>
<vertex x="4.044490625" y="3.2681125"/>
<vertex x="4.129709375" y="3.21455625"/>
</polygon>
<polygon width="0.01" layer="29" pour="solid">
<vertex x="4.212753125" y="3.133775"/>
<vertex x="4.324934375" y="3.12"/>
<vertex x="4.436290625" y="3.1325375"/>
<vertex x="4.541984375" y="3.169509375"/>
<vertex x="4.63668125" y="3.229021875"/>
<vertex x="4.715978125" y="3.30831875"/>
<vertex x="4.775490625" y="3.403015625"/>
<vertex x="4.8124625" y="3.508709375"/>
<vertex x="4.825" y="3.6200625"/>
<vertex x="4.825" y="4.7199375"/>
<vertex x="4.8124625" y="4.831290625"/>
<vertex x="4.775490625" y="4.936984375"/>
<vertex x="4.715978125" y="5.03168125"/>
<vertex x="4.63668125" y="5.110978125"/>
<vertex x="4.541984375" y="5.170490625"/>
<vertex x="4.436290625" y="5.2074625"/>
<vertex x="4.3249375" y="5.22"/>
<vertex x="4.214765625" y="5.208703125"/>
<vertex x="4.1097" y="5.17315"/>
<vertex x="4.01529375" y="5.1151375"/>
<vertex x="3.936015625" y="5.037428125"/>
<vertex x="3.8761875" y="4.94429375"/>
<vertex x="3.838521875" y="4.8399875"/>
<vertex x="3.825" y="4.730040625"/>
<vertex x="3.825" y="3.63003125"/>
<vertex x="3.83655" y="3.517559375"/>
<vertex x="3.872840625" y="3.41036875"/>
<vertex x="3.932025" y="3.314078125"/>
<vertex x="4.011265625" y="3.23325"/>
<vertex x="4.106353125" y="3.17218125"/>
</polygon>
<polygon width="0.01" layer="30" pour="solid">
<vertex x="4.212753125" y="3.133775"/>
<vertex x="4.324934375" y="3.12"/>
<vertex x="4.436290625" y="3.1325375"/>
<vertex x="4.541984375" y="3.169509375"/>
<vertex x="4.63668125" y="3.229021875"/>
<vertex x="4.715978125" y="3.30831875"/>
<vertex x="4.775490625" y="3.403015625"/>
<vertex x="4.8124625" y="3.508709375"/>
<vertex x="4.825" y="3.6200625"/>
<vertex x="4.825" y="4.7199375"/>
<vertex x="4.8124625" y="4.831290625"/>
<vertex x="4.775490625" y="4.936984375"/>
<vertex x="4.715978125" y="5.03168125"/>
<vertex x="4.63668125" y="5.110978125"/>
<vertex x="4.541984375" y="5.170490625"/>
<vertex x="4.436290625" y="5.2074625"/>
<vertex x="4.3249375" y="5.22"/>
<vertex x="4.214765625" y="5.208703125"/>
<vertex x="4.1097" y="5.17315"/>
<vertex x="4.01529375" y="5.1151375"/>
<vertex x="3.936015625" y="5.037428125"/>
<vertex x="3.8761875" y="4.94429375"/>
<vertex x="3.838521875" y="4.8399875"/>
<vertex x="3.825" y="4.730040625"/>
<vertex x="3.825" y="3.63003125"/>
<vertex x="3.83655" y="3.517559375"/>
<vertex x="3.872840625" y="3.41036875"/>
<vertex x="3.932025" y="3.314078125"/>
<vertex x="4.011265625" y="3.23325"/>
<vertex x="4.106353125" y="3.17218125"/>
</polygon>
<polygon width="0.01" layer="1" pour="solid">
<vertex x="-4.425165625" y="-0.83871875"/>
<vertex x="-4.325" y="-0.85"/>
<vertex x="-4.224834375" y="-0.83871875"/>
<vertex x="-4.129709375" y="-0.80544375"/>
<vertex x="-4.044490625" y="-0.7518875"/>
<vertex x="-3.9731125" y="-0.680509375"/>
<vertex x="-3.91955625" y="-0.595290625"/>
<vertex x="-3.88628125" y="-0.500165625"/>
<vertex x="-3.875" y="-0.3999375"/>
<vertex x="-3.875" y="0.3999375"/>
<vertex x="-3.88628125" y="0.500165625"/>
<vertex x="-3.91955625" y="0.595290625"/>
<vertex x="-3.9731125" y="0.680509375"/>
<vertex x="-4.044490625" y="0.7518875"/>
<vertex x="-4.129709375" y="0.80544375"/>
<vertex x="-4.224834375" y="0.83871875"/>
<vertex x="-4.325" y="0.85"/>
<vertex x="-4.425165625" y="0.83871875"/>
<vertex x="-4.520290625" y="0.80544375"/>
<vertex x="-4.605509375" y="0.7518875"/>
<vertex x="-4.6768875" y="0.680509375"/>
<vertex x="-4.73044375" y="0.595290625"/>
<vertex x="-4.76371875" y="0.500165625"/>
<vertex x="-4.775" y="0.3999375"/>
<vertex x="-4.775" y="-0.3999375"/>
<vertex x="-4.76371875" y="-0.500165625"/>
<vertex x="-4.73044375" y="-0.595290625"/>
<vertex x="-4.6768875" y="-0.680509375"/>
<vertex x="-4.605509375" y="-0.7518875"/>
<vertex x="-4.520290625" y="-0.80544375"/>
</polygon>
<polygon width="0.01" layer="16" pour="solid">
<vertex x="-4.425165625" y="-0.83871875"/>
<vertex x="-4.325" y="-0.85"/>
<vertex x="-4.224834375" y="-0.83871875"/>
<vertex x="-4.129709375" y="-0.80544375"/>
<vertex x="-4.044490625" y="-0.7518875"/>
<vertex x="-3.9731125" y="-0.680509375"/>
<vertex x="-3.91955625" y="-0.595290625"/>
<vertex x="-3.88628125" y="-0.500165625"/>
<vertex x="-3.875" y="-0.3999375"/>
<vertex x="-3.875" y="0.3999375"/>
<vertex x="-3.88628125" y="0.500165625"/>
<vertex x="-3.91955625" y="0.595290625"/>
<vertex x="-3.9731125" y="0.680509375"/>
<vertex x="-4.044490625" y="0.7518875"/>
<vertex x="-4.129709375" y="0.80544375"/>
<vertex x="-4.224834375" y="0.83871875"/>
<vertex x="-4.325" y="0.85"/>
<vertex x="-4.425165625" y="0.83871875"/>
<vertex x="-4.520290625" y="0.80544375"/>
<vertex x="-4.605509375" y="0.7518875"/>
<vertex x="-4.6768875" y="0.680509375"/>
<vertex x="-4.73044375" y="0.595290625"/>
<vertex x="-4.76371875" y="0.500165625"/>
<vertex x="-4.775" y="0.3999375"/>
<vertex x="-4.775" y="-0.3999375"/>
<vertex x="-4.76371875" y="-0.500165625"/>
<vertex x="-4.73044375" y="-0.595290625"/>
<vertex x="-4.6768875" y="-0.680509375"/>
<vertex x="-4.605509375" y="-0.7518875"/>
<vertex x="-4.520290625" y="-0.80544375"/>
</polygon>
<polygon width="0.01" layer="29" pour="solid">
<vertex x="-4.436290625" y="-0.8874625"/>
<vertex x="-4.325" y="-0.9"/>
<vertex x="-4.213709375" y="-0.8874625"/>
<vertex x="-4.108015625" y="-0.850490625"/>
<vertex x="-4.01331875" y="-0.790978125"/>
<vertex x="-3.934021875" y="-0.71168125"/>
<vertex x="-3.874509375" y="-0.616984375"/>
<vertex x="-3.8375375" y="-0.511290625"/>
<vertex x="-3.825" y="-0.3999375"/>
<vertex x="-3.825" y="0.3999375"/>
<vertex x="-3.8375375" y="0.511290625"/>
<vertex x="-3.874509375" y="0.616984375"/>
<vertex x="-3.934021875" y="0.71168125"/>
<vertex x="-4.01331875" y="0.790978125"/>
<vertex x="-4.108015625" y="0.850490625"/>
<vertex x="-4.213709375" y="0.8874625"/>
<vertex x="-4.325" y="0.9"/>
<vertex x="-4.436290625" y="0.8874625"/>
<vertex x="-4.541984375" y="0.850490625"/>
<vertex x="-4.63668125" y="0.790978125"/>
<vertex x="-4.715978125" y="0.71168125"/>
<vertex x="-4.775490625" y="0.616984375"/>
<vertex x="-4.8124625" y="0.511290625"/>
<vertex x="-4.825" y="0.3999375"/>
<vertex x="-4.825" y="-0.3999375"/>
<vertex x="-4.8124625" y="-0.511290625"/>
<vertex x="-4.775490625" y="-0.616984375"/>
<vertex x="-4.715978125" y="-0.71168125"/>
<vertex x="-4.63668125" y="-0.790978125"/>
<vertex x="-4.541984375" y="-0.850490625"/>
</polygon>
<polygon width="0.01" layer="30" pour="solid">
<vertex x="-4.436290625" y="-0.8874625"/>
<vertex x="-4.325" y="-0.9"/>
<vertex x="-4.213709375" y="-0.8874625"/>
<vertex x="-4.108015625" y="-0.850490625"/>
<vertex x="-4.01331875" y="-0.790978125"/>
<vertex x="-3.934021875" y="-0.71168125"/>
<vertex x="-3.874509375" y="-0.616984375"/>
<vertex x="-3.8375375" y="-0.511290625"/>
<vertex x="-3.825" y="-0.3999375"/>
<vertex x="-3.825" y="0.3999375"/>
<vertex x="-3.8375375" y="0.511290625"/>
<vertex x="-3.874509375" y="0.616984375"/>
<vertex x="-3.934021875" y="0.71168125"/>
<vertex x="-4.01331875" y="0.790978125"/>
<vertex x="-4.108015625" y="0.850490625"/>
<vertex x="-4.213709375" y="0.8874625"/>
<vertex x="-4.325" y="0.9"/>
<vertex x="-4.436290625" y="0.8874625"/>
<vertex x="-4.541984375" y="0.850490625"/>
<vertex x="-4.63668125" y="0.790978125"/>
<vertex x="-4.715978125" y="0.71168125"/>
<vertex x="-4.775490625" y="0.616984375"/>
<vertex x="-4.8124625" y="0.511290625"/>
<vertex x="-4.825" y="0.3999375"/>
<vertex x="-4.825" y="-0.3999375"/>
<vertex x="-4.8124625" y="-0.511290625"/>
<vertex x="-4.775490625" y="-0.616984375"/>
<vertex x="-4.715978125" y="-0.71168125"/>
<vertex x="-4.63668125" y="-0.790978125"/>
<vertex x="-4.541984375" y="-0.850490625"/>
</polygon>
<polygon width="0.01" layer="1" pour="solid">
<vertex x="4.224834375" y="-0.83871875"/>
<vertex x="4.325" y="-0.85"/>
<vertex x="4.425165625" y="-0.83871875"/>
<vertex x="4.520290625" y="-0.80544375"/>
<vertex x="4.605509375" y="-0.7518875"/>
<vertex x="4.6768875" y="-0.680509375"/>
<vertex x="4.73044375" y="-0.595290625"/>
<vertex x="4.76371875" y="-0.500165625"/>
<vertex x="4.775" y="-0.3999375"/>
<vertex x="4.775" y="0.3999375"/>
<vertex x="4.76371875" y="0.500165625"/>
<vertex x="4.73044375" y="0.595290625"/>
<vertex x="4.6768875" y="0.680509375"/>
<vertex x="4.605509375" y="0.7518875"/>
<vertex x="4.520290625" y="0.80544375"/>
<vertex x="4.425165625" y="0.83871875"/>
<vertex x="4.325" y="0.85"/>
<vertex x="4.224834375" y="0.83871875"/>
<vertex x="4.129709375" y="0.80544375"/>
<vertex x="4.044490625" y="0.7518875"/>
<vertex x="3.9731125" y="0.680509375"/>
<vertex x="3.91955625" y="0.595290625"/>
<vertex x="3.88628125" y="0.500165625"/>
<vertex x="3.875" y="0.3999375"/>
<vertex x="3.875" y="-0.3999375"/>
<vertex x="3.88628125" y="-0.500165625"/>
<vertex x="3.91955625" y="-0.595290625"/>
<vertex x="3.9731125" y="-0.680509375"/>
<vertex x="4.044490625" y="-0.7518875"/>
<vertex x="4.129709375" y="-0.80544375"/>
</polygon>
<polygon width="0.01" layer="16" pour="solid">
<vertex x="4.224834375" y="-0.83871875"/>
<vertex x="4.325" y="-0.85"/>
<vertex x="4.425165625" y="-0.83871875"/>
<vertex x="4.520290625" y="-0.80544375"/>
<vertex x="4.605509375" y="-0.7518875"/>
<vertex x="4.6768875" y="-0.680509375"/>
<vertex x="4.73044375" y="-0.595290625"/>
<vertex x="4.76371875" y="-0.500165625"/>
<vertex x="4.775" y="-0.3999375"/>
<vertex x="4.775" y="0.3999375"/>
<vertex x="4.76371875" y="0.500165625"/>
<vertex x="4.73044375" y="0.595290625"/>
<vertex x="4.6768875" y="0.680509375"/>
<vertex x="4.605509375" y="0.7518875"/>
<vertex x="4.520290625" y="0.80544375"/>
<vertex x="4.425165625" y="0.83871875"/>
<vertex x="4.325" y="0.85"/>
<vertex x="4.224834375" y="0.83871875"/>
<vertex x="4.129709375" y="0.80544375"/>
<vertex x="4.044490625" y="0.7518875"/>
<vertex x="3.9731125" y="0.680509375"/>
<vertex x="3.91955625" y="0.595290625"/>
<vertex x="3.88628125" y="0.500165625"/>
<vertex x="3.875" y="0.3999375"/>
<vertex x="3.875" y="-0.3999375"/>
<vertex x="3.88628125" y="-0.500165625"/>
<vertex x="3.91955625" y="-0.595290625"/>
<vertex x="3.9731125" y="-0.680509375"/>
<vertex x="4.044490625" y="-0.7518875"/>
<vertex x="4.129709375" y="-0.80544375"/>
</polygon>
<polygon width="0.01" layer="29" pour="solid">
<vertex x="4.213709375" y="-0.8874625"/>
<vertex x="4.325" y="-0.9"/>
<vertex x="4.436290625" y="-0.8874625"/>
<vertex x="4.541984375" y="-0.850490625"/>
<vertex x="4.63668125" y="-0.790978125"/>
<vertex x="4.715978125" y="-0.71168125"/>
<vertex x="4.775490625" y="-0.616984375"/>
<vertex x="4.8124625" y="-0.511290625"/>
<vertex x="4.825" y="-0.3999375"/>
<vertex x="4.825" y="0.3999375"/>
<vertex x="4.8124625" y="0.511290625"/>
<vertex x="4.775490625" y="0.616984375"/>
<vertex x="4.715978125" y="0.71168125"/>
<vertex x="4.63668125" y="0.790978125"/>
<vertex x="4.541984375" y="0.850490625"/>
<vertex x="4.436290625" y="0.8874625"/>
<vertex x="4.325" y="0.9"/>
<vertex x="4.213709375" y="0.8874625"/>
<vertex x="4.108015625" y="0.850490625"/>
<vertex x="4.01331875" y="0.790978125"/>
<vertex x="3.934021875" y="0.71168125"/>
<vertex x="3.874509375" y="0.616984375"/>
<vertex x="3.8375375" y="0.511290625"/>
<vertex x="3.825" y="0.3999375"/>
<vertex x="3.825" y="-0.3999375"/>
<vertex x="3.8375375" y="-0.511290625"/>
<vertex x="3.874509375" y="-0.616984375"/>
<vertex x="3.934021875" y="-0.71168125"/>
<vertex x="4.01331875" y="-0.790978125"/>
<vertex x="4.108015625" y="-0.850490625"/>
</polygon>
<polygon width="0.01" layer="30" pour="solid">
<vertex x="4.213709375" y="-0.8874625"/>
<vertex x="4.325" y="-0.9"/>
<vertex x="4.436290625" y="-0.8874625"/>
<vertex x="4.541984375" y="-0.850490625"/>
<vertex x="4.63668125" y="-0.790978125"/>
<vertex x="4.715978125" y="-0.71168125"/>
<vertex x="4.775490625" y="-0.616984375"/>
<vertex x="4.8124625" y="-0.511290625"/>
<vertex x="4.825" y="-0.3999375"/>
<vertex x="4.825" y="0.3999375"/>
<vertex x="4.8124625" y="0.511290625"/>
<vertex x="4.775490625" y="0.616984375"/>
<vertex x="4.715978125" y="0.71168125"/>
<vertex x="4.63668125" y="0.790978125"/>
<vertex x="4.541984375" y="0.850490625"/>
<vertex x="4.436290625" y="0.8874625"/>
<vertex x="4.325" y="0.9"/>
<vertex x="4.213709375" y="0.8874625"/>
<vertex x="4.108015625" y="0.850490625"/>
<vertex x="4.01331875" y="0.790978125"/>
<vertex x="3.934021875" y="0.71168125"/>
<vertex x="3.874509375" y="0.616984375"/>
<vertex x="3.8375375" y="0.511290625"/>
<vertex x="3.825" y="0.3999375"/>
<vertex x="3.825" y="-0.3999375"/>
<vertex x="3.8375375" y="-0.511290625"/>
<vertex x="3.874509375" y="-0.616984375"/>
<vertex x="3.934021875" y="-0.71168125"/>
<vertex x="4.01331875" y="-0.790978125"/>
<vertex x="4.108015625" y="-0.850490625"/>
</polygon>
<wire x1="4.47" y1="2.85" x2="4.47" y2="1.17" width="0.127" layer="21"/>
<wire x1="4.47" y1="-2.6" x2="4.47" y2="-1.17" width="0.127" layer="21"/>
<wire x1="-4.47" y1="2.85" x2="-4.47" y2="1.17" width="0.127" layer="21"/>
<wire x1="-4.47" y1="-2.6" x2="-4.47" y2="-1.17" width="0.127" layer="21"/>
<wire x1="-4.47" y1="-2.6" x2="4.47" y2="-2.6" width="0.127" layer="21"/>
<wire x1="-5.025" y1="5.57" x2="-5.025" y2="-2.85" width="0.05" layer="39"/>
<wire x1="-5.025" y1="-2.85" x2="5.025" y2="-2.85" width="0.05" layer="39"/>
<wire x1="5.025" y1="-2.85" x2="5.025" y2="5.57" width="0.05" layer="39"/>
<wire x1="5.025" y1="5.57" x2="-5.025" y2="5.57" width="0.05" layer="39"/>
<text x="-5.025" y="6.57" size="1.27" layer="25">&gt;NAME</text>
<text x="-5.025" y="-3.05" size="1.27" layer="27" align="top-left">&gt;VALUE</text>
<wire x1="-5.5" y1="-2.11" x2="9" y2="-2.11" width="0.127" layer="51"/>
<text x="5.2" y="-1.9" size="0.6096" layer="51">PCB EDGE</text>
<circle x="-3.2" y="6" radius="0.1" width="0.2" layer="21"/>
<circle x="-3.2" y="6" radius="0.1" width="0.2" layer="51"/>
<rectangle x1="-1.875" y1="4.13" x2="-1.625" y2="5.37" layer="29"/>
<rectangle x1="-1.375" y1="4.13" x2="-1.125" y2="5.37" layer="29"/>
<rectangle x1="-0.875" y1="4.13" x2="-0.625" y2="5.37" layer="29"/>
<rectangle x1="1.625" y1="4.13" x2="1.875" y2="5.37" layer="29"/>
<rectangle x1="1.125" y1="4.13" x2="1.375" y2="5.37" layer="29"/>
<rectangle x1="0.625" y1="4.13" x2="0.875" y2="5.37" layer="29"/>
<rectangle x1="-0.375" y1="4.13" x2="-0.125" y2="5.37" layer="29"/>
<rectangle x1="0.125" y1="4.13" x2="0.375" y2="5.37" layer="29"/>
<rectangle x1="-3.475" y1="4.13" x2="-2.925" y2="5.37" layer="29"/>
<rectangle x1="2.925" y1="4.13" x2="3.475" y2="5.37" layer="29"/>
<rectangle x1="-2.675" y1="4.13" x2="-2.125" y2="5.37" layer="29"/>
<rectangle x1="2.125" y1="4.13" x2="2.675" y2="5.37" layer="29"/>
<smd name="A1_B12" x="-3.2" y="4.75" dx="0.6" dy="1.14" layer="1" stop="no"/>
<smd name="A4_B9" x="-2.4" y="4.75" dx="0.6" dy="1.14" layer="1" stop="no"/>
<smd name="A6" x="-0.25" y="4.75" dx="0.3" dy="1.14" layer="1" stop="no"/>
<smd name="B7" x="-0.75" y="4.75" dx="0.3" dy="1.14" layer="1" stop="no"/>
<smd name="A5" x="-1.25" y="4.75" dx="0.3" dy="1.14" layer="1" stop="no"/>
<smd name="B8" x="-1.75" y="4.75" dx="0.3" dy="1.14" layer="1" stop="no"/>
<smd name="A7" x="0.25" y="4.75" dx="0.3" dy="1.14" layer="1" stop="no"/>
<smd name="B6" x="0.75" y="4.75" dx="0.3" dy="1.14" layer="1" stop="no"/>
<smd name="A8" x="1.25" y="4.75" dx="0.3" dy="1.14" layer="1" stop="no"/>
<smd name="B5" x="1.75" y="4.75" dx="0.3" dy="1.14" layer="1" stop="no"/>
<smd name="B4_A9" x="2.4" y="4.75" dx="0.6" dy="1.14" layer="1" stop="no"/>
<smd name="B1_A12" x="3.2" y="4.75" dx="0.6" dy="1.14" layer="1" stop="no"/>
<hole x="-2.89" y="3.68" drill="0.6"/>
<hole x="2.89" y="3.68" drill="0.6"/>
<pad name="S1" x="-4.325" y="4.17" drill="0.6" diameter="1" shape="long" rot="R90"/>
<pad name="S1B" x="-4.325" y="3.62" drill="0.6" diameter="0.9" stop="no" thermals="no"/>
<pad name="S1C" x="-4.325" y="3.7575" drill="0.6" diameter="0.9" stop="no" thermals="no"/>
<pad name="S1D" x="-4.325" y="3.895" drill="0.6" diameter="0.9" stop="no" thermals="no"/>
<pad name="S1E" x="-4.325" y="4.0325" drill="0.6" diameter="0.9" stop="no" thermals="no"/>
<pad name="S1F" x="-4.325" y="4.3075" drill="0.6" diameter="0.9" stop="no" thermals="no"/>
<pad name="S1G" x="-4.325" y="4.445" drill="0.6" diameter="0.9" stop="no" thermals="no"/>
<pad name="S1H" x="-4.325" y="4.5825" drill="0.6" diameter="0.9" stop="no" thermals="no"/>
<pad name="S1I" x="-4.325" y="4.72" drill="0.6" diameter="0.9" stop="no" thermals="no"/>
<pad name="S2" x="4.325" y="4.17" drill="0.6" diameter="1" shape="long" rot="R90"/>
<pad name="S2B" x="4.325" y="3.62" drill="0.6" diameter="0.9" stop="no" thermals="no"/>
<pad name="S2C" x="4.325" y="3.7575" drill="0.6" diameter="0.9" stop="no" thermals="no"/>
<pad name="S2D" x="4.325" y="3.895" drill="0.6" diameter="0.9" stop="no" thermals="no"/>
<pad name="S2E" x="4.325" y="4.0325" drill="0.6" diameter="0.9" stop="no" thermals="no"/>
<pad name="S2F" x="4.325" y="4.3075" drill="0.6" diameter="0.9" stop="no" thermals="no"/>
<pad name="S2G" x="4.325" y="4.445" drill="0.6" diameter="0.9" stop="no" thermals="no"/>
<pad name="S2H" x="4.325" y="4.5825" drill="0.6" diameter="0.9" stop="no" thermals="no"/>
<pad name="S2I" x="4.325" y="4.72" drill="0.6" diameter="0.9" stop="no" thermals="no"/>
<pad name="S3" x="-4.325" y="0" drill="0.6" diameter="1" shape="long" rot="R90"/>
<pad name="S3B" x="-4.325" y="-0.55" drill="0.6" diameter="0.9" stop="no" thermals="no"/>
<pad name="S3C" x="-4.325" y="-0.4125" drill="0.6" diameter="0.9" stop="no" thermals="no"/>
<pad name="S3D" x="-4.325" y="-0.275" drill="0.6" diameter="0.9" stop="no" thermals="no"/>
<pad name="S3E" x="-4.325" y="-0.1375" drill="0.6" diameter="0.9" stop="no" thermals="no"/>
<pad name="S3F" x="-4.325" y="0.1375" drill="0.6" diameter="0.9" stop="no" thermals="no"/>
<pad name="S3G" x="-4.325" y="0.275" drill="0.6" diameter="0.9" stop="no" thermals="no"/>
<pad name="S3H" x="-4.325" y="0.4125" drill="0.6" diameter="0.9" stop="no" thermals="no"/>
<pad name="S3I" x="-4.325" y="0.55" drill="0.6" diameter="0.9" stop="no" thermals="no"/>
<pad name="S4" x="4.325" y="0" drill="0.6" diameter="1" shape="long" rot="R90"/>
<pad name="S4B" x="4.325" y="-0.55" drill="0.6" diameter="0.9" stop="no" thermals="no"/>
<pad name="S4C" x="4.325" y="-0.4125" drill="0.6" diameter="0.9" stop="no" thermals="no"/>
<pad name="S4D" x="4.325" y="-0.275" drill="0.6" diameter="0.9" stop="no" thermals="no"/>
<pad name="S4E" x="4.325" y="-0.1375" drill="0.6" diameter="0.9" stop="no" thermals="no"/>
<pad name="S4F" x="4.325" y="0.1375" drill="0.6" diameter="0.9" stop="no" thermals="no"/>
<pad name="S4G" x="4.325" y="0.275" drill="0.6" diameter="0.9" stop="no" thermals="no"/>
<pad name="S4H" x="4.325" y="0.4125" drill="0.6" diameter="0.9" stop="no" thermals="no"/>
<pad name="S4I" x="4.325" y="0.55" drill="0.6" diameter="0.9" stop="no" thermals="no"/>
<wire x1="-4.325" y1="3.62" x2="-4.325" y2="4.72" width="0.6" layer="46"/>
<wire x1="4.325" y1="3.62" x2="4.325" y2="4.72" width="0.6" layer="46"/>
<wire x1="-4.325" y1="-0.55" x2="-4.325" y2="0.55" width="0.6" layer="46"/>
<wire x1="4.325" y1="-0.55" x2="4.325" y2="0.55" width="0.6" layer="46"/>
</package>
</packages>
<packages3d>
<package3d name="TYPE-C-31-M-12--3DMODEL-STEP-56544" urn="" wip_urn="urn:adsk.wipprod:fs.file:vf.VrcAKNroTJKLNHeQPZCpWA?version=2" locally_modified="yes" type="model">
<packageinstances>
<packageinstance name="HRO_TYPE-C-31-M-12"/>
</packageinstances>
</package3d>
</packages3d>
<symbols>
<symbol name="TYPE-C-31-M-12">
<wire x1="-12.7" y1="12.7" x2="12.7" y2="12.7" width="0.254" layer="94"/>
<wire x1="12.7" y1="12.7" x2="12.7" y2="-12.7" width="0.254" layer="94"/>
<wire x1="12.7" y1="-12.7" x2="-12.7" y2="-12.7" width="0.254" layer="94"/>
<wire x1="-12.7" y1="-12.7" x2="-12.7" y2="12.7" width="0.254" layer="94"/>
<text x="-12.7" y="13.462" size="1.778" layer="95">&gt;NAME</text>
<text x="-12.7" y="-13.462" size="1.778" layer="96" rot="MR180">&gt;VALUE</text>
<pin name="DP1" x="-17.78" y="2.54" length="middle"/>
<pin name="CC1" x="-17.78" y="5.08" length="middle"/>
<pin name="SBU1" x="-17.78" y="-2.54" length="middle"/>
<pin name="DN1" x="-17.78" y="0" length="middle"/>
<pin name="SHIELD" x="17.78" y="-7.62" length="middle" direction="pas" rot="R180"/>
<pin name="GND" x="17.78" y="-10.16" length="middle" direction="pwr" rot="R180"/>
<pin name="VBUS" x="17.78" y="10.16" length="middle" direction="pwr" rot="R180"/>
<pin name="DP2" x="17.78" y="0" length="middle" rot="R180"/>
<pin name="CC2" x="17.78" y="-2.54" length="middle" rot="R180"/>
<pin name="SBU2" x="17.78" y="5.08" length="middle" rot="R180"/>
<pin name="DN2" x="17.78" y="2.54" length="middle" rot="R180"/>
</symbol>
</symbols>
<devicesets>
<deviceset name="TYPE-C-31-M-12" prefix="J">
<description> &lt;a href="https://pricing.snapeda.com/parts/TYPE-C-31-M-12/HRO%20Electronics%20Co.%2C%20Ltd./view-part?ref=eda"&gt;Check availability&lt;/a&gt;</description>
<gates>
<gate name="G$1" symbol="TYPE-C-31-M-12" x="0" y="0"/>
</gates>
<devices>
<device name="" package="HRO_TYPE-C-31-M-12">
<connects>
<connect gate="G$1" pin="CC1" pad="A5"/>
<connect gate="G$1" pin="CC2" pad="B5"/>
<connect gate="G$1" pin="DN1" pad="A7"/>
<connect gate="G$1" pin="DN2" pad="B7"/>
<connect gate="G$1" pin="DP1" pad="A6"/>
<connect gate="G$1" pin="DP2" pad="B6"/>
<connect gate="G$1" pin="GND" pad="A1_B12 B1_A12"/>
<connect gate="G$1" pin="SBU1" pad="A8"/>
<connect gate="G$1" pin="SBU2" pad="B8"/>
<connect gate="G$1" pin="SHIELD" pad="S1 S2 S3 S4 S1B S1C S1D S1E S1F S1G S1H S1I S2B S2C S2D S2E S2F S2G S2H S2I S3B S3C S3D S3E S3F S3G S3H S3I S4B S4C S4D S4E S4F S4G S4H S4I"/>
<connect gate="G$1" pin="VBUS" pad="A4_B9 B4_A9"/>
</connects>

<package3dinstances>
<package3dinstance package3d_urn="urn:adsk.wipprod:fs.file:vf.VrcAKNroTJKLNHeQPZCpWA?version=2"/>
</package3dinstances>
<technologies>
<technology name="">
<attribute name="MF" value="HRO Electronics Co., Ltd."/>
<attribute name="DESCRIPTION" value="                                                      USB Connectors 24 Receptacle 1 8.94*7.3mm RoHS                                              "/>
<attribute name="PACKAGE" value="Package "/>
<attribute name="PRICE" value="None"/>
<attribute name="SNAPEDA_LINK" value="https://www.snapeda.com/parts/TYPE-C-31-M-12/HRO+Electronics+Co.%252C+Ltd./view-part/?ref=snap"/>
<attribute name="MP" value="TYPE-C-31-M-12"/>
<attribute name="AVAILABILITY" value="Not in stock"/>
<attribute name="CHECK_PRICES" value="https://www.snapeda.com/parts/TYPE-C-31-M-12/HRO+Electronics+Co.%252C+Ltd./view-part/?ref=eda"/>
</technology>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
<library name="USBLC6-2SC6" urn="urn:adsk.wipprod:fs.file:vf.6slYtN6HRKK38zIgSJqcUQ">
<packages>
<package name="SOT95P280X145-6N">
<wire x1="-0.8125" y1="1.4625" x2="0.8125" y2="1.4625" width="0.127" layer="51"/>
<wire x1="0.8125" y1="1.4625" x2="0.8125" y2="-1.4625" width="0.127" layer="51"/>
<wire x1="0.8125" y1="-1.4625" x2="-0.8125" y2="-1.4625" width="0.127" layer="51"/>
<wire x1="-0.8125" y1="-1.4625" x2="-0.8125" y2="1.4625" width="0.127" layer="51"/>
<wire x1="0.8125" y1="1.565" x2="-0.8125" y2="1.565" width="0.127" layer="21"/>
<wire x1="0.8125" y1="-1.565" x2="-0.8125" y2="-1.565" width="0.127" layer="21"/>
<wire x1="-2.11" y1="1.7125" x2="-2.11" y2="-1.7125" width="0.05" layer="39"/>
<wire x1="-2.11" y1="-1.7125" x2="2.11" y2="-1.7125" width="0.05" layer="39"/>
<wire x1="2.11" y1="-1.7125" x2="2.11" y2="1.7125" width="0.05" layer="39"/>
<wire x1="2.11" y1="1.7125" x2="-2.11" y2="1.7125" width="0.05" layer="39"/>
<circle x="-2.555" y="0.945" radius="0.1" width="0.2" layer="21"/>
<circle x="-2.555" y="0.945" radius="0.1" width="0.2" layer="51"/>
<text x="-2.11" y="2.0125" size="0.8128" layer="25">&gt;NAME</text>
<text x="-2.11" y="-1.9125" size="0.8128" layer="27" align="top-left">&gt;VALUE</text>
<smd name="1" x="-1.255" y="0.95" dx="1.21" dy="0.59" layer="1" roundness="25"/>
<smd name="2" x="-1.255" y="0" dx="1.21" dy="0.59" layer="1" roundness="25"/>
<smd name="3" x="-1.255" y="-0.95" dx="1.21" dy="0.59" layer="1" roundness="25"/>
<smd name="4" x="1.255" y="-0.95" dx="1.21" dy="0.59" layer="1" roundness="25"/>
<smd name="5" x="1.255" y="0" dx="1.21" dy="0.59" layer="1" roundness="25"/>
<smd name="6" x="1.255" y="0.95" dx="1.21" dy="0.59" layer="1" roundness="25"/>
</package>
</packages>
<packages3d>
<package3d name="SOT95P280X145-6N" urn="urn:adsk.eagle:package:18604126/4" type="model">
<packageinstances>
<packageinstance name="SOT95P280X145-6N"/>
</packageinstances>
</package3d>
</packages3d>
<symbols>
<symbol name="USBLC6-2SC6">
<text x="-15.2693" y="17.8093" size="1.78141875" layer="95">&gt;NAME</text>
<text x="-15.2559" y="-17.7959" size="1.77985" layer="96" align="top-left">&gt;VALUE</text>
<wire x1="-12.7" y1="7.62" x2="-7.62" y2="7.62" width="0.1524" layer="94"/>
<wire x1="-7.62" y1="7.62" x2="-7.62" y2="10.16" width="0.254" layer="94"/>
<wire x1="-7.62" y1="10.16" x2="-5.08" y2="7.62" width="0.254" layer="94"/>
<wire x1="-5.08" y1="7.62" x2="-7.62" y2="5.08" width="0.254" layer="94"/>
<wire x1="-7.62" y1="5.08" x2="-7.62" y2="7.62" width="0.254" layer="94"/>
<wire x1="-5.08" y1="10.16" x2="-5.08" y2="7.62" width="0.1524" layer="94"/>
<wire x1="-5.08" y1="7.62" x2="-5.08" y2="5.08" width="0.1524" layer="94"/>
<wire x1="-5.08" y1="7.62" x2="0" y2="7.62" width="0.1524" layer="94"/>
<wire x1="0" y1="7.62" x2="5.08" y2="7.62" width="0.1524" layer="94"/>
<wire x1="5.08" y1="7.62" x2="5.08" y2="10.16" width="0.254" layer="94"/>
<wire x1="5.08" y1="10.16" x2="7.62" y2="7.62" width="0.254" layer="94"/>
<wire x1="7.62" y1="7.62" x2="5.08" y2="5.08" width="0.254" layer="94"/>
<wire x1="5.08" y1="5.08" x2="5.08" y2="7.62" width="0.254" layer="94"/>
<wire x1="7.62" y1="10.16" x2="7.62" y2="7.62" width="0.254" layer="94"/>
<wire x1="7.62" y1="7.62" x2="7.62" y2="5.08" width="0.254" layer="94"/>
<wire x1="7.62" y1="7.62" x2="12.7" y2="7.62" width="0.1524" layer="94"/>
<polygon width="0.254" layer="94" pour="solid">
<vertex x="-7.62" y="10.15999375"/>
<vertex x="-7.62" y="5.08000625"/>
<vertex x="-5.08" y="7.62"/>
</polygon>
<polygon width="0.254" layer="94" pour="solid">
<vertex x="5.08" y="10.15999375"/>
<vertex x="5.08" y="5.08000625"/>
<vertex x="7.62" y="7.62"/>
</polygon>
<wire x1="-12.7" y1="-7.62" x2="-7.62" y2="-7.62" width="0.1524" layer="94"/>
<wire x1="-7.62" y1="-7.62" x2="-7.62" y2="-5.08" width="0.254" layer="94"/>
<wire x1="-7.62" y1="-5.08" x2="-5.08" y2="-7.62" width="0.254" layer="94"/>
<wire x1="-5.08" y1="-7.62" x2="-7.62" y2="-10.16" width="0.254" layer="94"/>
<wire x1="-7.62" y1="-10.16" x2="-7.62" y2="-7.62" width="0.254" layer="94"/>
<wire x1="-5.08" y1="-5.08" x2="-5.08" y2="-7.62" width="0.254" layer="94"/>
<wire x1="-5.08" y1="-7.62" x2="-5.08" y2="-10.16" width="0.254" layer="94"/>
<wire x1="-5.08" y1="-7.62" x2="0" y2="-7.62" width="0.1524" layer="94"/>
<wire x1="0" y1="-7.62" x2="5.08" y2="-7.62" width="0.1524" layer="94"/>
<wire x1="5.08" y1="-7.62" x2="5.08" y2="-5.08" width="0.254" layer="94"/>
<wire x1="5.08" y1="-5.08" x2="7.62" y2="-7.62" width="0.254" layer="94"/>
<wire x1="7.62" y1="-7.62" x2="5.08" y2="-10.16" width="0.254" layer="94"/>
<wire x1="5.08" y1="-10.16" x2="5.08" y2="-7.62" width="0.254" layer="94"/>
<wire x1="7.62" y1="-5.08" x2="7.62" y2="-7.62" width="0.254" layer="94"/>
<wire x1="7.62" y1="-7.62" x2="7.62" y2="-10.16" width="0.254" layer="94"/>
<wire x1="7.62" y1="-7.62" x2="12.7" y2="-7.62" width="0.1524" layer="94"/>
<polygon width="0.254" layer="94" pour="solid">
<vertex x="-7.62" y="-5.08000625"/>
<vertex x="-7.62" y="-10.15999375"/>
<vertex x="-5.08" y="-7.62"/>
</polygon>
<polygon width="0.254" layer="94" pour="solid">
<vertex x="5.08" y="-5.08000625"/>
<vertex x="5.08" y="-10.15999375"/>
<vertex x="7.62" y="-7.62"/>
</polygon>
<wire x1="-12.7" y1="0" x2="0" y2="0" width="0.1524" layer="94"/>
<wire x1="0" y1="0" x2="0" y2="2.54" width="0.254" layer="94"/>
<wire x1="0" y1="2.54" x2="2.54" y2="0" width="0.254" layer="94"/>
<wire x1="2.54" y1="0" x2="0" y2="-2.54" width="0.254" layer="94"/>
<wire x1="0" y1="-2.54" x2="0" y2="0" width="0.254" layer="94"/>
<wire x1="2.54" y1="0" x2="12.7" y2="0" width="0.1524" layer="94"/>
<wire x1="2.54" y1="2.54" x2="2.54" y2="-2.54" width="0.254" layer="94"/>
<wire x1="2.54" y1="2.54" x2="3.302" y2="3.302" width="0.254" layer="94"/>
<wire x1="2.54" y1="-2.54" x2="1.778" y2="-3.302" width="0.254" layer="94"/>
<wire x1="-12.7" y1="7.62" x2="-12.7" y2="0" width="0.1524" layer="94"/>
<wire x1="-12.7" y1="0" x2="-12.7" y2="-7.62" width="0.1524" layer="94"/>
<wire x1="12.7" y1="7.62" x2="12.7" y2="0" width="0.1524" layer="94"/>
<wire x1="12.7" y1="0" x2="12.7" y2="-7.62" width="0.1524" layer="94"/>
<wire x1="12.7" y1="0" x2="15.24" y2="0" width="0.1524" layer="94"/>
<wire x1="-12.7" y1="0" x2="-15.24" y2="0" width="0.1524" layer="94"/>
<polygon width="0.254" layer="94" pour="solid">
<vertex x="0" y="2.53999375"/>
<vertex x="0" y="-2.53999375"/>
<vertex x="2.54" y="0"/>
</polygon>
<circle x="0" y="-12.7" radius="0.254" width="0.381" layer="94"/>
<wire x1="0" y1="-7.62" x2="0" y2="-12.7" width="0.1524" layer="94"/>
<wire x1="0" y1="7.62" x2="0" y2="12.7" width="0.1524" layer="94"/>
<circle x="-12.7" y="0" radius="0.254" width="0.381" layer="94"/>
<circle x="12.7" y="0" radius="0.254" width="0.381" layer="94"/>
<circle x="0" y="12.7" radius="0.254" width="0.381" layer="94"/>
<circle x="0" y="7.62" radius="0.254" width="0.381" layer="94"/>
<circle x="0" y="-7.62" radius="0.254" width="0.381" layer="94"/>
<wire x1="-15.24" y1="12.7" x2="0" y2="12.7" width="0.1524" layer="94"/>
<wire x1="0" y1="12.7" x2="15.24" y2="12.7" width="0.1524" layer="94"/>
<wire x1="-15.24" y1="-12.7" x2="0" y2="-12.7" width="0.1524" layer="94"/>
<wire x1="15.24" y1="-12.7" x2="0" y2="-12.7" width="0.1524" layer="94"/>
<wire x1="-15.24" y1="15.24" x2="15.24" y2="15.24" width="0.254" layer="94"/>
<wire x1="15.24" y1="15.24" x2="15.24" y2="-15.24" width="0.254" layer="94"/>
<wire x1="15.24" y1="-15.24" x2="-15.24" y2="-15.24" width="0.254" layer="94"/>
<wire x1="-15.24" y1="-15.24" x2="-15.24" y2="15.24" width="0.254" layer="94"/>
<pin name="GND" x="-20.32" y="0" visible="pad" length="middle" direction="pwr"/>
<pin name="VBUS" x="20.32" y="0" visible="pad" length="middle" direction="pwr" rot="R180"/>
<pin name="I/O1_IN" x="-20.32" y="12.7" visible="pad" length="middle"/>
<pin name="I/O2_IN" x="-20.32" y="-12.7" visible="pad" length="middle"/>
<pin name="I/O1_OUT" x="20.32" y="12.7" visible="pad" length="middle" rot="R180"/>
<pin name="I/O2_OUT" x="20.32" y="-12.7" visible="pad" length="middle" rot="R180"/>
</symbol>
</symbols>
<devicesets>
<deviceset name="USBLC6-2SC6" prefix="U">
<description> &lt;a href="https://pricing.snapeda.com/parts/USBLC6-2SC6/STMicroelectronics/view-part?ref=eda"&gt;Check availability&lt;/a&gt;</description>
<gates>
<gate name="G$1" symbol="USBLC6-2SC6" x="0" y="0"/>
</gates>
<devices>
<device name="" package="SOT95P280X145-6N">
<connects>
<connect gate="G$1" pin="GND" pad="2"/>
<connect gate="G$1" pin="I/O1_IN" pad="1"/>
<connect gate="G$1" pin="I/O1_OUT" pad="6"/>
<connect gate="G$1" pin="I/O2_IN" pad="3"/>
<connect gate="G$1" pin="I/O2_OUT" pad="4"/>
<connect gate="G$1" pin="VBUS" pad="5"/>
</connects>

<package3dinstances>
<package3dinstance package3d_urn="urn:adsk.eagle:package:18604126/4"/>
</package3dinstances>
<technologies>
<technology name="">
<attribute name="MF" value="STMicroelectronics"/>
<attribute name="DESCRIPTION" value="                                                      17V Clamp 5A (8/20µs) Ipp Tvs Diode Surface Mount SOT-23-6                                              "/>
<attribute name="PACKAGE" value="SOT-23-6 STMicroelectronics"/>
<attribute name="PRICE" value="None"/>
<attribute name="CHECK_PRICES" value="https://www.snapeda.com/parts/USBLC6-2SC6/STMicroelectronics/view-part/?ref=eda"/>
<attribute name="SNAPEDA_LINK" value="https://www.snapeda.com/parts/USBLC6-2SC6/STMicroelectronics/view-part/?ref=snap"/>
<attribute name="MP" value="USBLC6-2SC6"/>
<attribute name="AVAILABILITY" value="In Stock"/>
<attribute name="PURCHASE-URL" value="https://www.snapeda.com/api/url_track_click_mouser/?unipart_id=231887&amp;manufacturer=STMicroelectronics&amp;part_name=USBLC6-2SC6&amp;search_term= usblc6-2sc6"/>
</technology>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
<library name="AO3400A" urn="urn:adsk.wipprod:fs.file:vf.206TU6vcR6CgwUWcVQ0mjQ">
<packages>
<package name="SOT95P280X125-3N">
<wire x1="-0.9" y1="1.55" x2="-0.9" y2="-1.55" width="0.127" layer="51"/>
<wire x1="-0.9" y1="-1.55" x2="0.9" y2="-1.55" width="0.127" layer="21"/>
<wire x1="0.9" y1="-1.55" x2="0.9" y2="1.55" width="0.127" layer="51"/>
<wire x1="0.9" y1="1.55" x2="-0.9" y2="1.55" width="0.127" layer="21"/>
<wire x1="-0.9" y1="-1.55" x2="0.9" y2="-1.55" width="0.127" layer="51"/>
<wire x1="0.9" y1="1.55" x2="-0.9" y2="1.55" width="0.127" layer="51"/>
<circle x="-2.31" y="0.95" radius="0.1" width="0.2" layer="21"/>
<circle x="-2.31" y="0.95" radius="0.1" width="0.2" layer="51"/>
<wire x1="-2.11" y1="-1.8" x2="-2.11" y2="1.8" width="0.05" layer="39"/>
<wire x1="-2.11" y1="1.8" x2="2.11" y2="1.8" width="0.05" layer="39"/>
<wire x1="2.11" y1="1.8" x2="2.11" y2="-1.8" width="0.05" layer="39"/>
<wire x1="2.11" y1="-1.8" x2="-2.11" y2="-1.8" width="0.05" layer="39"/>
<text x="-2" y="2" size="1.27" layer="25">&gt;NAME</text>
<text x="-2" y="-3.25" size="1.27" layer="27">&gt;VALUE</text>
<smd name="1" x="-1.255" y="0.95" dx="1.21" dy="0.59" layer="1" roundness="50"/>
<smd name="3" x="1.255" y="0" dx="1.21" dy="0.59" layer="1" roundness="50"/>
<smd name="2" x="-1.255" y="-0.95" dx="1.21" dy="0.59" layer="1" roundness="50"/>
</package>
</packages>
<packages3d>
<package3d name="SOT23" urn="urn:adsk.eagle:package:46660660/4" type="model">
<packageinstances>
<packageinstance name="SOT95P280X125-3N"/>
</packageinstances>
</package3d>
</packages3d>
<symbols>
<symbol name="AO3400A">
<wire x1="0.762" y1="0.762" x2="0.762" y2="0" width="0.254" layer="94"/>
<wire x1="0.762" y1="0" x2="0.762" y2="-0.762" width="0.254" layer="94"/>
<wire x1="0.762" y1="3.175" x2="0.762" y2="2.54" width="0.254" layer="94"/>
<wire x1="0.762" y1="2.54" x2="0.762" y2="1.905" width="0.254" layer="94"/>
<wire x1="0.762" y1="0" x2="2.54" y2="0" width="0.1524" layer="94"/>
<wire x1="2.54" y1="0" x2="2.54" y2="-2.54" width="0.1524" layer="94"/>
<wire x1="0.762" y1="-1.905" x2="0.762" y2="-2.54" width="0.254" layer="94"/>
<wire x1="0.762" y1="-2.54" x2="0.762" y2="-3.175" width="0.254" layer="94"/>
<wire x1="0" y1="2.54" x2="0" y2="-2.54" width="0.254" layer="94"/>
<wire x1="2.54" y1="-2.54" x2="0.762" y2="-2.54" width="0.1524" layer="94"/>
<wire x1="3.81" y1="2.54" x2="3.81" y2="0.508" width="0.1524" layer="94"/>
<wire x1="3.81" y1="0.508" x2="3.81" y2="-2.54" width="0.1524" layer="94"/>
<wire x1="2.54" y1="-2.54" x2="3.81" y2="-2.54" width="0.1524" layer="94"/>
<wire x1="0.762" y1="2.54" x2="3.81" y2="2.54" width="0.1524" layer="94"/>
<wire x1="4.318" y1="0.508" x2="3.81" y2="0.508" width="0.1524" layer="94"/>
<wire x1="3.81" y1="0.508" x2="3.302" y2="0.508" width="0.1524" layer="94"/>
<circle x="2.54" y="-2.54" radius="0.3592" width="0" layer="94"/>
<circle x="2.54" y="2.54" radius="0.3592" width="0" layer="94"/>
<text x="-8.89" y="-7.62" size="1.778" layer="96">&gt;VALUE</text>
<text x="-8.89" y="2.54" size="1.778" layer="95">&gt;NAME</text>
<polygon width="0.1524" layer="94" pour="solid">
<vertex x="3.302" y="-0.254"/>
<vertex x="4.318" y="-0.254"/>
<vertex x="3.81" y="0.508003125"/>
</polygon>
<polygon width="0.1524" layer="94" pour="solid">
<vertex x="1.015996875" y="0"/>
<vertex x="2.032" y="-0.76199375"/>
<vertex x="2.032" y="0.76199375"/>
</polygon>
<pin name="S" x="2.54" y="-5.08" visible="off" length="short" direction="pas" rot="R90"/>
<pin name="G" x="-2.54" y="-2.54" visible="off" length="short" direction="pas"/>
<pin name="D" x="2.54" y="5.08" visible="off" length="short" direction="pas" rot="R270"/>
</symbol>
</symbols>
<devicesets>
<deviceset name="AO3400A" prefix="Q">
<description> &lt;a href="https://pricing.snapeda.com/parts/AO3400A/UMW/view-part?ref=eda"&gt;Check availability&lt;/a&gt;</description>
<gates>
<gate name="G$1" symbol="AO3400A" x="0" y="0"/>
</gates>
<devices>
<device name="" package="SOT95P280X125-3N">
<connects>
<connect gate="G$1" pin="D" pad="3"/>
<connect gate="G$1" pin="G" pad="1"/>
<connect gate="G$1" pin="S" pad="2"/>
</connects>

<package3dinstances>
<package3dinstance package3d_urn="urn:adsk.eagle:package:46660660/4"/>
</package3dinstances>
<technologies>
<technology name="">
<attribute name="MF" value="UMW"/>
<attribute name="DESCRIPTION" value="                                                      N-Channel 30 V 5.8A (Ta) 1.4W (Ta) Surface Mount SOT-23                                              "/>
<attribute name="PACKAGE" value="None"/>
<attribute name="PRICE" value="None"/>
<attribute name="SNAPEDA_LINK" value="https://www.snapeda.com/parts/AO3400A/UMW/view-part/?ref=snap"/>
<attribute name="MP" value="AO3400A"/>
<attribute name="AVAILABILITY" value="In Stock"/>
<attribute name="CHECK_PRICES" value="https://www.snapeda.com/parts/AO3400A/UMW/view-part/?ref=eda"/>
</technology>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
<library name="TS-1187A" urn="urn:adsk.wipprod:fs.file:vf.xAAbsUAnTw6G54G6FC4J_g">
<packages>
<package name="TACT" library_version="1">
<text x="0" y="0" size="1" layer="25" align="center">&gt;NAME</text>
<smd name="1" x="-3" y="1.875" dx="1" dy="0.75" layer="1"/>
<smd name="2" x="3" y="1.875" dx="1" dy="0.75" layer="1"/>
<smd name="3" x="-3" y="-1.875" dx="1" dy="0.75" layer="1"/>
<smd name="4" x="3" y="-1.875" dx="1" dy="0.75" layer="1"/>
<wire x1="-2.55" y1="2.55" x2="2.55" y2="2.55" width="0.127" layer="21"/>
<wire x1="-2.55" y1="-2.55" x2="2.55" y2="-2.55" width="0.127" layer="21"/>
<wire x1="-2.55" y1="1.2" x2="-2.55" y2="-1.2" width="0.127" layer="21"/>
<wire x1="2.55" y1="1.2" x2="2.55" y2="-1.2" width="0.127" layer="21"/>
<circle x="0" y="0" radius="1" width="0.127" layer="21"/>
<wire x1="-2.55" y1="-2.55" x2="2.55" y2="-2.55" width="0.12" layer="51"/>
<wire x1="2.55" y1="-2.55" x2="2.55" y2="2.55" width="0.12" layer="51"/>
<wire x1="2.55" y1="2.55" x2="-2.55" y2="2.55" width="0.12" layer="51"/>
<wire x1="-2.55" y1="2.55" x2="-2.55" y2="-2.55" width="0.12" layer="51"/>
</package>
</packages>
<packages3d>
<package3d name="TS-1187A-B-A-B--3DMODEL-STEP-56544" urn="" wip_urn="urn:adsk.wipprod:fs.file:vf.a9rK-YRvTTGKbOQY9CMMJw?version=2" locally_modified="yes" type="model">
<packageinstances>
<packageinstance name="TACT"/>
</packageinstances>
</package3d>
</packages3d>
<symbols>
<symbol name="SWITCH">
<wire x1="-10.16" y1="-2.54" x2="10.16" y2="-2.54" width="0.254" layer="94"/>
<wire x1="10.16" y1="-2.54" x2="10.16" y2="2.54" width="0.254" layer="94"/>
<wire x1="10.16" y1="2.54" x2="-10.16" y2="2.54" width="0.254" layer="94"/>
<wire x1="-10.16" y1="2.54" x2="-10.16" y2="-2.54" width="0.254" layer="94"/>
<text x="-10.16" y="3.54" size="1.778" layer="95">&gt;NAME</text>
<text x="-10.16" y="-5.34" size="1.778" layer="96">&gt;VALUE</text>
<pin name="1" x="-12.7" y="0" visible="pin" length="point" direction="pas"/>
<pin name="2" x="12.7" y="0" visible="pin" length="point" direction="pas" rot="R180"/>
</symbol>
</symbols>
<devicesets>
<deviceset name="SW_TACT" prefix="SW" uservalue="yes">
<gates>
<gate name="G$1" symbol="SWITCH" x="0" y="0"/>
</gates>
<devices>
<device name="" package="TACT">
<connects>
<connect gate="G$1" pin="1" pad="1 2"/>
<connect gate="G$1" pin="2" pad="3 4"/>
</connects>

<package3dinstances>
<package3dinstance package3d_urn="urn:adsk.wipprod:fs.file:vf.a9rK-YRvTTGKbOQY9CMMJw?version=2"/>
</package3dinstances>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
<library name="JST-XH-BL" urn="urn:adsk.wipprod:fs.file:vf.UqJVv87FT1KjZqR4MQAHEw">
<packages>
<package name="XH2">
<text x="0" y="0" size="1" layer="25" align="center">&gt;NAME</text>
<pad name="1" x="-1.25" y="0" drill="0.95" diameter="1.7" shape="square"/>
<pad name="2" x="1.25" y="0" drill="0.95" diameter="1.7"/>
<wire x1="-3.7" y1="-2.35" x2="3.7" y2="-2.35" width="0.127" layer="21"/>
<wire x1="3.7" y1="-2.35" x2="3.7" y2="3.4" width="0.127" layer="21"/>
<wire x1="3.7" y1="3.4" x2="-3.7" y2="3.4" width="0.127" layer="21"/>
<wire x1="-3.7" y1="3.4" x2="-3.7" y2="-2.35" width="0.127" layer="21"/>
<wire x1="-3.7" y1="-2.35" x2="3.7" y2="-2.35" width="0.12" layer="51"/>
<wire x1="3.7" y1="-2.35" x2="3.7" y2="3.4" width="0.12" layer="51"/>
<wire x1="3.7" y1="3.4" x2="-3.7" y2="3.4" width="0.12" layer="51"/>
<wire x1="-3.7" y1="3.4" x2="-3.7" y2="-2.35" width="0.12" layer="51"/>
</package>
<package name="XH3">
<text x="0" y="0" size="1" layer="25" align="center">&gt;NAME</text>
<pad name="1" x="-2.5" y="0" drill="0.95" diameter="1.7" shape="square"/>
<pad name="2" x="0" y="0" drill="0.95" diameter="1.7"/>
<pad name="3" x="2.5" y="0" drill="0.95" diameter="1.7"/>
<wire x1="-4.95" y1="-2.35" x2="4.95" y2="-2.35" width="0.127" layer="21"/>
<wire x1="4.95" y1="-2.35" x2="4.95" y2="3.4" width="0.127" layer="21"/>
<wire x1="4.95" y1="3.4" x2="-4.95" y2="3.4" width="0.127" layer="21"/>
<wire x1="-4.95" y1="3.4" x2="-4.95" y2="-2.35" width="0.127" layer="21"/>
<wire x1="-4.95" y1="-2.35" x2="4.95" y2="-2.35" width="0.12" layer="51"/>
<wire x1="4.95" y1="-2.35" x2="4.95" y2="3.4" width="0.12" layer="51"/>
<wire x1="4.95" y1="3.4" x2="-4.95" y2="3.4" width="0.12" layer="51"/>
<wire x1="-4.95" y1="3.4" x2="-4.95" y2="-2.35" width="0.12" layer="51"/>
</package>
</packages>
<packages3d>
<package3d name="B2B-XH-A--3DMODEL-STEP-56544" urn="" wip_urn="urn:adsk.wipprod:fs.file:vf.HiWWKYzERta-VOpw5EMMBA?version=1" locally_modified="yes" type="model">
<packageinstances>
<packageinstance name="XH2"/>
</packageinstances>
</package3d>
<package3d name="B3B-XH-A--3DMODEL-STEP-56544" urn="" wip_urn="urn:adsk.wipprod:fs.file:vf.cDk4rO_yQsSxQZYxAG73Fg?version=1" locally_modified="yes" type="model">
<packageinstances>
<packageinstance name="XH3"/>
</packageinstances>
</package3d>
</packages3d>
<symbols>
<symbol name="CONN_LED">
<wire x1="-10.16" y1="-3.81" x2="10.16" y2="-3.81" width="0.254" layer="94"/>
<wire x1="10.16" y1="-3.81" x2="10.16" y2="3.81" width="0.254" layer="94"/>
<wire x1="10.16" y1="3.81" x2="-10.16" y2="3.81" width="0.254" layer="94"/>
<wire x1="-10.16" y1="3.81" x2="-10.16" y2="-3.81" width="0.254" layer="94"/>
<text x="-10.16" y="4.81" size="1.778" layer="95">&gt;NAME</text>
<text x="-10.16" y="-6.61" size="1.778" layer="96">&gt;VALUE</text>
<pin name="VP" x="-12.7" y="1.27" visible="pin" length="point" direction="pas"/>
<pin name="W" x="-12.7" y="-1.27" visible="pin" length="point" direction="pas"/>
<pin name="C" x="12.7" y="0" visible="pin" length="point" direction="pas" rot="R180"/>
</symbol>
<symbol name="CONN_KNOB">
<wire x1="-10.16" y1="-3.81" x2="10.16" y2="-3.81" width="0.254" layer="94"/>
<wire x1="10.16" y1="-3.81" x2="10.16" y2="3.81" width="0.254" layer="94"/>
<wire x1="10.16" y1="3.81" x2="-10.16" y2="3.81" width="0.254" layer="94"/>
<wire x1="-10.16" y1="3.81" x2="-10.16" y2="-3.81" width="0.254" layer="94"/>
<text x="-10.16" y="4.81" size="1.778" layer="95">&gt;NAME</text>
<text x="-10.16" y="-6.61" size="1.778" layer="96">&gt;VALUE</text>
<pin name="A" x="-12.7" y="1.27" visible="pin" length="point" direction="pas"/>
<pin name="B" x="-12.7" y="-1.27" visible="pin" length="point" direction="pas"/>
<pin name="GND" x="12.7" y="0" visible="pin" length="point" direction="pas" rot="R180"/>
</symbol>
<symbol name="CONN_BTN">
<wire x1="-10.16" y1="-2.54" x2="10.16" y2="-2.54" width="0.254" layer="94"/>
<wire x1="10.16" y1="-2.54" x2="10.16" y2="2.54" width="0.254" layer="94"/>
<wire x1="10.16" y1="2.54" x2="-10.16" y2="2.54" width="0.254" layer="94"/>
<wire x1="-10.16" y1="2.54" x2="-10.16" y2="-2.54" width="0.254" layer="94"/>
<text x="-10.16" y="3.54" size="1.778" layer="95">&gt;NAME</text>
<text x="-10.16" y="-5.34" size="1.778" layer="96">&gt;VALUE</text>
<pin name="SW" x="-12.7" y="0" visible="pin" length="point" direction="pas"/>
<pin name="GND" x="12.7" y="0" visible="pin" length="point" direction="pas" rot="R180"/>
</symbol>
</symbols>
<devicesets>
<deviceset name="CONN_LED_DS" prefix="J" uservalue="yes">
<gates>
<gate name="G$1" symbol="CONN_LED" x="0" y="0"/>
</gates>
<devices>
<device name="" package="XH3">
<connects>
<connect gate="G$1" pin="C" pad="3"/>
<connect gate="G$1" pin="VP" pad="1"/>
<connect gate="G$1" pin="W" pad="2"/>
</connects>

<package3dinstances>
<package3dinstance package3d_urn="urn:adsk.wipprod:fs.file:vf.cDk4rO_yQsSxQZYxAG73Fg?version=1"/>
</package3dinstances>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="CONN_KNOB_DS" prefix="J" uservalue="yes">
<gates>
<gate name="G$1" symbol="CONN_KNOB" x="0" y="0"/>
</gates>
<devices>
<device name="" package="XH3">
<connects>
<connect gate="G$1" pin="A" pad="1"/>
<connect gate="G$1" pin="B" pad="2"/>
<connect gate="G$1" pin="GND" pad="3"/>
</connects>

<package3dinstances>
<package3dinstance package3d_urn="urn:adsk.wipprod:fs.file:vf.cDk4rO_yQsSxQZYxAG73Fg?version=1"/>
</package3dinstances>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="CONN_BTN_DS" prefix="J" uservalue="yes">
<gates>
<gate name="G$1" symbol="CONN_BTN" x="0" y="0"/>
</gates>
<devices>
<device name="" package="XH2">
<connects>
<connect gate="G$1" pin="GND" pad="2"/>
<connect gate="G$1" pin="SW" pad="1"/>
</connects>

<package3dinstances>
<package3dinstance package3d_urn="urn:adsk.wipprod:fs.file:vf.HiWWKYzERta-VOpw5EMMBA?version=1"/>
</package3dinstances>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
<library name="KF350-SENSOR" urn="urn:adsk.wipprod:fs.file:vf.9G7HKEyURbCUNxv_2OvNIA">
<packages>
<package name="SCREW5">
<text x="0" y="0" size="1" layer="25" align="center">&gt;NAME</text>
<pad name="1" x="-7" y="0" drill="1.3" diameter="2.2" shape="square"/>
<pad name="2" x="-3.5" y="0" drill="1.3" diameter="2.2"/>
<pad name="3" x="0" y="0" drill="1.3" diameter="2.2"/>
<pad name="4" x="3.5" y="0" drill="1.3" diameter="2.2"/>
<pad name="5" x="7" y="0" drill="1.3" diameter="2.2"/>
<wire x1="-8.75" y1="-3.4" x2="8.75" y2="-3.4" width="0.127" layer="21"/>
<wire x1="8.75" y1="-3.4" x2="8.75" y2="3.5" width="0.127" layer="21"/>
<wire x1="8.75" y1="3.5" x2="-8.75" y2="3.5" width="0.127" layer="21"/>
<wire x1="-8.75" y1="3.5" x2="-8.75" y2="-3.4" width="0.127" layer="21"/>
<wire x1="-8.75" y1="-3.4" x2="8.75" y2="-3.4" width="0.12" layer="51"/>
<wire x1="8.75" y1="-3.4" x2="8.75" y2="3.5" width="0.12" layer="51"/>
<wire x1="8.75" y1="3.5" x2="-8.75" y2="3.5" width="0.12" layer="51"/>
<wire x1="-8.75" y1="3.5" x2="-8.75" y2="-3.4" width="0.12" layer="51"/>
</package>
</packages>
<packages3d>
<package3d name="SCREW5" urn="" wip_urn="urn:adsk.wipprod:fs.file:vf.8665xnF7T7yu4-AMVkdEwQ?version=2" locally_modified="yes" type="model">
<packageinstances>
<packageinstance name="SCREW5"/>
</packageinstances>
</package3d>
</packages3d>
<symbols>
<symbol name="CONN_SENSOR">
<wire x1="-10.16" y1="-5.08" x2="10.16" y2="-5.08" width="0.254" layer="94"/>
<wire x1="10.16" y1="-5.08" x2="10.16" y2="5.08" width="0.254" layer="94"/>
<wire x1="10.16" y1="5.08" x2="-10.16" y2="5.08" width="0.254" layer="94"/>
<wire x1="-10.16" y1="5.08" x2="-10.16" y2="-5.08" width="0.254" layer="94"/>
<text x="-10.16" y="6.08" size="1.778" layer="95">&gt;NAME</text>
<text x="-10.16" y="-7.88" size="1.778" layer="96">&gt;VALUE</text>
<pin name="VCC" x="-12.7" y="2.54" visible="pin" length="point" direction="pas"/>
<pin name="GND" x="-12.7" y="0" visible="pin" length="point" direction="pas"/>
<pin name="A" x="-12.7" y="-2.54" visible="pin" length="point" direction="pas"/>
<pin name="B" x="12.7" y="1.27" visible="pin" length="point" direction="pas" rot="R180"/>
<pin name="Z" x="12.7" y="-1.27" visible="pin" length="point" direction="pas" rot="R180"/>
</symbol>
</symbols>
<devicesets>
<deviceset name="CONN_SENSOR_DS" prefix="J" uservalue="yes">
<gates>
<gate name="G$1" symbol="CONN_SENSOR" x="0" y="0"/>
</gates>
<devices>
<device name="" package="SCREW5">
<connects>
<connect gate="G$1" pin="A" pad="3"/>
<connect gate="G$1" pin="B" pad="4"/>
<connect gate="G$1" pin="GND" pad="2"/>
<connect gate="G$1" pin="VCC" pad="1"/>
<connect gate="G$1" pin="Z" pad="5"/>
</connects>

<package3dinstances>
<package3dinstance package3d_urn="urn:adsk.wipprod:fs.file:vf.8665xnF7T7yu4-AMVkdEwQ?version=2"/>
</package3dinstances>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
</libraries>
<attributes>
</attributes>
<variantdefs>
</variantdefs>
<classes>
<class number="0" name="default" width="0" drill="0">
</class>
<class number="1" name="pwr24" width="0.4" drill="0">
</class>
<class number="2" name="pwr3v3" width="0.25" drill="0">
</class>
</classes>
<parts>
<part name="C_MCU1" library="BLLIB" deviceset="CAP_0805" device="" value="10u" package3d_urn="urn:adsk.eagle:package:16290897/6"/>
<part name="C_MCU2" library="BLLIB" deviceset="CAP_0603" device="" value="1u" package3d_urn="urn:adsk.eagle:package:16290898/6"/>
<part name="C_MCU3" library="BLLIB" deviceset="CAP_0402" device="" value="100n" package3d_urn="urn:adsk.eagle:package:16290895/6"/>
<part name="C_MCU4" library="BLLIB" deviceset="CAP_0402" device="" value="100n" package3d_urn="urn:adsk.eagle:package:16290895/6"/>
<part name="C_MCU_BULK" library="BLLIB" deviceset="CAP_D" device="" value="470u" package3d_urn="urn:adsk.eagle:package:16290885/4"/>
<part name="D_MCU" library="BLLIB" deviceset="DIODE_SOD323" device="" value="ESD" package3d_urn="urn:adsk.eagle:package:10898396/3"/>
<part name="R_EN" library="BLLIB" deviceset="RES_0603" device="" value="10k" package3d_urn="urn:adsk.eagle:package:16378565/7"/>
<part name="C_EN" library="BLLIB" deviceset="CAP_0603" device="" value="1u" package3d_urn="urn:adsk.eagle:package:16290898/6"/>
<part name="SW_RST" library="TS-1187A" library_urn="urn:adsk.wipprod:fs.file:vf.xAAbsUAnTw6G54G6FC4J_g" deviceset="SW_TACT" device="" value="RESET" package3d_urn="urn:adsk.wipprod:fs.file:vf.a9rK-YRvTTGKbOQY9CMMJw?version=2"/>
<part name="R_BOOT" library="BLLIB" deviceset="RES_0603" device="" value="10k" package3d_urn="urn:adsk.eagle:package:16378565/7"/>
<part name="SW_BOOT" library="TS-1187A" library_urn="urn:adsk.wipprod:fs.file:vf.xAAbsUAnTw6G54G6FC4J_g" deviceset="SW_TACT" device="" value="BOOT" package3d_urn="urn:adsk.wipprod:fs.file:vf.a9rK-YRvTTGKbOQY9CMMJw?version=2"/>
<part name="C_BOOT" library="BLLIB" deviceset="CAP_0402" device="" value="100p" package3d_urn="urn:adsk.eagle:package:16290895/6"/>
<part name="R_CC1" library="BLLIB" deviceset="RES_0603" device="" value="5.1k" package3d_urn="urn:adsk.eagle:package:16378565/7"/>
<part name="R_CC2" library="BLLIB" deviceset="RES_0603" device="" value="5.1k" package3d_urn="urn:adsk.eagle:package:16378565/7"/>
<part name="R_DP" library="BLLIB" deviceset="RES_0603" device="" value="22R" package3d_urn="urn:adsk.eagle:package:16378565/7"/>
<part name="R_DM" library="BLLIB" deviceset="RES_0603" device="" value="22R" package3d_urn="urn:adsk.eagle:package:16378565/7"/>
<part name="C_VBUS" library="BLLIB" deviceset="CAP_0603" device="" value="4.7u" package3d_urn="urn:adsk.eagle:package:16290898/6"/>
<part name="D_OR_USB" library="BLLIB" deviceset="DIODE_SMA" device="" value="SS14" package3d_urn="urn:adsk.eagle:package:48498326/1"/>
<part name="J_PROG" library="BLLIB" deviceset="CONN_PROG_DS" device="" value="UART-FALLBACK" package3d_urn="urn:adsk.eagle:package:51802562/2"/>
<part name="J_PWR" library="BLLIB" deviceset="CONN_PWR_DS" device="" value="24V-IN" package3d_urn="urn:adsk.eagle:package:51802550/2"/>
<part name="F1" library="BLLIB" deviceset="FUSE_NANO2" device="" value="2A" package3d_urn="urn:adsk.eagle:package:16378157/3"/>
<part name="Q1" library="BLLIB" deviceset="FET_SOT23" device="" value="PMOS-RevProt" package3d_urn="urn:adsk.eagle:package:46660660/4"/>
<part name="R_Q1" library="BLLIB" deviceset="RES_0603" device="" value="100k" package3d_urn="urn:adsk.eagle:package:16378565/7"/>
<part name="D_Q1" library="BLLIB" deviceset="DIODE_SOD123" device="" value="10V-Zener" package3d_urn="urn:adsk.eagle:package:10898395/2"/>
<part name="TVS1" library="BLLIB" deviceset="DIODE_SMB" device="" value="SMBJ28A" package3d_urn="urn:adsk.eagle:package:48498325/1"/>
<part name="C_IN_BULK" library="BLLIB" deviceset="CAP_V" device="" value="100u-50V" package3d_urn="urn:adsk.eagle:package:16290889/4"/>
<part name="U_BUCK" library="BLLIB" deviceset="BUCK_DS" device="" value="LMR51430" package3d_urn="urn:adsk.eagle:package:18604126/4"/>
<part name="C_IN1" library="BLLIB" deviceset="CAP_1210" device="" value="10u-50V" package3d_urn="urn:adsk.eagle:package:16290903/6"/>
<part name="C_IN2" library="BLLIB" deviceset="CAP_1206" device="" value="4.7u-50V" package3d_urn="urn:adsk.eagle:package:16290893/6"/>
<part name="R_ENB" library="BLLIB" deviceset="RES_0603" device="" value="100k" package3d_urn="urn:adsk.eagle:package:16378565/7"/>
<part name="L1" library="BLLIB" deviceset="IND_SMD" device="" value="4.7uH" package3d_urn="urn:adsk.eagle:package:16378474/4"/>
<part name="C_BOOT_B" library="BLLIB" deviceset="CAP_0402" device="" value="100n" package3d_urn="urn:adsk.eagle:package:16290895/6"/>
<part name="C_OUT1" library="BLLIB" deviceset="CAP_0805" device="" value="22u" package3d_urn="urn:adsk.eagle:package:16290897/6"/>
<part name="C_OUT2" library="BLLIB" deviceset="CAP_0805" device="" value="22u" package3d_urn="urn:adsk.eagle:package:16290897/6"/>
<part name="R_FBT" library="BLLIB" deviceset="RES_0603" device="" value="45.3k" package3d_urn="urn:adsk.eagle:package:16378565/7"/>
<part name="R_FBB" library="BLLIB" deviceset="RES_0603" device="" value="10k" package3d_urn="urn:adsk.eagle:package:16378565/7"/>
<part name="C_FF" library="BLLIB" deviceset="CAP_0402" device="" value="22p" package3d_urn="urn:adsk.eagle:package:16290895/6"/>
<part name="C_CP" library="BLLIB" deviceset="CAP_0603" device="" value="10n" package3d_urn="urn:adsk.eagle:package:16290898/6"/>
<part name="C_VCP" library="BLLIB" deviceset="CAP_0603" device="" value="100n" package3d_urn="urn:adsk.eagle:package:16290898/6"/>
<part name="C_V3P3" library="BLLIB" deviceset="CAP_0603" device="" value="470n" package3d_urn="urn:adsk.eagle:package:16290898/6"/>
<part name="C_VM1" library="BLLIB" deviceset="CAP_0603" device="" value="100n-50V" package3d_urn="urn:adsk.eagle:package:16290898/6"/>
<part name="C_VM2" library="BLLIB" deviceset="CAP_0603" device="" value="100n-50V" package3d_urn="urn:adsk.eagle:package:16290898/6"/>
<part name="C_VM_BULK" library="BLLIB" deviceset="CAP_V" device="" value="100u-50V" package3d_urn="urn:adsk.eagle:package:16290889/4"/>
<part name="R_NSLEEP" library="BLLIB" deviceset="RES_0603" device="" value="10k" package3d_urn="urn:adsk.eagle:package:16378565/7"/>
<part name="R_NRESET" library="BLLIB" deviceset="RES_0603" device="" value="10k" package3d_urn="urn:adsk.eagle:package:16378565/7"/>
<part name="R_NFAULT" library="BLLIB" deviceset="RES_0603" device="" value="10k" package3d_urn="urn:adsk.eagle:package:16378565/7"/>
<part name="R_EN_DRV" library="BLLIB" deviceset="RES_0603" device="" value="10k" package3d_urn="urn:adsk.eagle:package:16378565/7"/>
<part name="J_MOTOR" library="BLLIB" deviceset="CONN_MOTOR_DS" device="" value="MOTOR" package3d_urn="urn:adsk.eagle:package:51802625/2"/>
<part name="J_SENSOR" library="KF350-SENSOR" library_urn="urn:adsk.wipprod:fs.file:vf.9G7HKEyURbCUNxv_2OvNIA" package3d_urn="urn:adsk.wipprod:fs.file:vf.8665xnF7T7yu4-AMVkdEwQ?version=2" deviceset="CONN_SENSOR_DS" device="" value="MT6701"/>
<part name="C_SENS" library="BLLIB" deviceset="CAP_0402" device="" value="100n" package3d_urn="urn:adsk.eagle:package:16290895/6"/>
<part name="R_AZ_A" library="BLLIB" deviceset="RES_0603" device="" value="10k-DNP" package3d_urn="urn:adsk.eagle:package:16378565/7"/>
<part name="R_AZ_B" library="BLLIB" deviceset="RES_0603" device="" value="10k-DNP" package3d_urn="urn:adsk.eagle:package:16378565/7"/>
<part name="R_AZ_C" library="BLLIB" deviceset="RES_0603" device="" value="10k-DNP" package3d_urn="urn:adsk.eagle:package:16378565/7"/>
<part name="J_LED" library="JST-XH-BL" library_urn="urn:adsk.wipprod:fs.file:vf.UqJVv87FT1KjZqR4MQAHEw" deviceset="CONN_LED_DS" device="" value="CCT-STRIP" package3d_urn="urn:adsk.wipprod:fs.file:vf.cDk4rO_yQsSxQZYxAG73Fg?version=1"/>
<part name="R_GW" library="BLLIB" deviceset="RES_0603" device="" value="33R" package3d_urn="urn:adsk.eagle:package:16378565/7"/>
<part name="R_GC" library="BLLIB" deviceset="RES_0603" device="" value="33R" package3d_urn="urn:adsk.eagle:package:16378565/7"/>
<part name="R_PDW" library="BLLIB" deviceset="RES_0603" device="" value="100k" package3d_urn="urn:adsk.eagle:package:16378565/7"/>
<part name="R_PDC" library="BLLIB" deviceset="RES_0603" device="" value="100k" package3d_urn="urn:adsk.eagle:package:16378565/7"/>
<part name="D_CLW" library="BLLIB" deviceset="DIODE_SOD323" device="" value="clamp-DNP" package3d_urn="urn:adsk.eagle:package:10898396/3"/>
<part name="D_CLC" library="BLLIB" deviceset="DIODE_SOD323" device="" value="clamp-DNP" package3d_urn="urn:adsk.eagle:package:10898396/3"/>
<part name="J_KNOB" library="JST-XH-BL" library_urn="urn:adsk.wipprod:fs.file:vf.UqJVv87FT1KjZqR4MQAHEw" deviceset="CONN_KNOB_DS" device="" value="KNOB" package3d_urn="urn:adsk.wipprod:fs.file:vf.cDk4rO_yQsSxQZYxAG73Fg?version=1"/>
<part name="J_BTN" library="JST-XH-BL" library_urn="urn:adsk.wipprod:fs.file:vf.UqJVv87FT1KjZqR4MQAHEw" deviceset="CONN_BTN_DS" device="" value="BUTTON" package3d_urn="urn:adsk.wipprod:fs.file:vf.HiWWKYzERta-VOpw5EMMBA?version=1"/>
<part name="J_EXP" library="BLLIB" deviceset="CONN_EXP_DS" device="" value="GPIO-EXP" package3d_urn="urn:adsk.eagle:package:51802615/2"/>
<part name="R_STATUS" library="BLLIB" deviceset="RES_0603" device="" value="1k" package3d_urn="urn:adsk.eagle:package:16378565/7"/>
<part name="LED_STATUS" library="BLLIB" deviceset="LED_0603" device="" value="STATUS" package3d_urn="urn:adsk.eagle:package:24294786/3"/>
<part name="U_MCU" library="ESP32-S3-WROOM-1-N8R8" library_urn="urn:adsk.wipprod:fs.file:vf.hVdIrY1CQSyaVlBVWFbtZQ" deviceset="ESP32-S3-WROOM-1-N8R8" device="" package3d_urn="urn:adsk.wipprod:fs.file:vf.dXc3RDZRTwmGOH6vqNPrOw?version=2"/>
<part name="U_DRV" library="DRV8313PWPR" library_urn="urn:adsk.wipprod:fs.file:vf.Qd50y5P0SAeBl-SYBauaqQ" deviceset="DRV8313PWPR" device="" package3d_urn="urn:adsk.wipprod:fs.file:vf.8ar484oXSRiencqLNEvKVw?version=2"/>
<part name="J_USB" library="TYPE-C-31-M-12" library_urn="urn:adsk.wipprod:fs.file:vf.lrnFXZjHR86kpQ-OMBs1Ug" deviceset="TYPE-C-31-M-12" device="" package3d_urn="urn:adsk.wipprod:fs.file:vf.VrcAKNroTJKLNHeQPZCpWA?version=2"/>
<part name="D_USB" library="USBLC6-2SC6" library_urn="urn:adsk.wipprod:fs.file:vf.6slYtN6HRKK38zIgSJqcUQ" deviceset="USBLC6-2SC6" device="" package3d_urn="urn:adsk.eagle:package:18604126/4"/>
<part name="Q_WW" library="AO3400A" library_urn="urn:adsk.wipprod:fs.file:vf.206TU6vcR6CgwUWcVQ0mjQ" deviceset="AO3400A" device="" package3d_urn="urn:adsk.eagle:package:46660660/4"/>
<part name="Q_CW" library="AO3400A" library_urn="urn:adsk.wipprod:fs.file:vf.206TU6vcR6CgwUWcVQ0mjQ" deviceset="AO3400A" device="" package3d_urn="urn:adsk.eagle:package:46660660/4"/>
</parts>
<sheets>
<sheet>
<plain>
</plain>
<instances>
<instance part="C_MCU1" gate="G$1" x="0" y="0" smashed="yes"/>
<instance part="C_MCU2" gate="G$1" x="88.9" y="0" smashed="yes"/>
<instance part="C_MCU3" gate="G$1" x="177.8" y="0" smashed="yes"/>
<instance part="C_MCU4" gate="G$1" x="266.7" y="0" smashed="yes"/>
<instance part="C_MCU_BULK" gate="G$1" x="355.6" y="0" smashed="yes"/>
<instance part="D_MCU" gate="G$1" x="444.5" y="0" smashed="yes"/>
<instance part="R_EN" gate="G$1" x="0" y="-76.2" smashed="yes"/>
<instance part="C_EN" gate="G$1" x="88.9" y="-76.2" smashed="yes"/>
<instance part="SW_RST" gate="G$1" x="177.8" y="-76.2" smashed="yes"/>
<instance part="R_BOOT" gate="G$1" x="266.7" y="-76.2" smashed="yes"/>
<instance part="SW_BOOT" gate="G$1" x="355.6" y="-76.2" smashed="yes"/>
<instance part="C_BOOT" gate="G$1" x="444.5" y="-76.2" smashed="yes"/>
<instance part="R_CC1" gate="G$1" x="0" y="-152.4" smashed="yes"/>
<instance part="R_CC2" gate="G$1" x="88.9" y="-152.4" smashed="yes"/>
<instance part="R_DP" gate="G$1" x="177.8" y="-152.4" smashed="yes"/>
<instance part="R_DM" gate="G$1" x="266.7" y="-152.4" smashed="yes"/>
<instance part="C_VBUS" gate="G$1" x="355.6" y="-152.4" smashed="yes"/>
<instance part="D_OR_USB" gate="G$1" x="444.5" y="-152.4" smashed="yes"/>
<instance part="J_PROG" gate="G$1" x="0" y="-228.6" smashed="yes"/>
<instance part="J_PWR" gate="G$1" x="88.9" y="-228.6" smashed="yes"/>
<instance part="F1" gate="G$1" x="177.8" y="-228.6" smashed="yes"/>
<instance part="Q1" gate="G$1" x="266.7" y="-228.6" smashed="yes"/>
<instance part="R_Q1" gate="G$1" x="355.6" y="-228.6" smashed="yes"/>
<instance part="D_Q1" gate="G$1" x="444.5" y="-228.6" smashed="yes"/>
<instance part="TVS1" gate="G$1" x="0" y="-304.8" smashed="yes"/>
<instance part="C_IN_BULK" gate="G$1" x="88.9" y="-304.8" smashed="yes"/>
<instance part="U_BUCK" gate="G$1" x="177.8" y="-304.8" smashed="yes"/>
<instance part="C_IN1" gate="G$1" x="266.7" y="-304.8" smashed="yes"/>
<instance part="C_IN2" gate="G$1" x="355.6" y="-304.8" smashed="yes"/>
<instance part="R_ENB" gate="G$1" x="444.5" y="-304.8" smashed="yes"/>
<instance part="L1" gate="G$1" x="0" y="-381" smashed="yes"/>
<instance part="C_BOOT_B" gate="G$1" x="88.9" y="-381" smashed="yes"/>
<instance part="C_OUT1" gate="G$1" x="177.8" y="-381" smashed="yes"/>
<instance part="C_OUT2" gate="G$1" x="266.7" y="-381" smashed="yes"/>
<instance part="R_FBT" gate="G$1" x="355.6" y="-381" smashed="yes"/>
<instance part="R_FBB" gate="G$1" x="444.5" y="-381" smashed="yes"/>
<instance part="C_FF" gate="G$1" x="0" y="-457.2" smashed="yes"/>
<instance part="C_CP" gate="G$1" x="88.9" y="-457.2" smashed="yes"/>
<instance part="C_VCP" gate="G$1" x="177.8" y="-457.2" smashed="yes"/>
<instance part="C_V3P3" gate="G$1" x="266.7" y="-457.2" smashed="yes"/>
<instance part="C_VM1" gate="G$1" x="355.6" y="-457.2" smashed="yes"/>
<instance part="C_VM2" gate="G$1" x="444.5" y="-457.2" smashed="yes"/>
<instance part="C_VM_BULK" gate="G$1" x="0" y="-533.4" smashed="yes"/>
<instance part="R_NSLEEP" gate="G$1" x="88.9" y="-533.4" smashed="yes"/>
<instance part="R_NRESET" gate="G$1" x="177.8" y="-533.4" smashed="yes"/>
<instance part="R_NFAULT" gate="G$1" x="266.7" y="-533.4" smashed="yes"/>
<instance part="R_EN_DRV" gate="G$1" x="355.6" y="-533.4" smashed="yes"/>
<instance part="J_MOTOR" gate="G$1" x="444.5" y="-533.4" smashed="yes"/>
<instance part="J_SENSOR" gate="G$1" x="0" y="-609.6" smashed="yes"/>
<instance part="C_SENS" gate="G$1" x="88.9" y="-609.6" smashed="yes"/>
<instance part="R_AZ_A" gate="G$1" x="177.8" y="-609.6" smashed="yes"/>
<instance part="R_AZ_B" gate="G$1" x="266.7" y="-609.6" smashed="yes"/>
<instance part="R_AZ_C" gate="G$1" x="355.6" y="-609.6" smashed="yes"/>
<instance part="J_LED" gate="G$1" x="444.5" y="-609.6" smashed="yes"/>
<instance part="R_GW" gate="G$1" x="0" y="-685.8" smashed="yes"/>
<instance part="R_GC" gate="G$1" x="88.9" y="-685.8" smashed="yes"/>
<instance part="R_PDW" gate="G$1" x="177.8" y="-685.8" smashed="yes"/>
<instance part="R_PDC" gate="G$1" x="266.7" y="-685.8" smashed="yes"/>
<instance part="D_CLW" gate="G$1" x="355.6" y="-685.8" smashed="yes"/>
<instance part="D_CLC" gate="G$1" x="444.5" y="-685.8" smashed="yes"/>
<instance part="J_KNOB" gate="G$1" x="0" y="-762" smashed="yes"/>
<instance part="J_BTN" gate="G$1" x="88.9" y="-762" smashed="yes"/>
<instance part="J_EXP" gate="G$1" x="177.8" y="-762" smashed="yes"/>
<instance part="R_STATUS" gate="G$1" x="266.7" y="-762" smashed="yes"/>
<instance part="LED_STATUS" gate="G$1" x="355.6" y="-762" smashed="yes"/>
<instance part="U_MCU" gate="G$1" x="444.5" y="-762" smashed="yes"/>
<instance part="U_DRV" gate="G$1" x="0" y="-838.2" smashed="yes"/>
<instance part="J_USB" gate="G$1" x="88.9" y="-838.2" smashed="yes"/>
<instance part="D_USB" gate="G$1" x="177.8" y="-838.2" smashed="yes"/>
<instance part="Q_WW" gate="G$1" x="266.7" y="-838.2" smashed="yes"/>
<instance part="Q_CW" gate="G$1" x="355.6" y="-838.2" smashed="yes"/>
</instances>
<busses>
</busses>
<nets>
<net name="BOOT" class="0">
<segment>
<pinref part="R_BOOT" gate="G$1" pin="2"/>
<wire x1="279.4" y1="-76.2" x2="281.94" y2="-76.2" width="0.1524" layer="91"/>
<label x="281.94" y="-76.2" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="SW_BOOT" gate="G$1" pin="1"/>
<wire x1="342.9" y1="-76.2" x2="340.36" y2="-76.2" width="0.1524" layer="91"/>
<label x="340.36" y="-76.2" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="C_BOOT" gate="G$1" pin="1"/>
<wire x1="431.8" y1="-76.2" x2="429.26" y2="-76.2" width="0.1524" layer="91"/>
<label x="429.26" y="-76.2" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="J_PROG" gate="G$1" pin="IO0"/>
<wire x1="12.7" y1="-226.06" x2="15.24" y2="-226.06" width="0.1524" layer="91"/>
<label x="15.24" y="-226.06" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="U_MCU" gate="G$1" pin="IO0"/>
<wire x1="429.26" y1="-746.76" x2="426.72" y2="-746.76" width="0.1524" layer="91"/>
<label x="426.72" y="-746.76" size="1.27" layer="91"/>
</segment>
</net>
<net name="BST" class="0">
<segment>
<pinref part="U_BUCK" gate="G$1" pin="BST"/>
<wire x1="190.5" y1="-307.34" x2="193.04" y2="-307.34" width="0.1524" layer="91"/>
<label x="193.04" y="-307.34" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="C_BOOT_B" gate="G$1" pin="1"/>
<wire x1="76.2" y1="-381" x2="73.66" y2="-381" width="0.1524" layer="91"/>
<label x="73.66" y="-381" size="1.27" layer="91"/>
</segment>
</net>
<net name="BTN" class="0">
<segment>
<pinref part="J_BTN" gate="G$1" pin="SW"/>
<wire x1="76.2" y1="-762" x2="73.66" y2="-762" width="0.1524" layer="91"/>
<label x="73.66" y="-762" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="U_MCU" gate="G$1" pin="IO10"/>
<wire x1="429.26" y1="-772.16" x2="426.72" y2="-772.16" width="0.1524" layer="91"/>
<label x="426.72" y="-772.16" size="1.27" layer="91"/>
</segment>
</net>
<net name="BUCK_EN" class="0">
<segment>
<pinref part="U_BUCK" gate="G$1" pin="EN"/>
<wire x1="165.1" y1="-307.34" x2="162.56" y2="-307.34" width="0.1524" layer="91"/>
<label x="162.56" y="-307.34" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="R_ENB" gate="G$1" pin="2"/>
<wire x1="457.2" y1="-304.8" x2="459.74" y2="-304.8" width="0.1524" layer="91"/>
<label x="459.74" y="-304.8" size="1.27" layer="91"/>
</segment>
</net>
<net name="CC1" class="0">
<segment>
<pinref part="R_CC1" gate="G$1" pin="1"/>
<wire x1="-12.7" y1="-152.4" x2="-15.24" y2="-152.4" width="0.1524" layer="91"/>
<label x="-15.24" y="-152.4" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="J_USB" gate="G$1" pin="CC1"/>
<wire x1="71.12" y1="-833.12" x2="68.58" y2="-833.12" width="0.1524" layer="91"/>
<label x="68.58" y="-833.12" size="1.27" layer="91"/>
</segment>
</net>
<net name="CC2" class="0">
<segment>
<pinref part="R_CC2" gate="G$1" pin="1"/>
<wire x1="76.2" y1="-152.4" x2="73.66" y2="-152.4" width="0.1524" layer="91"/>
<label x="73.66" y="-152.4" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="J_USB" gate="G$1" pin="CC2"/>
<wire x1="106.68" y1="-840.74" x2="109.22" y2="-840.74" width="0.1524" layer="91"/>
<label x="109.22" y="-840.74" size="1.27" layer="91"/>
</segment>
</net>
<net name="CPH" class="0">
<segment>
<pinref part="C_CP" gate="G$1" pin="1"/>
<wire x1="76.2" y1="-457.2" x2="73.66" y2="-457.2" width="0.1524" layer="91"/>
<label x="73.66" y="-457.2" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="U_DRV" gate="G$1" pin="CPH"/>
<wire x1="17.78" y1="-822.96" x2="20.32" y2="-822.96" width="0.1524" layer="91"/>
<label x="20.32" y="-822.96" size="1.27" layer="91"/>
</segment>
</net>
<net name="CPL" class="0">
<segment>
<pinref part="C_CP" gate="G$1" pin="2"/>
<wire x1="101.6" y1="-457.2" x2="104.14" y2="-457.2" width="0.1524" layer="91"/>
<label x="104.14" y="-457.2" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="U_DRV" gate="G$1" pin="CPL"/>
<wire x1="17.78" y1="-820.42" x2="20.32" y2="-820.42" width="0.1524" layer="91"/>
<label x="20.32" y="-820.42" size="1.27" layer="91"/>
</segment>
</net>
<net name="CW_G" class="0">
<segment>
<pinref part="R_GC" gate="G$1" pin="1"/>
<wire x1="76.2" y1="-685.8" x2="73.66" y2="-685.8" width="0.1524" layer="91"/>
<label x="73.66" y="-685.8" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="R_PDC" gate="G$1" pin="1"/>
<wire x1="254" y1="-685.8" x2="251.46" y2="-685.8" width="0.1524" layer="91"/>
<label x="251.46" y="-685.8" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="Q_CW" gate="G$1" pin="G"/>
<wire x1="353.06" y1="-840.74" x2="350.52" y2="-840.74" width="0.1524" layer="91"/>
<label x="350.52" y="-840.74" size="1.27" layer="91"/>
</segment>
</net>
<net name="DRVEN" class="0">
<segment>
<pinref part="R_EN_DRV" gate="G$1" pin="2"/>
<wire x1="368.3" y1="-533.4" x2="370.84" y2="-533.4" width="0.1524" layer="91"/>
<label x="370.84" y="-533.4" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="U_MCU" gate="G$1" pin="IO15"/>
<wire x1="429.26" y1="-784.86" x2="426.72" y2="-784.86" width="0.1524" layer="91"/>
<label x="426.72" y="-784.86" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="U_DRV" gate="G$1" pin="EN1"/>
<wire x1="-17.78" y1="-825.5" x2="-20.32" y2="-825.5" width="0.1524" layer="91"/>
<label x="-20.32" y="-825.5" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="U_DRV" gate="G$1" pin="EN2"/>
<wire x1="-17.78" y1="-828.04" x2="-20.32" y2="-828.04" width="0.1524" layer="91"/>
<label x="-20.32" y="-828.04" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="U_DRV" gate="G$1" pin="EN3"/>
<wire x1="-17.78" y1="-830.58" x2="-20.32" y2="-830.58" width="0.1524" layer="91"/>
<label x="-20.32" y="-830.58" size="1.27" layer="91"/>
</segment>
</net>
<net name="DRV_V3P3" class="0">
<segment>
<pinref part="C_V3P3" gate="G$1" pin="1"/>
<wire x1="254" y1="-457.2" x2="251.46" y2="-457.2" width="0.1524" layer="91"/>
<label x="251.46" y="-457.2" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="U_DRV" gate="G$1" pin="V3P3"/>
<wire x1="17.78" y1="-812.8" x2="20.32" y2="-812.8" width="0.1524" layer="91"/>
<label x="20.32" y="-812.8" size="1.27" layer="91"/>
</segment>
</net>
<net name="ENCA" class="0">
<segment>
<pinref part="J_SENSOR" gate="G$1" pin="A"/>
<wire x1="-12.7" y1="-612.14" x2="-15.24" y2="-612.14" width="0.1524" layer="91"/>
<label x="-15.24" y="-612.14" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="R_AZ_A" gate="G$1" pin="2"/>
<wire x1="190.5" y1="-609.6" x2="193.04" y2="-609.6" width="0.1524" layer="91"/>
<label x="193.04" y="-609.6" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="U_MCU" gate="G$1" pin="IO17"/>
<wire x1="459.74" y1="-746.76" x2="462.28" y2="-746.76" width="0.1524" layer="91"/>
<label x="462.28" y="-746.76" size="1.27" layer="91"/>
</segment>
</net>
<net name="ENCB" class="0">
<segment>
<pinref part="J_SENSOR" gate="G$1" pin="B"/>
<wire x1="12.7" y1="-608.33" x2="15.24" y2="-608.33" width="0.1524" layer="91"/>
<label x="15.24" y="-608.33" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="R_AZ_B" gate="G$1" pin="2"/>
<wire x1="279.4" y1="-609.6" x2="281.94" y2="-609.6" width="0.1524" layer="91"/>
<label x="281.94" y="-609.6" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="U_MCU" gate="G$1" pin="IO18"/>
<wire x1="459.74" y1="-749.3" x2="462.28" y2="-749.3" width="0.1524" layer="91"/>
<label x="462.28" y="-749.3" size="1.27" layer="91"/>
</segment>
</net>
<net name="ENCZ" class="0">
<segment>
<pinref part="J_SENSOR" gate="G$1" pin="Z"/>
<wire x1="12.7" y1="-610.87" x2="15.24" y2="-610.87" width="0.1524" layer="91"/>
<label x="15.24" y="-610.87" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="R_AZ_C" gate="G$1" pin="2"/>
<wire x1="368.3" y1="-609.6" x2="370.84" y2="-609.6" width="0.1524" layer="91"/>
<label x="370.84" y="-609.6" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="U_MCU" gate="G$1" pin="IO14"/>
<wire x1="429.26" y1="-782.32" x2="426.72" y2="-782.32" width="0.1524" layer="91"/>
<label x="426.72" y="-782.32" size="1.27" layer="91"/>
</segment>
</net>
<net name="EN_RST" class="0">
<segment>
<pinref part="R_EN" gate="G$1" pin="2"/>
<wire x1="12.7" y1="-76.2" x2="15.24" y2="-76.2" width="0.1524" layer="91"/>
<label x="15.24" y="-76.2" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="C_EN" gate="G$1" pin="1"/>
<wire x1="76.2" y1="-76.2" x2="73.66" y2="-76.2" width="0.1524" layer="91"/>
<label x="73.66" y="-76.2" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="SW_RST" gate="G$1" pin="1"/>
<wire x1="165.1" y1="-76.2" x2="162.56" y2="-76.2" width="0.1524" layer="91"/>
<label x="162.56" y="-76.2" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="J_PROG" gate="G$1" pin="EN"/>
<wire x1="-12.7" y1="-231.14" x2="-15.24" y2="-231.14" width="0.1524" layer="91"/>
<label x="-15.24" y="-231.14" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="U_MCU" gate="G$1" pin="EN"/>
<wire x1="429.26" y1="-734.06" x2="426.72" y2="-734.06" width="0.1524" layer="91"/>
<label x="426.72" y="-734.06" size="1.27" layer="91"/>
</segment>
</net>
<net name="EXP_IO1" class="0">
<segment>
<pinref part="J_EXP" gate="G$1" pin="IO1"/>
<wire x1="165.1" y1="-762" x2="162.56" y2="-762" width="0.1524" layer="91"/>
<label x="162.56" y="-762" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="U_MCU" gate="G$1" pin="IO1"/>
<wire x1="429.26" y1="-749.3" x2="426.72" y2="-749.3" width="0.1524" layer="91"/>
<label x="426.72" y="-749.3" size="1.27" layer="91"/>
</segment>
</net>
<net name="EXP_IO13" class="0">
<segment>
<pinref part="J_EXP" gate="G$1" pin="IO13"/>
<wire x1="165.1" y1="-767.08" x2="162.56" y2="-767.08" width="0.1524" layer="91"/>
<label x="162.56" y="-767.08" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="U_MCU" gate="G$1" pin="IO13"/>
<wire x1="429.26" y1="-779.78" x2="426.72" y2="-779.78" width="0.1524" layer="91"/>
<label x="426.72" y="-779.78" size="1.27" layer="91"/>
</segment>
</net>
<net name="EXP_IO2" class="0">
<segment>
<pinref part="J_EXP" gate="G$1" pin="IO2"/>
<wire x1="165.1" y1="-764.54" x2="162.56" y2="-764.54" width="0.1524" layer="91"/>
<label x="162.56" y="-764.54" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="U_MCU" gate="G$1" pin="IO2"/>
<wire x1="429.26" y1="-751.84" x2="426.72" y2="-751.84" width="0.1524" layer="91"/>
<label x="426.72" y="-751.84" size="1.27" layer="91"/>
</segment>
</net>
<net name="EXP_IO38" class="0">
<segment>
<pinref part="J_EXP" gate="G$1" pin="IO38"/>
<wire x1="190.5" y1="-756.92" x2="193.04" y2="-756.92" width="0.1524" layer="91"/>
<label x="193.04" y="-756.92" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="U_MCU" gate="G$1" pin="IO38"/>
<wire x1="459.74" y1="-767.08" x2="462.28" y2="-767.08" width="0.1524" layer="91"/>
<label x="462.28" y="-767.08" size="1.27" layer="91"/>
</segment>
</net>
<net name="EXP_IO39" class="0">
<segment>
<pinref part="J_EXP" gate="G$1" pin="IO39"/>
<wire x1="190.5" y1="-759.46" x2="193.04" y2="-759.46" width="0.1524" layer="91"/>
<label x="193.04" y="-759.46" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="U_MCU" gate="G$1" pin="IO39"/>
<wire x1="459.74" y1="-769.62" x2="462.28" y2="-769.62" width="0.1524" layer="91"/>
<label x="462.28" y="-769.62" size="1.27" layer="91"/>
</segment>
</net>
<net name="EXP_IO40" class="0">
<segment>
<pinref part="J_EXP" gate="G$1" pin="IO40"/>
<wire x1="190.5" y1="-762" x2="193.04" y2="-762" width="0.1524" layer="91"/>
<label x="193.04" y="-762" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="U_MCU" gate="G$1" pin="IO40"/>
<wire x1="459.74" y1="-772.16" x2="462.28" y2="-772.16" width="0.1524" layer="91"/>
<label x="462.28" y="-772.16" size="1.27" layer="91"/>
</segment>
</net>
<net name="EXP_IO41" class="0">
<segment>
<pinref part="J_EXP" gate="G$1" pin="IO41"/>
<wire x1="190.5" y1="-764.54" x2="193.04" y2="-764.54" width="0.1524" layer="91"/>
<label x="193.04" y="-764.54" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="U_MCU" gate="G$1" pin="IO41"/>
<wire x1="459.74" y1="-774.7" x2="462.28" y2="-774.7" width="0.1524" layer="91"/>
<label x="462.28" y="-774.7" size="1.27" layer="91"/>
</segment>
</net>
<net name="EXP_IO42" class="0">
<segment>
<pinref part="J_EXP" gate="G$1" pin="IO42"/>
<wire x1="190.5" y1="-767.08" x2="193.04" y2="-767.08" width="0.1524" layer="91"/>
<label x="193.04" y="-767.08" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="U_MCU" gate="G$1" pin="IO42"/>
<wire x1="459.74" y1="-777.24" x2="462.28" y2="-777.24" width="0.1524" layer="91"/>
<label x="462.28" y="-777.24" size="1.27" layer="91"/>
</segment>
</net>
<net name="FB" class="0">
<segment>
<pinref part="U_BUCK" gate="G$1" pin="FB"/>
<wire x1="190.5" y1="-302.26" x2="193.04" y2="-302.26" width="0.1524" layer="91"/>
<label x="193.04" y="-302.26" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="R_FBT" gate="G$1" pin="2"/>
<wire x1="368.3" y1="-381" x2="370.84" y2="-381" width="0.1524" layer="91"/>
<label x="370.84" y="-381" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="R_FBB" gate="G$1" pin="1"/>
<wire x1="431.8" y1="-381" x2="429.26" y2="-381" width="0.1524" layer="91"/>
<label x="429.26" y="-381" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="C_FF" gate="G$1" pin="2"/>
<wire x1="12.7" y1="-457.2" x2="15.24" y2="-457.2" width="0.1524" layer="91"/>
<label x="15.24" y="-457.2" size="1.27" layer="91"/>
</segment>
</net>
<net name="GND" class="0">
<segment>
<pinref part="C_MCU1" gate="G$1" pin="2"/>
<wire x1="12.7" y1="0" x2="15.24" y2="0" width="0.1524" layer="91"/>
<label x="15.24" y="0" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="C_MCU2" gate="G$1" pin="2"/>
<wire x1="101.6" y1="0" x2="104.14" y2="0" width="0.1524" layer="91"/>
<label x="104.14" y="0" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="C_MCU3" gate="G$1" pin="2"/>
<wire x1="190.5" y1="0" x2="193.04" y2="0" width="0.1524" layer="91"/>
<label x="193.04" y="0" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="C_MCU4" gate="G$1" pin="2"/>
<wire x1="279.4" y1="0" x2="281.94" y2="0" width="0.1524" layer="91"/>
<label x="281.94" y="0" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="C_MCU_BULK" gate="G$1" pin="2"/>
<wire x1="368.3" y1="0" x2="370.84" y2="0" width="0.1524" layer="91"/>
<label x="370.84" y="0" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="D_MCU" gate="G$1" pin="A"/>
<wire x1="431.8" y1="0" x2="429.26" y2="0" width="0.1524" layer="91"/>
<label x="429.26" y="0" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="C_EN" gate="G$1" pin="2"/>
<wire x1="101.6" y1="-76.2" x2="104.14" y2="-76.2" width="0.1524" layer="91"/>
<label x="104.14" y="-76.2" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="SW_RST" gate="G$1" pin="2"/>
<wire x1="190.5" y1="-76.2" x2="193.04" y2="-76.2" width="0.1524" layer="91"/>
<label x="193.04" y="-76.2" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="SW_BOOT" gate="G$1" pin="2"/>
<wire x1="368.3" y1="-76.2" x2="370.84" y2="-76.2" width="0.1524" layer="91"/>
<label x="370.84" y="-76.2" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="C_BOOT" gate="G$1" pin="2"/>
<wire x1="457.2" y1="-76.2" x2="459.74" y2="-76.2" width="0.1524" layer="91"/>
<label x="459.74" y="-76.2" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="R_CC1" gate="G$1" pin="2"/>
<wire x1="12.7" y1="-152.4" x2="15.24" y2="-152.4" width="0.1524" layer="91"/>
<label x="15.24" y="-152.4" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="R_CC2" gate="G$1" pin="2"/>
<wire x1="101.6" y1="-152.4" x2="104.14" y2="-152.4" width="0.1524" layer="91"/>
<label x="104.14" y="-152.4" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="C_VBUS" gate="G$1" pin="2"/>
<wire x1="368.3" y1="-152.4" x2="370.84" y2="-152.4" width="0.1524" layer="91"/>
<label x="370.84" y="-152.4" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="J_PROG" gate="G$1" pin="GND"/>
<wire x1="-12.7" y1="-228.6" x2="-15.24" y2="-228.6" width="0.1524" layer="91"/>
<label x="-15.24" y="-228.6" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="J_PWR" gate="G$1" pin="VN"/>
<wire x1="101.6" y1="-228.6" x2="104.14" y2="-228.6" width="0.1524" layer="91"/>
<label x="104.14" y="-228.6" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="R_Q1" gate="G$1" pin="2"/>
<wire x1="368.3" y1="-228.6" x2="370.84" y2="-228.6" width="0.1524" layer="91"/>
<label x="370.84" y="-228.6" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="TVS1" gate="G$1" pin="A"/>
<wire x1="-12.7" y1="-304.8" x2="-15.24" y2="-304.8" width="0.1524" layer="91"/>
<label x="-15.24" y="-304.8" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="C_IN_BULK" gate="G$1" pin="2"/>
<wire x1="101.6" y1="-304.8" x2="104.14" y2="-304.8" width="0.1524" layer="91"/>
<label x="104.14" y="-304.8" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="U_BUCK" gate="G$1" pin="GND"/>
<wire x1="165.1" y1="-304.8" x2="162.56" y2="-304.8" width="0.1524" layer="91"/>
<label x="162.56" y="-304.8" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="C_IN1" gate="G$1" pin="2"/>
<wire x1="279.4" y1="-304.8" x2="281.94" y2="-304.8" width="0.1524" layer="91"/>
<label x="281.94" y="-304.8" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="C_IN2" gate="G$1" pin="2"/>
<wire x1="368.3" y1="-304.8" x2="370.84" y2="-304.8" width="0.1524" layer="91"/>
<label x="370.84" y="-304.8" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="C_OUT1" gate="G$1" pin="2"/>
<wire x1="190.5" y1="-381" x2="193.04" y2="-381" width="0.1524" layer="91"/>
<label x="193.04" y="-381" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="C_OUT2" gate="G$1" pin="2"/>
<wire x1="279.4" y1="-381" x2="281.94" y2="-381" width="0.1524" layer="91"/>
<label x="281.94" y="-381" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="R_FBB" gate="G$1" pin="2"/>
<wire x1="457.2" y1="-381" x2="459.74" y2="-381" width="0.1524" layer="91"/>
<label x="459.74" y="-381" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="C_V3P3" gate="G$1" pin="2"/>
<wire x1="279.4" y1="-457.2" x2="281.94" y2="-457.2" width="0.1524" layer="91"/>
<label x="281.94" y="-457.2" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="C_VM1" gate="G$1" pin="2"/>
<wire x1="368.3" y1="-457.2" x2="370.84" y2="-457.2" width="0.1524" layer="91"/>
<label x="370.84" y="-457.2" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="C_VM2" gate="G$1" pin="2"/>
<wire x1="457.2" y1="-457.2" x2="459.74" y2="-457.2" width="0.1524" layer="91"/>
<label x="459.74" y="-457.2" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="C_VM_BULK" gate="G$1" pin="2"/>
<wire x1="12.7" y1="-533.4" x2="15.24" y2="-533.4" width="0.1524" layer="91"/>
<label x="15.24" y="-533.4" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="J_SENSOR" gate="G$1" pin="GND"/>
<wire x1="-12.7" y1="-609.6" x2="-15.24" y2="-609.6" width="0.1524" layer="91"/>
<label x="-15.24" y="-609.6" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="C_SENS" gate="G$1" pin="2"/>
<wire x1="101.6" y1="-609.6" x2="104.14" y2="-609.6" width="0.1524" layer="91"/>
<label x="104.14" y="-609.6" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="R_PDW" gate="G$1" pin="2"/>
<wire x1="190.5" y1="-685.8" x2="193.04" y2="-685.8" width="0.1524" layer="91"/>
<label x="193.04" y="-685.8" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="R_PDC" gate="G$1" pin="2"/>
<wire x1="279.4" y1="-685.8" x2="281.94" y2="-685.8" width="0.1524" layer="91"/>
<label x="281.94" y="-685.8" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="J_KNOB" gate="G$1" pin="GND"/>
<wire x1="12.7" y1="-762" x2="15.24" y2="-762" width="0.1524" layer="91"/>
<label x="15.24" y="-762" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="J_BTN" gate="G$1" pin="GND"/>
<wire x1="101.6" y1="-762" x2="104.14" y2="-762" width="0.1524" layer="91"/>
<label x="104.14" y="-762" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="J_EXP" gate="G$1" pin="GND"/>
<wire x1="165.1" y1="-759.46" x2="162.56" y2="-759.46" width="0.1524" layer="91"/>
<label x="162.56" y="-759.46" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="LED_STATUS" gate="G$1" pin="C"/>
<wire x1="368.3" y1="-762" x2="370.84" y2="-762" width="0.1524" layer="91"/>
<label x="370.84" y="-762" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="U_MCU" gate="G$1" pin="GND"/>
<wire x1="459.74" y1="-792.48" x2="462.28" y2="-792.48" width="0.1524" layer="91"/>
<label x="462.28" y="-792.48" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="U_DRV" gate="G$1" pin="GND"/>
<wire x1="17.78" y1="-866.14" x2="20.32" y2="-866.14" width="0.1524" layer="91"/>
<label x="20.32" y="-866.14" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="U_DRV" gate="G$1" pin="PGND1"/>
<wire x1="17.78" y1="-858.52" x2="20.32" y2="-858.52" width="0.1524" layer="91"/>
<label x="20.32" y="-858.52" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="U_DRV" gate="G$1" pin="PGND2"/>
<wire x1="17.78" y1="-861.06" x2="20.32" y2="-861.06" width="0.1524" layer="91"/>
<label x="20.32" y="-861.06" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="U_DRV" gate="G$1" pin="PGND3"/>
<wire x1="17.78" y1="-863.6" x2="20.32" y2="-863.6" width="0.1524" layer="91"/>
<label x="20.32" y="-863.6" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="U_DRV" gate="G$1" pin="COMPP"/>
<wire x1="-17.78" y1="-848.36" x2="-20.32" y2="-848.36" width="0.1524" layer="91"/>
<label x="-20.32" y="-848.36" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="U_DRV" gate="G$1" pin="COMPN"/>
<wire x1="-17.78" y1="-845.82" x2="-20.32" y2="-845.82" width="0.1524" layer="91"/>
<label x="-20.32" y="-845.82" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="U_DRV" gate="G$1" pin="!COMPO"/>
<wire x1="17.78" y1="-845.82" x2="20.32" y2="-845.82" width="0.1524" layer="91"/>
<label x="20.32" y="-845.82" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="U_DRV" gate="G$1" pin="EXP"/>
<wire x1="17.78" y1="-853.44" x2="20.32" y2="-853.44" width="0.1524" layer="91"/>
<label x="20.32" y="-853.44" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="J_USB" gate="G$1" pin="GND"/>
<wire x1="106.68" y1="-848.36" x2="109.22" y2="-848.36" width="0.1524" layer="91"/>
<label x="109.22" y="-848.36" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="J_USB" gate="G$1" pin="SHIELD"/>
<wire x1="106.68" y1="-845.82" x2="109.22" y2="-845.82" width="0.1524" layer="91"/>
<label x="109.22" y="-845.82" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="D_USB" gate="G$1" pin="GND"/>
<wire x1="157.48" y1="-838.2" x2="154.94" y2="-838.2" width="0.1524" layer="91"/>
<label x="154.94" y="-838.2" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="Q_WW" gate="G$1" pin="S"/>
<wire x1="269.24" y1="-843.28" x2="271.78" y2="-843.28" width="0.1524" layer="91"/>
<label x="271.78" y="-843.28" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="Q_CW" gate="G$1" pin="S"/>
<wire x1="358.14" y1="-843.28" x2="360.68" y2="-843.28" width="0.1524" layer="91"/>
<label x="360.68" y="-843.28" size="1.27" layer="91"/>
</segment>
</net>
<net name="GW_G" class="0">
<segment>
<pinref part="R_GW" gate="G$1" pin="1"/>
<wire x1="-12.7" y1="-685.8" x2="-15.24" y2="-685.8" width="0.1524" layer="91"/>
<label x="-15.24" y="-685.8" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="R_PDW" gate="G$1" pin="1"/>
<wire x1="165.1" y1="-685.8" x2="162.56" y2="-685.8" width="0.1524" layer="91"/>
<label x="162.56" y="-685.8" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="Q_WW" gate="G$1" pin="G"/>
<wire x1="264.16" y1="-840.74" x2="261.62" y2="-840.74" width="0.1524" layer="91"/>
<label x="261.62" y="-840.74" size="1.27" layer="91"/>
</segment>
</net>
<net name="LEDCW" class="0">
<segment>
<pinref part="R_GC" gate="G$1" pin="2"/>
<wire x1="101.6" y1="-685.8" x2="104.14" y2="-685.8" width="0.1524" layer="91"/>
<label x="104.14" y="-685.8" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="U_MCU" gate="G$1" pin="IO12"/>
<wire x1="429.26" y1="-777.24" x2="426.72" y2="-777.24" width="0.1524" layer="91"/>
<label x="426.72" y="-777.24" size="1.27" layer="91"/>
</segment>
</net>
<net name="LEDC_D" class="0">
<segment>
<pinref part="J_LED" gate="G$1" pin="C"/>
<wire x1="457.2" y1="-609.6" x2="459.74" y2="-609.6" width="0.1524" layer="91"/>
<label x="459.74" y="-609.6" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="D_CLC" gate="G$1" pin="A"/>
<wire x1="431.8" y1="-685.8" x2="429.26" y2="-685.8" width="0.1524" layer="91"/>
<label x="429.26" y="-685.8" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="Q_CW" gate="G$1" pin="D"/>
<wire x1="358.14" y1="-833.12" x2="360.68" y2="-833.12" width="0.1524" layer="91"/>
<label x="360.68" y="-833.12" size="1.27" layer="91"/>
</segment>
</net>
<net name="LEDWW" class="0">
<segment>
<pinref part="R_GW" gate="G$1" pin="2"/>
<wire x1="12.7" y1="-685.8" x2="15.24" y2="-685.8" width="0.1524" layer="91"/>
<label x="15.24" y="-685.8" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="U_MCU" gate="G$1" pin="IO11"/>
<wire x1="429.26" y1="-774.7" x2="426.72" y2="-774.7" width="0.1524" layer="91"/>
<label x="426.72" y="-774.7" size="1.27" layer="91"/>
</segment>
</net>
<net name="LEDW_D" class="0">
<segment>
<pinref part="J_LED" gate="G$1" pin="W"/>
<wire x1="431.8" y1="-610.87" x2="429.26" y2="-610.87" width="0.1524" layer="91"/>
<label x="429.26" y="-610.87" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="D_CLW" gate="G$1" pin="A"/>
<wire x1="342.9" y1="-685.8" x2="340.36" y2="-685.8" width="0.1524" layer="91"/>
<label x="340.36" y="-685.8" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="Q_WW" gate="G$1" pin="D"/>
<wire x1="269.24" y1="-833.12" x2="271.78" y2="-833.12" width="0.1524" layer="91"/>
<label x="271.78" y="-833.12" size="1.27" layer="91"/>
</segment>
</net>
<net name="MOT_U" class="0">
<segment>
<pinref part="J_MOTOR" gate="G$1" pin="U"/>
<wire x1="431.8" y1="-532.13" x2="429.26" y2="-532.13" width="0.1524" layer="91"/>
<label x="429.26" y="-532.13" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="U_DRV" gate="G$1" pin="OUT1"/>
<wire x1="17.78" y1="-828.04" x2="20.32" y2="-828.04" width="0.1524" layer="91"/>
<label x="20.32" y="-828.04" size="1.27" layer="91"/>
</segment>
</net>
<net name="MOT_V" class="0">
<segment>
<pinref part="J_MOTOR" gate="G$1" pin="V"/>
<wire x1="431.8" y1="-534.67" x2="429.26" y2="-534.67" width="0.1524" layer="91"/>
<label x="429.26" y="-534.67" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="U_DRV" gate="G$1" pin="OUT2"/>
<wire x1="17.78" y1="-830.58" x2="20.32" y2="-830.58" width="0.1524" layer="91"/>
<label x="20.32" y="-830.58" size="1.27" layer="91"/>
</segment>
</net>
<net name="MOT_W" class="0">
<segment>
<pinref part="J_MOTOR" gate="G$1" pin="W"/>
<wire x1="457.2" y1="-533.4" x2="459.74" y2="-533.4" width="0.1524" layer="91"/>
<label x="459.74" y="-533.4" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="U_DRV" gate="G$1" pin="OUT3"/>
<wire x1="17.78" y1="-833.12" x2="20.32" y2="-833.12" width="0.1524" layer="91"/>
<label x="20.32" y="-833.12" size="1.27" layer="91"/>
</segment>
</net>
<net name="NFAULT" class="0">
<segment>
<pinref part="R_NFAULT" gate="G$1" pin="2"/>
<wire x1="279.4" y1="-533.4" x2="281.94" y2="-533.4" width="0.1524" layer="91"/>
<label x="281.94" y="-533.4" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="U_MCU" gate="G$1" pin="IO16"/>
<wire x1="429.26" y1="-787.4" x2="426.72" y2="-787.4" width="0.1524" layer="91"/>
<label x="426.72" y="-787.4" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="U_DRV" gate="G$1" pin="!FAULT"/>
<wire x1="17.78" y1="-838.2" x2="20.32" y2="-838.2" width="0.1524" layer="91"/>
<label x="20.32" y="-838.2" size="1.27" layer="91"/>
</segment>
</net>
<net name="NRESET" class="0">
<segment>
<pinref part="R_NRESET" gate="G$1" pin="2"/>
<wire x1="190.5" y1="-533.4" x2="193.04" y2="-533.4" width="0.1524" layer="91"/>
<label x="193.04" y="-533.4" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="U_DRV" gate="G$1" pin="!RESET"/>
<wire x1="-17.78" y1="-817.88" x2="-20.32" y2="-817.88" width="0.1524" layer="91"/>
<label x="-20.32" y="-817.88" size="1.27" layer="91"/>
</segment>
</net>
<net name="NSLEEP" class="0">
<segment>
<pinref part="R_NSLEEP" gate="G$1" pin="2"/>
<wire x1="101.6" y1="-533.4" x2="104.14" y2="-533.4" width="0.1524" layer="91"/>
<label x="104.14" y="-533.4" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="U_MCU" gate="G$1" pin="IO7"/>
<wire x1="429.26" y1="-764.54" x2="426.72" y2="-764.54" width="0.1524" layer="91"/>
<label x="426.72" y="-764.54" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="U_DRV" gate="G$1" pin="!SLEEP"/>
<wire x1="-17.78" y1="-820.42" x2="-20.32" y2="-820.42" width="0.1524" layer="91"/>
<label x="-20.32" y="-820.42" size="1.27" layer="91"/>
</segment>
</net>
<net name="P24" class="1">
<segment>
<pinref part="D_OR_USB" gate="G$1" pin="C"/>
<wire x1="457.2" y1="-152.4" x2="459.74" y2="-152.4" width="0.1524" layer="91"/>
<label x="459.74" y="-152.4" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="Q1" gate="G$1" pin="S"/>
<wire x1="279.4" y1="-228.6" x2="281.94" y2="-228.6" width="0.1524" layer="91"/>
<label x="281.94" y="-228.6" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="D_Q1" gate="G$1" pin="C"/>
<wire x1="457.2" y1="-228.6" x2="459.74" y2="-228.6" width="0.1524" layer="91"/>
<label x="459.74" y="-228.6" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="TVS1" gate="G$1" pin="C"/>
<wire x1="12.7" y1="-304.8" x2="15.24" y2="-304.8" width="0.1524" layer="91"/>
<label x="15.24" y="-304.8" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="C_IN_BULK" gate="G$1" pin="1"/>
<wire x1="76.2" y1="-304.8" x2="73.66" y2="-304.8" width="0.1524" layer="91"/>
<label x="73.66" y="-304.8" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="U_BUCK" gate="G$1" pin="VIN"/>
<wire x1="165.1" y1="-302.26" x2="162.56" y2="-302.26" width="0.1524" layer="91"/>
<label x="162.56" y="-302.26" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="C_IN1" gate="G$1" pin="1"/>
<wire x1="254" y1="-304.8" x2="251.46" y2="-304.8" width="0.1524" layer="91"/>
<label x="251.46" y="-304.8" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="C_IN2" gate="G$1" pin="1"/>
<wire x1="342.9" y1="-304.8" x2="340.36" y2="-304.8" width="0.1524" layer="91"/>
<label x="340.36" y="-304.8" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="R_ENB" gate="G$1" pin="1"/>
<wire x1="431.8" y1="-304.8" x2="429.26" y2="-304.8" width="0.1524" layer="91"/>
<label x="429.26" y="-304.8" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="C_VCP" gate="G$1" pin="2"/>
<wire x1="190.5" y1="-457.2" x2="193.04" y2="-457.2" width="0.1524" layer="91"/>
<label x="193.04" y="-457.2" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="C_VM1" gate="G$1" pin="1"/>
<wire x1="342.9" y1="-457.2" x2="340.36" y2="-457.2" width="0.1524" layer="91"/>
<label x="340.36" y="-457.2" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="C_VM2" gate="G$1" pin="1"/>
<wire x1="431.8" y1="-457.2" x2="429.26" y2="-457.2" width="0.1524" layer="91"/>
<label x="429.26" y="-457.2" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="C_VM_BULK" gate="G$1" pin="1"/>
<wire x1="-12.7" y1="-533.4" x2="-15.24" y2="-533.4" width="0.1524" layer="91"/>
<label x="-15.24" y="-533.4" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="J_LED" gate="G$1" pin="VP"/>
<wire x1="431.8" y1="-608.33" x2="429.26" y2="-608.33" width="0.1524" layer="91"/>
<label x="429.26" y="-608.33" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="D_CLW" gate="G$1" pin="C"/>
<wire x1="368.3" y1="-685.8" x2="370.84" y2="-685.8" width="0.1524" layer="91"/>
<label x="370.84" y="-685.8" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="D_CLC" gate="G$1" pin="C"/>
<wire x1="457.2" y1="-685.8" x2="459.74" y2="-685.8" width="0.1524" layer="91"/>
<label x="459.74" y="-685.8" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="U_DRV" gate="G$1" pin="VM"/>
<wire x1="17.78" y1="-815.34" x2="20.32" y2="-815.34" width="0.1524" layer="91"/>
<label x="20.32" y="-815.34" size="1.27" layer="91"/>
</segment>
</net>
<net name="P24_F" class="1">
<segment>
<pinref part="F1" gate="G$1" pin="2"/>
<wire x1="190.5" y1="-228.6" x2="193.04" y2="-228.6" width="0.1524" layer="91"/>
<label x="193.04" y="-228.6" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="Q1" gate="G$1" pin="D"/>
<wire x1="254" y1="-229.87" x2="251.46" y2="-229.87" width="0.1524" layer="91"/>
<label x="251.46" y="-229.87" size="1.27" layer="91"/>
</segment>
</net>
<net name="P24_RAW" class="1">
<segment>
<pinref part="J_PWR" gate="G$1" pin="VP"/>
<wire x1="76.2" y1="-228.6" x2="73.66" y2="-228.6" width="0.1524" layer="91"/>
<label x="73.66" y="-228.6" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="F1" gate="G$1" pin="1"/>
<wire x1="165.1" y1="-228.6" x2="162.56" y2="-228.6" width="0.1524" layer="91"/>
<label x="162.56" y="-228.6" size="1.27" layer="91"/>
</segment>
</net>
<net name="P3V3" class="2">
<segment>
<pinref part="C_MCU1" gate="G$1" pin="1"/>
<wire x1="-12.7" y1="0" x2="-15.24" y2="0" width="0.1524" layer="91"/>
<label x="-15.24" y="0" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="C_MCU2" gate="G$1" pin="1"/>
<wire x1="76.2" y1="0" x2="73.66" y2="0" width="0.1524" layer="91"/>
<label x="73.66" y="0" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="C_MCU3" gate="G$1" pin="1"/>
<wire x1="165.1" y1="0" x2="162.56" y2="0" width="0.1524" layer="91"/>
<label x="162.56" y="0" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="C_MCU4" gate="G$1" pin="1"/>
<wire x1="254" y1="0" x2="251.46" y2="0" width="0.1524" layer="91"/>
<label x="251.46" y="0" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="C_MCU_BULK" gate="G$1" pin="1"/>
<wire x1="342.9" y1="0" x2="340.36" y2="0" width="0.1524" layer="91"/>
<label x="340.36" y="0" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="D_MCU" gate="G$1" pin="C"/>
<wire x1="457.2" y1="0" x2="459.74" y2="0" width="0.1524" layer="91"/>
<label x="459.74" y="0" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="R_EN" gate="G$1" pin="1"/>
<wire x1="-12.7" y1="-76.2" x2="-15.24" y2="-76.2" width="0.1524" layer="91"/>
<label x="-15.24" y="-76.2" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="R_BOOT" gate="G$1" pin="1"/>
<wire x1="254" y1="-76.2" x2="251.46" y2="-76.2" width="0.1524" layer="91"/>
<label x="251.46" y="-76.2" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="J_PROG" gate="G$1" pin="3V3"/>
<wire x1="-12.7" y1="-226.06" x2="-15.24" y2="-226.06" width="0.1524" layer="91"/>
<label x="-15.24" y="-226.06" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="L1" gate="G$1" pin="2"/>
<wire x1="12.7" y1="-381" x2="15.24" y2="-381" width="0.1524" layer="91"/>
<label x="15.24" y="-381" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="C_OUT1" gate="G$1" pin="1"/>
<wire x1="165.1" y1="-381" x2="162.56" y2="-381" width="0.1524" layer="91"/>
<label x="162.56" y="-381" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="C_OUT2" gate="G$1" pin="1"/>
<wire x1="254" y1="-381" x2="251.46" y2="-381" width="0.1524" layer="91"/>
<label x="251.46" y="-381" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="R_FBT" gate="G$1" pin="1"/>
<wire x1="342.9" y1="-381" x2="340.36" y2="-381" width="0.1524" layer="91"/>
<label x="340.36" y="-381" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="C_FF" gate="G$1" pin="1"/>
<wire x1="-12.7" y1="-457.2" x2="-15.24" y2="-457.2" width="0.1524" layer="91"/>
<label x="-15.24" y="-457.2" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="R_NSLEEP" gate="G$1" pin="1"/>
<wire x1="76.2" y1="-533.4" x2="73.66" y2="-533.4" width="0.1524" layer="91"/>
<label x="73.66" y="-533.4" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="R_NRESET" gate="G$1" pin="1"/>
<wire x1="165.1" y1="-533.4" x2="162.56" y2="-533.4" width="0.1524" layer="91"/>
<label x="162.56" y="-533.4" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="R_NFAULT" gate="G$1" pin="1"/>
<wire x1="254" y1="-533.4" x2="251.46" y2="-533.4" width="0.1524" layer="91"/>
<label x="251.46" y="-533.4" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="R_EN_DRV" gate="G$1" pin="1"/>
<wire x1="342.9" y1="-533.4" x2="340.36" y2="-533.4" width="0.1524" layer="91"/>
<label x="340.36" y="-533.4" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="J_SENSOR" gate="G$1" pin="VCC"/>
<wire x1="-12.7" y1="-607.06" x2="-15.24" y2="-607.06" width="0.1524" layer="91"/>
<label x="-15.24" y="-607.06" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="C_SENS" gate="G$1" pin="1"/>
<wire x1="76.2" y1="-609.6" x2="73.66" y2="-609.6" width="0.1524" layer="91"/>
<label x="73.66" y="-609.6" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="R_AZ_A" gate="G$1" pin="1"/>
<wire x1="165.1" y1="-609.6" x2="162.56" y2="-609.6" width="0.1524" layer="91"/>
<label x="162.56" y="-609.6" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="R_AZ_B" gate="G$1" pin="1"/>
<wire x1="254" y1="-609.6" x2="251.46" y2="-609.6" width="0.1524" layer="91"/>
<label x="251.46" y="-609.6" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="R_AZ_C" gate="G$1" pin="1"/>
<wire x1="342.9" y1="-609.6" x2="340.36" y2="-609.6" width="0.1524" layer="91"/>
<label x="340.36" y="-609.6" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="J_EXP" gate="G$1" pin="3V3"/>
<wire x1="165.1" y1="-756.92" x2="162.56" y2="-756.92" width="0.1524" layer="91"/>
<label x="162.56" y="-756.92" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="U_MCU" gate="G$1" pin="3V3"/>
<wire x1="459.74" y1="-731.52" x2="462.28" y2="-731.52" width="0.1524" layer="91"/>
<label x="462.28" y="-731.52" size="1.27" layer="91"/>
</segment>
</net>
<net name="PWMA" class="0">
<segment>
<pinref part="U_MCU" gate="G$1" pin="IO4"/>
<wire x1="429.26" y1="-756.92" x2="426.72" y2="-756.92" width="0.1524" layer="91"/>
<label x="426.72" y="-756.92" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="U_DRV" gate="G$1" pin="IN1"/>
<wire x1="-17.78" y1="-835.66" x2="-20.32" y2="-835.66" width="0.1524" layer="91"/>
<label x="-20.32" y="-835.66" size="1.27" layer="91"/>
</segment>
</net>
<net name="PWMB" class="0">
<segment>
<pinref part="U_MCU" gate="G$1" pin="IO5"/>
<wire x1="429.26" y1="-759.46" x2="426.72" y2="-759.46" width="0.1524" layer="91"/>
<label x="426.72" y="-759.46" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="U_DRV" gate="G$1" pin="IN2"/>
<wire x1="-17.78" y1="-838.2" x2="-20.32" y2="-838.2" width="0.1524" layer="91"/>
<label x="-20.32" y="-838.2" size="1.27" layer="91"/>
</segment>
</net>
<net name="PWMC" class="0">
<segment>
<pinref part="U_MCU" gate="G$1" pin="IO6"/>
<wire x1="429.26" y1="-762" x2="426.72" y2="-762" width="0.1524" layer="91"/>
<label x="426.72" y="-762" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="U_DRV" gate="G$1" pin="IN3"/>
<wire x1="-17.78" y1="-840.74" x2="-20.32" y2="-840.74" width="0.1524" layer="91"/>
<label x="-20.32" y="-840.74" size="1.27" layer="91"/>
</segment>
</net>
<net name="Q1G" class="0">
<segment>
<pinref part="Q1" gate="G$1" pin="G"/>
<wire x1="254" y1="-227.33" x2="251.46" y2="-227.33" width="0.1524" layer="91"/>
<label x="251.46" y="-227.33" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="R_Q1" gate="G$1" pin="1"/>
<wire x1="342.9" y1="-228.6" x2="340.36" y2="-228.6" width="0.1524" layer="91"/>
<label x="340.36" y="-228.6" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="D_Q1" gate="G$1" pin="A"/>
<wire x1="431.8" y1="-228.6" x2="429.26" y2="-228.6" width="0.1524" layer="91"/>
<label x="429.26" y="-228.6" size="1.27" layer="91"/>
</segment>
</net>
<net name="ROTA" class="0">
<segment>
<pinref part="J_KNOB" gate="G$1" pin="A"/>
<wire x1="-12.7" y1="-760.73" x2="-15.24" y2="-760.73" width="0.1524" layer="91"/>
<label x="-15.24" y="-760.73" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="U_MCU" gate="G$1" pin="IO8"/>
<wire x1="429.26" y1="-767.08" x2="426.72" y2="-767.08" width="0.1524" layer="91"/>
<label x="426.72" y="-767.08" size="1.27" layer="91"/>
</segment>
</net>
<net name="ROTB" class="0">
<segment>
<pinref part="J_KNOB" gate="G$1" pin="B"/>
<wire x1="-12.7" y1="-763.27" x2="-15.24" y2="-763.27" width="0.1524" layer="91"/>
<label x="-15.24" y="-763.27" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="U_MCU" gate="G$1" pin="IO9"/>
<wire x1="429.26" y1="-769.62" x2="426.72" y2="-769.62" width="0.1524" layer="91"/>
<label x="426.72" y="-769.62" size="1.27" layer="91"/>
</segment>
</net>
<net name="STAT" class="0">
<segment>
<pinref part="R_STATUS" gate="G$1" pin="1"/>
<wire x1="254" y1="-762" x2="251.46" y2="-762" width="0.1524" layer="91"/>
<label x="251.46" y="-762" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="U_MCU" gate="G$1" pin="IO21"/>
<wire x1="459.74" y1="-756.92" x2="462.28" y2="-756.92" width="0.1524" layer="91"/>
<label x="462.28" y="-756.92" size="1.27" layer="91"/>
</segment>
</net>
<net name="STAT_A" class="0">
<segment>
<pinref part="R_STATUS" gate="G$1" pin="2"/>
<wire x1="279.4" y1="-762" x2="281.94" y2="-762" width="0.1524" layer="91"/>
<label x="281.94" y="-762" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="LED_STATUS" gate="G$1" pin="A"/>
<wire x1="342.9" y1="-762" x2="340.36" y2="-762" width="0.1524" layer="91"/>
<label x="340.36" y="-762" size="1.27" layer="91"/>
</segment>
</net>
<net name="SW" class="0">
<segment>
<pinref part="U_BUCK" gate="G$1" pin="SW"/>
<wire x1="190.5" y1="-304.8" x2="193.04" y2="-304.8" width="0.1524" layer="91"/>
<label x="193.04" y="-304.8" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="L1" gate="G$1" pin="1"/>
<wire x1="-12.7" y1="-381" x2="-15.24" y2="-381" width="0.1524" layer="91"/>
<label x="-15.24" y="-381" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="C_BOOT_B" gate="G$1" pin="2"/>
<wire x1="101.6" y1="-381" x2="104.14" y2="-381" width="0.1524" layer="91"/>
<label x="104.14" y="-381" size="1.27" layer="91"/>
</segment>
</net>
<net name="U0RXD" class="0">
<segment>
<pinref part="J_PROG" gate="G$1" pin="RXD"/>
<wire x1="12.7" y1="-231.14" x2="15.24" y2="-231.14" width="0.1524" layer="91"/>
<label x="15.24" y="-231.14" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="U_MCU" gate="G$1" pin="RXD0"/>
<wire x1="429.26" y1="-739.14" x2="426.72" y2="-739.14" width="0.1524" layer="91"/>
<label x="426.72" y="-739.14" size="1.27" layer="91"/>
</segment>
</net>
<net name="U0TXD" class="0">
<segment>
<pinref part="J_PROG" gate="G$1" pin="TXD"/>
<wire x1="12.7" y1="-228.6" x2="15.24" y2="-228.6" width="0.1524" layer="91"/>
<label x="15.24" y="-228.6" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="U_MCU" gate="G$1" pin="TXD0"/>
<wire x1="429.26" y1="-741.68" x2="426.72" y2="-741.68" width="0.1524" layer="91"/>
<label x="426.72" y="-741.68" size="1.27" layer="91"/>
</segment>
</net>
<net name="USBDM_RAW" class="0">
<segment>
<pinref part="R_DM" gate="G$1" pin="1"/>
<wire x1="254" y1="-152.4" x2="251.46" y2="-152.4" width="0.1524" layer="91"/>
<label x="251.46" y="-152.4" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="J_USB" gate="G$1" pin="DN1"/>
<wire x1="71.12" y1="-838.2" x2="68.58" y2="-838.2" width="0.1524" layer="91"/>
<label x="68.58" y="-838.2" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="J_USB" gate="G$1" pin="DN2"/>
<wire x1="106.68" y1="-835.66" x2="109.22" y2="-835.66" width="0.1524" layer="91"/>
<label x="109.22" y="-835.66" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="D_USB" gate="G$1" pin="I/O1_IN"/>
<wire x1="157.48" y1="-825.5" x2="154.94" y2="-825.5" width="0.1524" layer="91"/>
<label x="154.94" y="-825.5" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="D_USB" gate="G$1" pin="I/O1_OUT"/>
<wire x1="198.12" y1="-825.5" x2="200.66" y2="-825.5" width="0.1524" layer="91"/>
<label x="200.66" y="-825.5" size="1.27" layer="91"/>
</segment>
</net>
<net name="USBDP_RAW" class="0">
<segment>
<pinref part="R_DP" gate="G$1" pin="1"/>
<wire x1="165.1" y1="-152.4" x2="162.56" y2="-152.4" width="0.1524" layer="91"/>
<label x="162.56" y="-152.4" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="J_USB" gate="G$1" pin="DP1"/>
<wire x1="71.12" y1="-835.66" x2="68.58" y2="-835.66" width="0.1524" layer="91"/>
<label x="68.58" y="-835.66" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="J_USB" gate="G$1" pin="DP2"/>
<wire x1="106.68" y1="-838.2" x2="109.22" y2="-838.2" width="0.1524" layer="91"/>
<label x="109.22" y="-838.2" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="D_USB" gate="G$1" pin="I/O2_IN"/>
<wire x1="157.48" y1="-850.9" x2="154.94" y2="-850.9" width="0.1524" layer="91"/>
<label x="154.94" y="-850.9" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="D_USB" gate="G$1" pin="I/O2_OUT"/>
<wire x1="198.12" y1="-850.9" x2="200.66" y2="-850.9" width="0.1524" layer="91"/>
<label x="200.66" y="-850.9" size="1.27" layer="91"/>
</segment>
</net>
<net name="USB_DM_MCU" class="0">
<segment>
<pinref part="R_DM" gate="G$1" pin="2"/>
<wire x1="279.4" y1="-152.4" x2="281.94" y2="-152.4" width="0.1524" layer="91"/>
<label x="281.94" y="-152.4" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="U_MCU" gate="G$1" pin="IO19"/>
<wire x1="459.74" y1="-751.84" x2="462.28" y2="-751.84" width="0.1524" layer="91"/>
<label x="462.28" y="-751.84" size="1.27" layer="91"/>
</segment>
</net>
<net name="USB_DP_MCU" class="0">
<segment>
<pinref part="R_DP" gate="G$1" pin="2"/>
<wire x1="190.5" y1="-152.4" x2="193.04" y2="-152.4" width="0.1524" layer="91"/>
<label x="193.04" y="-152.4" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="U_MCU" gate="G$1" pin="IO20"/>
<wire x1="459.74" y1="-754.38" x2="462.28" y2="-754.38" width="0.1524" layer="91"/>
<label x="462.28" y="-754.38" size="1.27" layer="91"/>
</segment>
</net>
<net name="VBUS" class="2">
<segment>
<pinref part="C_VBUS" gate="G$1" pin="1"/>
<wire x1="342.9" y1="-152.4" x2="340.36" y2="-152.4" width="0.1524" layer="91"/>
<label x="340.36" y="-152.4" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="D_OR_USB" gate="G$1" pin="A"/>
<wire x1="431.8" y1="-152.4" x2="429.26" y2="-152.4" width="0.1524" layer="91"/>
<label x="429.26" y="-152.4" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="J_USB" gate="G$1" pin="VBUS"/>
<wire x1="106.68" y1="-828.04" x2="109.22" y2="-828.04" width="0.1524" layer="91"/>
<label x="109.22" y="-828.04" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="D_USB" gate="G$1" pin="VBUS"/>
<wire x1="198.12" y1="-838.2" x2="200.66" y2="-838.2" width="0.1524" layer="91"/>
<label x="200.66" y="-838.2" size="1.27" layer="91"/>
</segment>
</net>
<net name="VCP" class="0">
<segment>
<pinref part="C_VCP" gate="G$1" pin="1"/>
<wire x1="165.1" y1="-457.2" x2="162.56" y2="-457.2" width="0.1524" layer="91"/>
<label x="162.56" y="-457.2" size="1.27" layer="91"/>
</segment>
<segment>
<pinref part="U_DRV" gate="G$1" pin="VCP"/>
<wire x1="17.78" y1="-810.26" x2="20.32" y2="-810.26" width="0.1524" layer="91"/>
<label x="20.32" y="-810.26" size="1.27" layer="91"/>
</segment>
</net>
</nets>
</sheet>
</sheets>
</schematic>
</drawing>
</eagle>
