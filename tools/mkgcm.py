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
MAX_DISC_SIZE = 0x57058000


def align_up(value, alignment):
    return (value + alignment - 1) & ~(alignment - 1)


def fail(message):
    sys.exit("error: " + message)


class Entry:
    def __init__(self, name, path, is_dir, parent):
        self.name = name
        self.path = path
        self.is_dir = is_dir
        self.parent = parent
        self.next = 0
        self.offset = 0
        self.size = 0


def scan_tree(root):
    entries = [Entry("", root, True, 0)]

    def add_children(directory, parent_index):
        with os.scandir(directory.path) as it:
            items = list(it)
        dirs = sorted([e for e in items if e.is_dir()],
                      key=lambda e: e.name)
        files = sorted([e for e in items if e.is_file()],
                       key=lambda e: e.name)

        for item in dirs:
            entry = Entry(item.name, item.path, True, parent_index)
            index = len(entries)
            entries.append(entry)
            add_children(entry, index)

        for item in files:
            entry = Entry(item.name, item.path, False, parent_index)
            entries.append(entry)

        entries[parent_index].next = len(entries)

    add_children(entries[0], 0)
    return entries


def build_fst(entries, fst_off):
    strings = bytearray(b"\x00")
    name_offsets = [0]

    for entry in entries[1:]:
        name_offsets.append(len(strings))
        strings += entry.name.encode("ascii", "strict") + b"\x00"

    raw_size = len(entries) * 12 + len(strings)
    fst_size = align_up(raw_size, FST_ALIGN)

    cursor = align_up(fst_off + fst_size, DISC_ALIGN)
    for entry in entries:
        if not entry.is_dir:
            entry.offset = cursor
            entry.size = os.path.getsize(entry.path)
            cursor = align_up(cursor + entry.size, DISC_ALIGN)

    fst = bytearray()
    for index, entry in enumerate(entries):
        name_off = name_offsets[index]
        if entry.is_dir:
            word1 = entry.parent if index else 0
            word2 = entry.next if index else len(entries)
        else:
            word1 = entry.offset
            word2 = entry.size

        fst += bytes((1 if entry.is_dir else 0,
                      (name_off >> 16) & 0xFF,
                      (name_off >> 8) & 0xFF,
                      name_off & 0xFF))
        fst += struct.pack(">II", word1, word2)

    fst += strings
    fst += b"\x00" * (fst_size - len(fst))
    return bytes(fst), fst_size, cursor


def write_zeros(out, count):
    chunk = b"\x00" * min(count, 1024 * 1024)
    while count > 0:
        n = min(count, len(chunk))
        out.write(chunk[:n])
        count -= n


def write_file(out, path):
    with open(path, "rb") as src:
        while True:
            data = src.read(1024 * 1024)
            if not data:
                break
            out.write(data)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--dol", required=True)
    parser.add_argument("--apploader", required=True)
    parser.add_argument("--out", required=True)
    parser.add_argument("--id", required=True)
    parser.add_argument("--maker", required=True)
    parser.add_argument("--name", required=True)
    parser.add_argument("--root")
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

    entries = None
    files = []
    if args.root:
        if not os.path.isdir(args.root):
            fail("root is not a directory: " + args.root)
        entries = scan_tree(args.root)
        fst, fst_size, total = build_fst(entries, fst_off)
        files = [e for e in entries if not e.is_dir]
    else:
        fst = struct.pack(">III", 0x01000000, 0, 1) + b"\x00"
        fst += b"\x00" * (align_up(len(fst), FST_ALIGN) - len(fst))
        fst_size = len(fst)
        total = align_up(fst_off + fst_size, DISC_ALIGN)

    if total > MAX_DISC_SIZE:
        fail("image is larger than the GameCube disc size")

    header_size = APPLOADER_CODE_OFF + apploader_size
    header = bytearray(header_size)

    header[0x00:0x04] = args.id.encode("ascii")
    header[0x04:0x06] = args.maker.encode("ascii")
    header[0x06] = 0
    header[0x07] = 0
    struct.pack_into(">I", header, 0x1C, 0xC2339F3D)
    header[0x20:0x20 + len(name)] = name
    struct.pack_into(">I", header, 0x420, dol_off)
    struct.pack_into(">I", header, 0x424, fst_off)
    struct.pack_into(">I", header, 0x428, fst_size)
    struct.pack_into(">I", header, 0x42C, fst_size)

    struct.pack_into(">I", header, BI2_OFF + 0x04, 0x01800000)
    struct.pack_into(">I", header, BI2_OFF + 0x18, 1)

    date = b"2026/11/10"
    header[APPLOADER_HDR_OFF:APPLOADER_HDR_OFF + len(date)] = date
    struct.pack_into(">I", header, APPLOADER_HDR_OFF + 0x10, APPLOADER_ENTRY)
    struct.pack_into(">I", header, APPLOADER_HDR_OFF + 0x14, apploader_size)
    struct.pack_into(">I", header, APPLOADER_HDR_OFF + 0x18, 0)

    header[APPLOADER_CODE_OFF:APPLOADER_CODE_OFF + len(apploader)] = apploader

    out_dir = os.path.dirname(args.out)
    if out_dir:
        os.makedirs(out_dir, exist_ok=True)

    with open(args.out, "wb") as out:
        out.write(header)
        write_zeros(out, dol_off - len(header))
        out.write(dol)
        write_zeros(out, fst_off - (dol_off + len(dol)))
        out.write(fst)
        data_start = align_up(fst_off + fst_size, DISC_ALIGN)
        write_zeros(out, data_start - (fst_off + fst_size))
        for entry in files:
            write_file(out, entry.path)
            write_zeros(out, align_up(entry.offset + entry.size, DISC_ALIGN) -
                        (entry.offset + entry.size))

    print("boot.bin:         0x%08x" % BOOT_OFF)
    print("bi2.bin:          0x%08x" % BI2_OFF)
    print("apploader header: 0x%08x" % APPLOADER_HDR_OFF)
    print("apploader code:   0x%08x (0x%x bytes)" % (APPLOADER_CODE_OFF, apploader_size))
    print("DOL:              0x%08x (0x%x bytes)" % (dol_off, len(dol)))
    print("FST:              0x%08x (0x%x bytes)" % (fst_off, fst_size))
    print("total:            0x%08x" % total)


if __name__ == "__main__":
    main()
