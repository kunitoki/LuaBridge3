#!/usr/bin/env python3

import sys
import xml.etree.ElementTree as ET
from collections import defaultdict


def extract_uncovered(root):
    """
    Returns:
        dict[str, list[int]]  -> filename -> uncovered line numbers
    """
    result = defaultdict(list)

    for cls in root.findall(".//class"):
        filename = cls.attrib.get("filename", "unknown")
        lines = cls.find("lines")
        if lines is None:
            continue

        for line in lines.findall("line"):
            if line.attrib.get("hits") == "0":
                result[filename].append(int(line.attrib["number"]))

    return result


def compress_ranges(numbers):
    """
    Converts sorted list like [1,2,3,5,6,10] -> ["1-3","5-6","10"]
    """
    if not numbers:
        return []

    numbers = sorted(set(numbers))
    ranges = []

    start = prev = numbers[0]

    for n in numbers[1:]:
        if n == prev + 1:
            prev = n
        else:
            ranges.append((start, prev))
            start = prev = n

    ranges.append((start, prev))

    # Format
    out = []
    for s, e in ranges:
        if s == e:
            out.append(f"{s}")
        else:
            out.append(f"{s}-{e}")

    return out


def write_ultra_compact(data, output_path):
    """
    Writes:
        file:range,range,...
    """
    with open(output_path, "w", encoding="utf-8") as f:
        for filename in sorted(data.keys()):
            ranges = compress_ranges(data[filename])
            if ranges:
                f.write(f"{filename}:{','.join(ranges)}\n")


def main():
    if len(sys.argv) != 3:
        print(f"Usage: {sys.argv[0]} input.xml output.txt")
        sys.exit(1)

    input_path = sys.argv[1]
    output_path = sys.argv[2]

    tree = ET.parse(input_path)
    root = tree.getroot()

    uncovered = extract_uncovered(root)
    write_ultra_compact(uncovered, output_path)


if __name__ == "__main__":
    main()