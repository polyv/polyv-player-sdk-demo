#! /usr/bin/env python3
# -*- coding: UTF-8 -*-

import argparse
import re
import sys
from pathlib import Path
import subprocess
import shlex
import difflib
from typing import Union, List


def file_diff(src: str, dst: str,
              src_file: Union[Path, str], dst_file: Union[Path, str],
              **kw) -> List[str]:
    if isinstance(src_file, Path):
        src_file = str(src_file)
    if isinstance(dst_file, Path):
        dst_file = str(dst_file)
    # diff = difflib.ndiff(ori_rc_src.splitlines(keepends=True),
    #                      rc_src.splitlines(keepends=True))
    diff = difflib.unified_diff(src.splitlines(keepends=True),
                                dst.splitlines(keepends=True),
                                src_file, dst_file)
    diff_list = list(diff)
    if len(diff_list) == 0:
        return diff_list
    print('diff'.center(40, '-'))
    print(''.join(diff_list))
    print('~diff~'.center(40, '-'))
    return diff_list


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('--rc-file', dest='rc_file', required=True)
    parser.add_argument('--encoding', dest='encoding', required=True)
    parser.add_argument('--app-ver-major', dest='app_ver_major', required=True)
    parser.add_argument('--app-ver-minor', dest='app_ver_minor', required=True)
    parser.add_argument('--app-ver-patch', dest='app_ver_patch', required=True)

    args = parser.parse_args()
    print(Path(__file__).name, args)

    def remove_path_quote(path: str) -> str:
        pa = re.compile(r'^"(.*)"$')
        if pa.findall(path):
            return pa.findall(path)[0]
        else:
            return path

    rc_file = Path(remove_path_quote(args.rc_file))
    if not rc_file.exists():
        raise IOError(f'input file({rc_file}) does not exists.')

    with open(rc_file, 'r', encoding=args.encoding) as f:
        src = f.read()

    commit_hash = subprocess.check_output(
        'git rev-parse --short=8 HEAD').decode().strip()
    dst = src

    find_group = re.compile(
        r'(^.*#define VERSION_STR[ \t]+"(\d+\.\d+\.\d+\.\w+).*$)',
        re.M).findall(dst)[0]
    sub_res = find_group[0].replace(
        find_group[1],
        f'{args.app_ver_major}.{args.app_ver_minor}.{args.app_ver_patch}.{commit_hash}')
    dst = dst.replace(find_group[0], sub_res)

    file_diff(src, dst, rc_file, rc_file)

    with open(rc_file, 'w', encoding=args.encoding) as f:
        f.write(dst)
