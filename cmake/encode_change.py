#! /usr/bin/env python3
# -*- coding: UTF-8 -*-

import argparse
import re
import sys
from pathlib import Path

# sys.argv += ['--input', '"/generate/resource-utf-8.rc"']
# sys.argv += ['--output', '"/generate/resource.rc"']
# sys.argv += ['--input-encoding', 'utf-8']
# sys.argv += ['--output-encoding', 'utf-16']

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('--input', dest='input', required=True)
    parser.add_argument('--output', dest='output', required=True)
    parser.add_argument('--input-encoding',
                        dest='input_encoding', required=True)
    parser.add_argument('--output-encoding',
                        dest='output_encoding', default=None)

    args = parser.parse_args()
    print(Path(__file__).name, args)

    def remove_path_quote(path: str) -> str:
        pa = re.compile(r'^"(.*)"$')
        if pa.findall(path):
            return pa.findall(path)[0]
        else:
            return path

    input_file = Path(remove_path_quote(args.input))
    if not input_file.exists():
        raise IOError(f'input file({input_file}) does not exists.')
    output_file = Path(remove_path_quote(args.output))

    with open(input_file, 'r', encoding=args.input_encoding) as f:
        src = f.read()

    with open(output_file, 'w', encoding=args.output_encoding) as f:
        f.write(src)
