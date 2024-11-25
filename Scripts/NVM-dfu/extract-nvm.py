#!/usr/bin/env python3

import argparse
import re
import subprocess

OBJDUMP = '/home/lex/.platformio/packages/toolchain-gccarmnoneeabi/bin/arm-none-eabi-objdump'

def get_rostart_and_nvm_blob(args: dict) -> tuple[int, int, int]:
    """Get .rodata section start, nvm blob address, and nvm blob size."""
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
    RODATA_RE = re.compile(
        r'''^([\da-fA-F]{8})
        \s\w\s+\w\s+
        .rodata
        \s
        00000000
        \s
        .rodata$
''',
        re.MULTILINE | re.X
    )

    rostart = nvm_start = nvm_size = None
    m = RODATA_RE.match(r.stdout)
    if m:
        rostart = int(m[1])
    else:
        raise RuntimeError(f"Couldn't find .rodata in {r.stdout}")
    
    print(rostart)


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
