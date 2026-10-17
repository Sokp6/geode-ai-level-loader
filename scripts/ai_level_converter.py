#!/usr/bin/env python3
"""
AI Level Converter for Geometry Dash (Geode Mod Companion)
Converts JSON level definitions into native Geometry Dash level strings.
"""

import sys
import json
import argparse
import subprocess
import shutil

EASING_MAP = {
    "none": 0, "ease_in_out": 1, "ease_in": 2, "ease_out": 3,
    "elastic_in_out": 4, "elastic_in": 5, "elastic_out": 6,
    "bounce_in_out": 7, "bounce_in": 8, "bounce_out": 9,
    "exponential_in_out": 10, "exponential_in": 11, "exponential_out": 12,
    "sine_in_out": 13, "sine_in": 14, "sine_out": 15,
    "back_in_out": 16, "back_in": 17, "back_out": 18
}

COLOR_CHANNELS = {
    "bg": 1000, "background": 1000,
    "g1": 1001, "ground1": 1001,
    "line": 1002, "3dl": 1003,
    "obj": 1004, "object": 1004,
    "g2": 1009, "ground2": 1009
}

OBJECT_TYPES = {
    "block": 1, "block_outline": 6, "slab": 7, "half_block": 7, "slab_outline": 8,
    "spike": 2, "spike_short": 3, "spike_hanging": 4, "spike_upsidedown": 5,
    "orb_yellow": 35, "orb_pink": 84, "orb_red": 140, "orb_blue": 1022,
    "orb_green": 1005, "orb_black": 1704, "orb_dash_green": 1332, "orb_dash_pink": 1333,
    "pad_yellow": 36, "pad_pink": 141, "pad_red": 1331, "pad_blue": 67,
    "coin": 1329,
    "portal_cube": 10, "portal_ship": 11, "portal_ball": 12, "portal_ufo": 13,
    "portal_wave": 47, "portal_robot": 1330, "portal_spider": 1331, "portal_swing": 2013,
    "portal_speed_slow": 13, "portal_speed_normal": 12, "portal_speed_fast": 47,
    "portal_speed_faster": 200, "portal_speed_fastest": 1334,
    "portal_gravity_down": 10, "portal_gravity_up": 11,
    "portal_size_normal": 14, "portal_size_mini": 101,
    "trigger_color": 899, "trigger_move": 901, "trigger_pulse": 1006,
    "trigger_alpha": 1007, "trigger_toggle": 1049, "trigger_spawn": 1268,
    "trigger_rotate": 1815, "trigger_shake": 1520, "trigger_stop": 1611
}

def get_default_header(speed="1x", bg_color=(40, 125, 255), ground_color=(0, 102, 255)):
    speed_val = {"0.5x": 1, "1x": 0, "2x": 2, "3x": 3, "4x": 4}.get(speed, 0)
    bg_r, bg_g, bg_b = bg_color
    g_r, g_g, g_b = ground_color
    return (
        f"kS38,1_{bg_r}_2_{bg_g}_3_{bg_b}_11_255_12_255_13_255_4_-1_6_1000_7_1_15_1_18_0_8_1|"
        f"1_{g_r}_2_{g_g}_3_{g_b}_11_255_12_255_13_255_4_-1_6_1001_7_1_15_1_18_0_8_1,"
        f"kA13,0,kA15,0,kA16,0,kA14,0,kA6,0,kA7,0,kA17,0,kA18,0,kS39,0,kA2,{speed_val},"
        "kA3,0,kA8,0,kA4,0,kA9,0,kA10,0,kA11,0;"
    )

