#!/usr/bin/env python3
import struct, sys
MAGIC = 0x31534654
NAME = 64
files = [("/bin/init", open(sys.argv[1], "rb").read())]
header = struct.pack("<II", MAGIC, len(files))
table_size = len(files) * (NAME + 8)
offset = len(header) + table_size
entries = []
for name, data in files:
    raw = name.encode()[:NAME-1] + b"\0"
    raw += b"\0" * (NAME-len(raw))
    entries.append(struct.pack(f"<{NAME}sII", raw, offset, len(data)))
    offset += len(data)
with open(sys.argv[2], "wb") as out:
    out.write(header)
    for e in entries: out.write(e)
    for _, data in files: out.write(data)
