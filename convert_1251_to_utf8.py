#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
convert_to_utf8_standalone.py

Recursively converts *.c, *.cpp, *.h from CP1251 to UTF-8.
Preserves CR/LF sequences, file permissions and timestamps.
Standalone version without external dependencies.
"""

import os
import sys
import argparse
import re
import traceback

SRC_ENCODING = 'windows-1251'
DST_ENCODING = 'utf-8'
EXTENSIONS = {'.c', '.cpp', '.h', '.hpp', '.asm', '.cmd', '.ld'}

def needs_conversion(raw: bytes) -> bool:
    """True if file needs conversion from CP1251 to UTF-8."""
    # 1) Already UTF-8?
    try:
        raw.decode(DST_ENCODING)
        return False
    except UnicodeDecodeError:
        pass

    # 2) Only ASCII? Such files are already valid UTF-8.
    if all(b < 128 for b in raw):
        return False

    # 3) Successfully decodes as CP1251?
    try:
        raw.decode(SRC_ENCODING)
        return True
    except UnicodeDecodeError:
        return False

def convert_file(path: str):
    """Convert a single file from CP1251 to UTF-8."""
    try:
        with open(path, 'rb') as f:
            raw = f.read()

        if not needs_conversion(raw):
            return  # Nothing to do

        st = os.stat(path)  # Remember metadata

        # Split into parts to preserve original line endings
        parts = re.split(b'(\r\n|\r|\n)', raw)
        out = bytearray()

        for chunk in parts:
            if chunk in (b'\r\n', b'\r', b'\n'):
                out += chunk  # Line ending unchanged
            else:
                text = chunk.decode(SRC_ENCODING)
                out += text.encode(DST_ENCODING)

        tmp = path + '.tmp'
        with open(tmp, 'wb') as f:
            f.write(out)

        os.replace(tmp, path)  # Atomic replacement
        os.chmod(path, st.st_mode)
        os.utime(path, (st.st_atime, st.st_mtime))
        print(f"-> UTF-8: {path}")

    except Exception as e:
        print(f"Error converting {path}: {e}")

def main(root: str):
    """Main conversion function."""
    if not os.path.exists(root):
        print(f"Error: Directory '{root}' does not exist.")
        return

    converted_count = 0
    for dirpath, _, files in os.walk(root):
        for name in files:
            if os.path.splitext(name)[1].lower() in EXTENSIONS:
                full_path = os.path.join(dirpath, name)
                old_size = os.path.getsize(full_path) if os.path.exists(full_path) else 0
                convert_file(full_path)
                new_size = os.path.getsize(full_path) if os.path.exists(full_path) else 0
                if new_size != old_size:
                    converted_count += 1

    print(f"\nConversion complete. {converted_count} files were converted to UTF-8.")

if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Convert *.c, *.cpp, *.h files from CP1251 to UTF-8',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  %(prog)s                    # Convert current directory
  %(prog)s C:\\MyProject       # Convert specific directory
        """
    )
    parser.add_argument('directory', nargs='?', default='.',
                        help='Project root directory (default: current directory)')
    parser.add_argument('--version', action='version', version='%(prog)s 1.0')
    
    args = parser.parse_args()
    
    try:
        print(f"Converting files in: {os.path.abspath(args.directory)}")
        print(f"Target extensions: {', '.join(sorted(EXTENSIONS))}")
        print(f"Encoding: {SRC_ENCODING} -> {DST_ENCODING}")
        print("-" * 50)
        main(args.directory)
    except KeyboardInterrupt:
        print("\nOperation cancelled by user.")
    except Exception:
        print("\nUnexpected error occurred:")
        traceback.print_exc()
    finally:
        input("\nPress Enter to exit...")