def convert_json_to_gd(data):
    bg_color = tuple(data.get("bg_color", [40, 125, 255]))
    ground_color = tuple(data.get("ground_color", [0, 102, 255]))
    speed = data.get("speed", "1x")
    
    header = get_default_header(speed, bg_color, ground_color)
    objects = data.get("objects", [])
    
    obj_strs = []
    trigger_count = 0

    for item in objects:
        obj_id = item.get("id", OBJECT_TYPES.get(item.get("type", "block"), 1))
        
        # Position
        pos_x = item.get("raw_x", item.get("x", 0) * 30 + 15)
        pos_y = item.get("raw_y", item.get("y", 0) * 30 + 15)
        
        parts = [f"1,{obj_id}", f"2,{pos_x}", f"3,{pos_y}"]
        
        if "rotation" in item:
            parts.append(f"6,{item['rotation']}")
        if item.get("flip_x"):
            parts.append("4,1")
        if item.get("flip_y"):
            parts.append("5,1")
        if "groups" in item and item["groups"]:
            parts.append(f"57,{'.'.join(str(g) for g in item['groups'])}")
            
        # Triggers
        if obj_id == 901:  # Move
            trigger_count += 1
            grp = item.get("target_group", 0)
            mx = item.get("move_x_blocks", 0) * 10 if "move_x_blocks" in item else item.get("move_x", 0)
            my = item.get("move_y_blocks", 0) * 10 if "move_y_blocks" in item else item.get("move_y", 0)
            dur = item.get("duration", 0.5)
            easing = EASING_MAP.get(str(item.get("easing", "none")), 0)
            parts.extend([f"28,{grp}", f"30,{mx}", f"31,{my}", f"10,{dur}", f"35,{easing}"])
        elif obj_id == 899:  # Color
            trigger_count += 1
            ch = COLOR_CHANNELS.get(str(item.get("channel", "bg")), 1000)
            r = item.get("r", 255)
            g = item.get("g", 255)
            b = item.get("b", 255)
            dur = item.get("duration", 0.0)
            blend = 1 if item.get("blending") else 0
            parts.extend([f"23,{ch}", f"7,{r}", f"8,{g}", f"9,{b}", f"10,{dur}", f"17,{blend}"])
        elif obj_id == 1006:  # Pulse
            trigger_count += 1
            ttype = 1 if "target_group" in item else 0
            tid = item.get("target_group", COLOR_CHANNELS.get(str(item.get("channel", "bg")), 1000))
            r = item.get("r", 255)
            g = item.get("g", 255)
            b = item.get("b", 255)
            fade_in = item.get("fade_in", 0.0)
            hold = item.get("hold", 0.0)
            fade_out = item.get("fade_out", 0.0)
            parts.extend([f"51,{ttype}", f"52,{tid}", f"7,{r}", f"8,{g}", f"9,{b}", f"45,{fade_in}", f"46,{hold}", f"47,{fade_out}", "48,0"])
        elif obj_id == 1007:  # Alpha
            trigger_count += 1
            grp = item.get("target_group", 0)
            opacity = item.get("opacity", 1.0)
            dur = item.get("duration", 0.0)
            parts.extend([f"28,{grp}", f"85,{opacity}", f"10,{dur}"])
        elif obj_id == 1815:  # Rotate
            trigger_count += 1
            grp = item.get("target_group", 0)
            cgrp = item.get("center_group", 0)
            deg = item.get("degrees", 0)
            times360 = item.get("times_360", 0)
            lock = 1 if item.get("lock_rotation") else 0
            dur = item.get("duration", 1.0)
            parts.extend([f"28,{grp}", f"29,{cgrp}", f"68,{deg}", f"69,{times360}", f"70,{lock}", f"10,{dur}"])
        elif obj_id == 1520:  # Shake
            trigger_count += 1
            strength = item.get("strength", 2.0)
            interval = item.get("interval", 0.05)
            dur = item.get("duration", 0.5)
            parts.extend([f"75,{strength}", f"76,{interval}", f"10,{dur}"])
        elif obj_id == 1049:  # Toggle
            trigger_count += 1
            grp = item.get("target_group", 0)
            act = 1 if item.get("activate", False) else 0
            parts.extend([f"28,{grp}", f"56,{act}"])
        elif obj_id == 1268:  # Spawn
            trigger_count += 1
            grp = item.get("target_group", 0)
            delay = item.get("delay", 0.0)
            parts.extend([f"28,{grp}", f"63,{delay}"])
        elif obj_id == 1611:  # Stop
            trigger_count += 1
            grp = item.get("target_group", 0)
            parts.append(f"28,{grp}")
        elif obj_id in (899, 901, 1006, 1007, 1049, 1268, 1520, 1611, 1815):
            trigger_count += 1

        obj_strs.append(",".join(parts) + ";")

    objects_data = "".join(obj_strs)
    full_level = header + objects_data
    return full_level, objects_data, len(objects), trigger_count

def copy_to_clipboard(text):
    if shutil.which("termux-clipboard-set"):
        p = subprocess.Popen(["termux-clipboard-set"], stdin=subprocess.PIPE)
        p.communicate(input=text.encode("utf-8"))
        return True
    return False

def main():
    parser = argparse.ArgumentParser(description="Convert AI level JSON to Geometry Dash level string")
    parser.add_argument("input", help="Path to input JSON level file")
    parser.add_argument("-o", "--output", help="Output file (e.g. /sdcard/Download/ai_level.json)")
    parser.add_argument("--clipboard", action="store_true", help="Copy objects string to Android/system clipboard")
    parser.add_argument("--print-raw", action="store_true", help="Print raw GD string to stdout")

    args = parser.parse_args()

    with open(args.input, "r", encoding="utf-8") as f:
        data = json.load(f)

    full_string, obj_string, total_objs, total_triggers = convert_json_to_gd(data)

    print(f"✨ Level '{data.get('name', 'Untitled')}' converted successfully!")
    print(f"📦 Total objects: {total_objs}")
    print(f"⚡ Total triggers: {total_triggers}")

    if args.output:
        # If output ends with .json, write formatted JSON to that destination
        if args.output.endswith(".json"):
            with open(args.output, "w", encoding="utf-8") as f:
                json.dump(data, f, indent=2, ensure_ascii=False)
            print(f"💾 Copied JSON to: {args.output}")
        else:
            with open(args.output, "w", encoding="utf-8") as f:
                f.write(full_string)
            print(f"💾 Saved GD level string to: {args.output}")

    if args.clipboard:
        if copy_to_clipboard(obj_string):
            print("📋 Objects copied to clipboard! You can paste them in the GD editor.")
        else:
            print("⚠️ termux-clipboard-set not found. Install 'termux-api' or use stdout.")

    if args.print_raw:
        print("\n--- GD LEVEL STRING ---")
        print(full_string)

if __name__ == "__main__":
    main()
