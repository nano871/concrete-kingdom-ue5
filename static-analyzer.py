#!/usr/bin/env python3
"""
Concrete Kingdom UE5 C++ Static Analyzer
Run before every push from either machine (Linux or Windows w/ Python).
Checks for common UHT/compile errors that crash the build.
"""

import os
import re
import sys
from pathlib import Path

# Detect repo root (this script lives at repo root)
SCRIPT_DIR = Path(__file__).resolve().parent
SOURCE_DIR = SCRIPT_DIR / "Source" / "ConcreteKingdom"

errors = []
warnings = []

def e(file, line, msg):
    errors.append(f"  [ERROR]   {file}:{line}  {msg}")

def w(file, line, msg):
    warnings.append(f"  [WARNING] {file}:{line}  {msg}")


def read_lines(path):
    try:
        with open(path, encoding="utf-8") as f:
            return f.readlines()
    except Exception as ex:
        e(path.name, 0, f"Cannot read: {ex}")
        return []


def check_file(path):
    lines = read_lines(path)
    if not lines:
        return
    name = path.name
    ext = path.suffix
    content = "".join(lines)

    # === HEADER FILES ===
    if ext == ".h":

        # 1. Must have #pragma once or include guard
        if "#pragma once" not in content and not re.search(r'#ifndef\s+[A-Z_]', content):
            e(name, 1, "Missing #pragma once or include guard")

        # 2. UClass/UStruct/UEnum without GENERATED_BODY()
        macros = re.findall(r'(UCLASS|USTRUCT|UENUM|UINTERFACE)\s*\(', content)
        has_gen = "GENERATED_BODY()" in content
        if macros and not has_gen:
            # More precise: count open braces after each macro
            e(name, 1, f"{len(macros)} U-macro(s) but no GENERATED_BODY() found")

        # 3. GENERATED_BODY() inside USTRUCT should be followed by ;
        for i, line in enumerate(lines, 1):
            s = line.strip()
            if "GENERATED_BODY()" in s and not s.endswith(";"):
                # In USTRUCT context, GENERATED_BODY() needs semicolon
                w(name, i, "GENERATED_BODY() without trailing ';' — ok in UCLASS, error in USTRUCT")

    # === SOURCE FILES ===
    if ext == ".cpp":

        # 1. Empty/stub function bodies
        func_pattern = re.compile(
            r'^\s*(?:virtual\s+|static\s+)?'
            r'(?:void|bool|int32|int64|float|double|uint8|uint16|uint32|uint64|F\w+|T\w+|U\w+|A\w+)'
            r'(?:\s*\*)?\s+\w+::\w+\s*\('
        )
        i = 0
        while i < len(lines):
            line = lines[i]
            if func_pattern.match(line):
                func_start = i
                brace_depth = 0
                started = False
                j = i
                while j < len(lines):
                    brace_depth += lines[j].count("{") - lines[j].count("}")
                    if brace_depth > 0:
                        started = True
                    if brace_depth <= 0 and started:
                        # Function ended
                        body_lines = lines[func_start:j+1]
                        body_text = "".join(body_lines)
                        fb = body_text.find("{")
                        lb = body_text.rfind("}")
                        if fb >= 0 and lb > fb:
                            inner = body_text[fb+1:lb].strip()
                            meaningful = [
                                l for l in inner.split("\n")
                                if l.strip() and not l.strip().startswith("//")
                                and not l.strip().startswith("UE_LOG")
                                and not l.strip().startswith("Super::")
                                and not l.strip().startswith("PrimaryComponentTick")
                                and not l.strip().startswith("bConnected")
                                and not "return" in l.strip()
                            ]
                            if len(meaningful) == 0 and len(inner) > 0:
                                e(name, func_start+1, "Stub function — only logs/returns")
                        i = j
                        break
                    j += 1
            i += 1

    # === BOTH ===
    for i, line in enumerate(lines, 1):
        s = line.strip()
        if "TODO" in s:
            w(name, i, f"TODO remains: {s[:80]}")
        if "FIXME" in s:
            w(name, i, f"FIXME remains: {s[:80]}")

    # Trailing whitespace
    for i, line in enumerate(lines, 1):
        if i < len(lines) and line.rstrip("\n").endswith((" ", "\t")):
            if any(kw in line for kw in ["#include", "//", "/*", "*"]):
                continue
            w(name, i, "Trailing whitespace")


def check_build_cs():
    for bf in SOURCE_DIR.parent.glob("*.Build.cs"):
        content = "".join(read_lines(bf))
        name = bf.name
        if "Json" not in content:
            w(name, 0, "Module 'Json' missing from dependency list")
        for dep in ["Core", "CoreUObject", "Engine"]:
            if dep not in content:
                e(name, 0, f"Required module '{dep}' missing")


def main():
    pad = "=" * 58
    print(pad)
    print("  Concrete Kingdom UE5 C++ Static Analyzer")
    print(pad)

    if not SOURCE_DIR.exists():
        print(f"\n  Source directory not found: {SOURCE_DIR}")
        print(f"  Run this script from the repo root.\n")
        return 1

    cpps = sorted(SOURCE_DIR.glob("*.cpp"))
    hdrs = sorted(SOURCE_DIR.glob("*.h"))
    print(f"\n  Scanning {len(cpps)} .cpp + {len(hdrs)} .h files\n")

    for f in cpps + hdrs:
        check_file(f)

    check_build_cs()

    print(pad)
    print(f"  {len(errors)} errors  |  {len(warnings)} warnings")
    print(pad)
    print()

    if errors:
        print("ERRORS — fix these before pushing:")
        for err in errors:
            print(err)
        print()
    if warnings:
        print("WARNINGS — review before pushing:")
        for warn in warnings:
            print(warn)
        print()
    if not errors and not warnings:
        print("  ✅ Clean")

    return len(errors)


if __name__ == "__main__":
    sys.exit(main())
