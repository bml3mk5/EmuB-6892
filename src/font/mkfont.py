#!/usr/bin/python
# マーク５エミュレータのフォントファイルを作成
#
# Copyright (c) 2020, Sasaji

import struct

infile="display_font.bmp"
outfile="FONT.ROM"

print(f'-------- {infile}')

## @param[in] data: input file
def check_bmp_header(data: bytes) -> int:
    global offset

    # file header 14 bytes
    if data.find(b'BM', 0, 2) != 0:
        print('This is not BMP format.')
        return 1
    
    offset = struct.unpack('<I', data[0x0a:0x0e])
    print(f'offset: {offset[0]}')

    # info header
    infosize = struct.unpack('<I', data[0x0e:0x12])
    if infosize[0] != 40:
        print('Windows BMP format only.')
        return 1
    
    width = struct.unpack('<I', data[0x12:0x16])
    if width[0] != 256:
        print('width must be 256 pixel.')
        return 1
    
    height = struct.unpack('<I', data[0x16:0x1a])
    if height[0] != 128:
        print('height must be 128 pixel.')
        return 1
    
    bpp = struct.unpack('<H', data[0x1c:0x1e])
    if bpp[0] != 1:
        print('Supported data is only 1bit per pixel(B/W data).')
        return 1
    
    compress = struct.unpack('<I', data[0x1e:0x22])
    if compress[0] != 0:
        print('Supported data is only no compression.')
        return 1

    return 0

## main

with open(infile, "rb") as f:
    indata = f.read()

offset = [0]
if check_bmp_header(indata):
    raise Exception('Invalid format in the input file.')

outdatas = dict()
chcodes = [0x180, 0x1a0, 0x1c0, 0x1e0]
line = 0
while line < 4:
    chcode = chcodes[line]
    x = 0
    while x < 32:
        print('{0:2X}:'.format(chcode), end = '')
        outdatas[chcode]=bytes()
        y = 0
        while y < 16:
            pos = (3 - line) * 16 * 32 + (15 - y) * 32 + x + offset[0]
            val = indata[pos]
            print(' {0:2X}'.format(val), end = '')
            outdatas[chcode] += struct.pack('B', val)
            y += 1
        print()
        chcode += 1
        x += 1
    line += 1


chcodes=[0x00, 0x20, 0x40, 0x60, 0x80, 0xa0, 0xc0, 0xe0]
line = 0
while line < 8:
    chcode = chcodes[line]
    x = 0
    while x < 32:
        print('{0:2X}:'.format(chcode), end = '')
        outdatas[chcode]=bytes()
        y = 0
        while y < 8:
            pos = (15 - line) * 8 * 32 + (7 - y) * 32 + x + offset[0]
            val = indata[pos]
            print(' {0:2X}'.format(val), end = '')
            outdatas[chcode] += struct.pack('B', val)
            y += 1
        print()
        chcode += 1
        x += 1
    line += 1

outdata = bytes()
chcode = 0x00
while chcode < 0x80:
    x = 0
    while x < 8:
        outdata += struct.pack('B', outdatas[chcode][x])
        outdata += struct.pack('B', outdatas[chcode + 0x80][x])
        x += 1
    chcode += 1

chcode = 0x180
while chcode < 0x200:
    x = 0
    while x < 16:
        outdata += struct.pack('B', outdatas[chcode][x])
        x += 1
    chcode += 1

with open(outfile, "wb") as f:
    f.write(outdata)

input('Complete.')


