#!/usr/bin/env python3

import argparse
import re
import subprocess

OBJDUMP = '/home/lex/.platformio/packages/toolchain-gccarmnoneeabi/bin/arm-none-eabi-objdump'

def get_symbol_addr(args: dict, symbol_name="nvmSettingsStorage") -> tuple[int, int, int]:
    """Get .rodata section start, symbol address, and symbol size."""
    r = subprocess.run(
        [OBJDUMP, '-t', '-j', '.rodata', args.firmware],
        capture_output=True,
        text=True,
    )
    if r.returncode != 0:
        raise RuntimeError(f"arm-none-eabi-objdump was not successful: {r.stderr}")
    
#SYMBOL TABLE:
#08011300 l    d  .rodata        00000000 .rodata
#08011bc8 l     O .rodata        00000006 CSWTCH.94
#08011bc0 l     O .rodata        00000008 CSWTCH.95
#08011bbc l     O .rodata        00000004 CSWTCH.97
#08011bb8 l     O .rodata        00000004 CSWTCH.99
#08012428 l     O .rodata        00000064 _ZL18nvmSettingsStorage
#080133b4 l     O .rodata        00000014 fpi.1
    SYM_RE = re.compile(
        r'''^
        ([\da-fA-F]{8})  # Addr
        \s+[lgw]\s+\w\s+
        .rodata
        \s+
        ([\da-fA-F]{8})  # Length
        \s+
        ([\.\w\d]+)        # symbol name
        $
''',
        re.MULTILINE | re.X
    )
    FIND_SYM_RE = re.compile(f'^[_\d\w]+{symbol_name}$', re.X)


    rostart = sym_addr = sym_size = None
    for line in r.stdout.splitlines():
        line = line.rstrip()
        if (m := SYM_RE.match(line)):
            addr, length, symbol = m[1], m[2], m[3]
            if symbol == '.rodata':
                rostart = int(addr, 16)
            elif FIND_SYM_RE.match(symbol):
                sym_addr = int(addr, 16)
                sym_size = int(length, 16)
                break
    
    if None in (rostart, sym_addr, sym_size):
        raise RuntimeError(f"Couldn't find .rodata in {r.stdout}")

    return rostart, sym_addr, sym_size
    

def get_rostart_and_nvm_blob(args):
        rostart, symaddr, size = get_symbol_addr(args, symbol_name="nvmSettingsStorage")
        print(f"RO Start: {rostart}, nvm blob is at {symaddr} and is {size} bytes long\r\n")


def main(args):
    get_rostart_and_nvm_blob(args)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        prog = "extract-nvm.py",
        description = "Extract NVM blob and create a DFU file from it"
    )
    
    parser.add_argument(
        "-f",
        "--firmware",
        type=str,
        required=True,
        help="Specify firmware ELF",
    )
    parser.add_argument(
        "--nvm-address",
        type=int,
        required=False,
        help="Specify where NVM blob is stored in flash memory",
        default=0x080FE000
    )

    args = parser.parse_args()
    main(args)