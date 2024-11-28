#!/usr/bin/env python3

import argparse
import logging
import re
import subprocess
import os
import tempfile

LOGGER = logging.getLogger(__name__)

OBJDUMP = f'{os.environ["HOME"]}/.platformio/packages/toolchain-gccarmnoneeabi/bin/arm-none-eabi-objdump'
OBJCOPY = f'{os.environ["HOME"]}/.platformio/packages/toolchain-gccarmnoneeabi/bin/arm-none-eabi-objcopy'

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
    FIND_SYM_RE = re.compile(f'^([_\d\w]+{symbol_name})$', re.X)


    rostart = sym_addr = sym_size = None
    for line in r.stdout.splitlines():
        line = line.rstrip()
        if (m := SYM_RE.match(line)):
            addr, length, symbol = m[1], m[2], m[3]
            if symbol == '.rodata':
                rostart = int(addr, 16)
                LOGGER.debug("found rostart at 0x%x", rostart)
            elif m := FIND_SYM_RE.match(symbol):
                sym_addr = int(addr, 16)
                sym_size = int(length, 16)
                LOGGER.debug("found '%s' symbol at 0x%x, %d bytes long", symbol, sym_addr, sym_size)
                break
    
    if None in (rostart, sym_addr, sym_size):
        raise RuntimeError(f"Couldn't find .rodata in {r.stdout}")

    return rostart, sym_addr, sym_size
    

def get_nvm_blob(args) -> bytes:
    """Get NVM Blob.

    Find nvmSettingsStorage symbol in the .rodata segment, export the segment
    as a binary file, and read just the symbol blob.
    """
    rostart, symaddr, size = get_symbol_addr(args, symbol_name="nvmSettingsStorage")
    fp, tmpfile = tempfile.mkstemp(prefix="frm-rodata", suffix=".bin")

    # export .rodata segment
    r = subprocess.run(
        [OBJCOPY, '-O', 'binary', '-j', '.rodata', args.firmware, tmpfile],
        capture_output=True,
        text=True,
    )
    if r.returncode != 0:
        raise RuntimeError(f"arm-none-eabi-objcopy was not successful: {r.stderr}")
    
    # read the nvmSettingsStorage symbol
    with open(tmpfile, "rb") as fp:
        seek = symaddr - rostart
        LOGGER.debug("Seeking %s to 0x%x", tmpfile, seek)
        fp.seek(seek)
        nvm_blob = fp.read(size)
    return nvm_blob



def main(args):
    nvm = get_nvm_blob(args)
    try:
        with open(args.output_nvm_blob, "wb+") as fp:
            fp.write(nvm)
    except IOError as e:
        LOGGER.error("Couldn't opent %s for writing: %s", args.output_nvm_blob, e)
        raise e


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        prog = "extract-nvm.py",
        description = "Extract NVM blob and create a DFU file from it"
    )
    
    parser.add_argument(
        "--nvm-address",
        type=int,
        required=False,
        help="Specify where NVM blob is stored in flash memory",
        default=0x080FE000
    )
    parser.add_argument(
        "firmware",
        type=str,
        help="Specify firmware ELF",
    )
    parser.add_argument(
        "output_nvm_blob",
        type=str,
        help="Output binary blob for the NVM content",
    )

    args = parser.parse_args()
    main(args)
