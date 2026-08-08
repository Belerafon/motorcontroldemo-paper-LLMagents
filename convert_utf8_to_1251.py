#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
convert_to_win1251_standalone.py

Recursively converts *.c, *.cpp, *.h from UTF-8 to Windows-1251.
Preserves CR/LF sequences, file permissions and timestamps.
Standalone version without external dependencies.
"""

import os
import sys
import argparse
import re
import traceback

SRC_ENCODING = 'utf-8'
DST_ENCODING = 'windows-1251'
EXTENSIONS = {'.c', '.cpp', '.h'}

def detect_encoding_simple(raw_bytes: bytes) -> str:
    """Simple encoding detection without external dependencies."""
    # Check for BOM
    if raw_bytes.startswith(b'\xef\xbb\xbf'):
        return 'utf-8-sig'
    
    # Try to decode as UTF-8
    try:
        raw_bytes.decode('utf-8')
        return 'utf-8'
    except UnicodeDecodeError:
        pass
    
    # Try to decode as Windows-1251
    try:
        raw_bytes.decode('windows-1251')
        return 'windows-1251'
    except UnicodeDecodeError:
        pass
    
    # Check if it's pure ASCII
    if all(b < 128 for b in raw_bytes):
        return 'ascii'
    
    return 'unknown'

def needs_conversion(raw: bytes) -> bool:
    """True if file needs conversion from UTF-8 to Windows-1251."""
    encoding = detect_encoding_simple(raw)
    return encoding in ('utf-8', 'utf-8-sig')

def convert_file(path: str):
    """Convert a single file from UTF-8 to Windows-1251."""
    try:
        with open(path, 'rb') as f:
            raw = f.read()

        if not needs_conversion(raw):
            return  # Nothing to do

        st = os.stat(path)  # Remember metadata

        # Remove BOM if present
        if raw.startswith(b'\xef\xbb\xbf'):
            raw = raw[3:]

        # Split into parts to preserve original line endings
        parts = re.split(b'(\r\n|\r|\n)', raw)
        out = bytearray()

        for chunk in parts:
            if chunk in (b'\r\n', b'\r', b'\n'):
                out += chunk  # Line ending unchanged
            else:
                try:
                    text = chunk.decode(SRC_ENCODING)
                    encoded_chunk = text.encode(DST_ENCODING)
                    out += encoded_chunk
                except UnicodeEncodeError as e:
                    print(f"Warning: Cannot encode some characters in {path}: {e}")
                    # Try to encode with error handling
                    text = chunk.decode(SRC_ENCODING, errors='replace')
                    encoded_chunk = text.encode(DST_ENCODING, errors='replace')
                    out += encoded_chunk

        tmp = path + '.tmp'
        with open(tmp, 'wb') as f:
            f.write(out)

        os.replace(tmp, path)  # Atomic replacement
        os.chmod(path, st.st_mode)
        os.utime(path, (st.st_atime, st.st_mtime))
        print(f"<- Windows-1251: {path}")

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

    print(f"\nConversion complete. {converted_count} files were converted to Windows-1251.")

if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Convert *.c, *.cpp, *.h files from UTF-8 to Windows-1251',
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