#!/usr/bin/env python3
import argparse
import os
import struct
import sys

BOOT_OFF = 0x0000
BOOT_SIZE = 0x440
BI2_OFF = 0x0440
BI2_SIZE = 0x2000
APPLOADER_HDR_OFF = 0x2440
APPLOADER_HDR_SIZE = 0x20
APPLOADER_CODE_OFF = 0x2460
APPLOADER_ENTRY = 0x81200000
DOL_MIN_OFF = 0x10000
FST_ALIGN = 32
DISC_ALIGN = 32 * 1024


def align_up(value, alignment):
    return (value + alignment - 1) & ~(alignment - 1)


def fail(message):
    sys.exit("error: " + message)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--dol", required=True)
    parser.add_argument("--apploader", required=True)
    parser.add_argument("--out", required=True)
    parser.add_argument("--id", required=True)
    parser.add_argument("--maker", required=True)
    parser.add_argument("--name", required=True)
    args = parser.parse_args()

    with open(args.dol, "rb") as f:
        dol = f.read()
    with open(args.apploader, "rb") as f:
        apploader = f.read()

    if len(dol) <= 0x100:
        fail("DOL file is empty or too small")
    if len(apploader) >= 0x20000:
        fail("apploader is too large")
    if len(args.id) != 4:
        fail("game ID must be 4 characters")
    if len(args.maker) != 2:
        fail("maker code must be 2 characters")

    name = args.name.encode("ascii")
    if len(name) > 0x3E0:
        fail("game name is longer than 0x3E0 bytes")

    apploader_size = align_up(len(apploader), 32)
    dol_off = max(align_up(APPLOADER_CODE_OFF + apploader_size, 0x100), DOL_MIN_OFF)
    fst_off = align_up(dol_off + len(dol), FST_ALIGN)

    fst = struct.pack(">III", 0x01000000, 0, 1) + b"\x00"
    fst += b"\x00" * (align_up(len(fst), FST_ALIGN) - len(fst))
    fst_size = len(fst)

    total = align_up(fst_off + fst_size, DISC_ALIGN)

    image = bytearray(total)

    image[0x00:0x04] = args.id.encode("ascii")
    image[0x04:0x06] = args.maker.encode("ascii")
    image[0x06] = 0
    image[0x07] = 0
    struct.pack_into(">I", image, 0x1C, 0xC2339F3D)
    image[0x20:0x20 + len(name)] = name
    struct.pack_into(">I", image, 0x420, dol_off)
    struct.pack_into(">I", image, 0x424, fst_off)
    struct.pack_into(">I", image, 0x428, fst_size)
    struct.pack_into(">I", image, 0x42C, fst_size)

    struct.pack_into(">I", image, BI2_OFF + 0x04, 0x01800000)
    struct.pack_into(">I", image, BI2_OFF + 0x18, 1)

    date = b"2026/11/10"
    image[APPLOADER_HDR_OFF:APPLOADER_HDR_OFF + len(date)] = date
    struct.pack_into(">I", image, APPLOADER_HDR_OFF + 0x10, APPLOADER_ENTRY)
    struct.pack_into(">I", image, APPLOADER_HDR_OFF + 0x14, apploader_size)
    struct.pack_into(">I", image, APPLOADER_HDR_OFF + 0x18, 0)

    image[APPLOADER_CODE_OFF:APPLOADER_CODE_OFF + len(apploader)] = apploader
    image[dol_off:dol_off + len(dol)] = dol
    image[fst_off:fst_off + fst_size] = fst

    out_dir = os.path.dirname(args.out)
    if out_dir:
        os.makedirs(out_dir, exist_ok=True)
    with open(args.out, "wb") as f:
        f.write(image)

    print("boot.bin:         0x%08x" % BOOT_OFF)
    print("bi2.bin:          0x%08x" % BI2_OFF)
    print("apploader header: 0x%08x" % APPLOADER_HDR_OFF)
    print("apploader code:   0x%08x (0x%x bytes)" % (APPLOADER_CODE_OFF, apploader_size))
    print("DOL:              0x%08x (0x%x bytes)" % (dol_off, len(dol)))
    print("FST:              0x%08x (0x%x bytes)" % (fst_off, fst_size))
    print("total:            0x%08x" % total)


if __name__ == "__main__":
    main()
