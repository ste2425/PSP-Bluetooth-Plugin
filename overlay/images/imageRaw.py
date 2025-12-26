# This is AI made. It converts an image to a raw c array to be embedded directly.

#!/usr/bin/env python3
# png_to_carray.py
from PIL import Image
import sys
import os
import argparse

#print("Usage: python imageRaw.py input.png output.h varname [--width W] [--height H]")
parser = argparse.ArgumentParser()
parser.add_argument('input_png', help='PNG to parse')
parser.add_argument('output', help='output file')
parser.add_argument('varname', help='variable name to use in output')
parser.add_argument('--width', type=int, default=None, help='override image width (scales image)')
parser.add_argument('--height', type=int, default=None, help='override image height (scales image)')
args = parser.parse_args()

inp = args.input_png
out = args.output
varname = args.varname

im = Image.open(inp).convert("RGBA")
w, h = im.size

# Apply scaling if width or height overrides are provided
if args.width is not None or args.height is not None:
    new_w = args.width if args.width is not None else w
    new_h = args.height if args.height is not None else h
    im = im.resize((new_w, new_h), Image.Resampling.LANCZOS)
    w, h = new_w, new_h
data = im.tobytes()  # RGBA order

with open(out, "a+") as f:
    f.write(f"// {varname} --------------------------------\n")
    f.write(f"#define {varname}_W {w}\n")
    f.write(f"#define {varname}_H {h}\n\n")
    f.write(f"static const unsigned char {varname}[] = {{\n")
    for i, b in enumerate(data):
        if i % 12 == 0:
            f.write("    ")
        f.write("0x%02X, " % b)
        if i % 12 == 11:
            f.write("\n")
    if len(data) % 12 != 0:
        f.write("\n")
    f.write("};\n")
    f.write(f"static const unsigned int {varname}_len = {len(data)};\n")
    f.write("//--------------------------------\n")

print(f"Wrote {out} ({w}x{h}, {len(data)} bytes)")
