import os
from pathlib import Path
import typing

### CONSTANTS ###
CLANG = "clang"


### Util Functions ###
def remove_redundant_log(log: str):
    logs = log.split('\n')
    key_logs = []
    splitted_logs = []
    tmp_logs = []
    for tmp_log in logs:
        if '---' in tmp_log:
            str_log = '\n'.join(tmp_logs)
            key_log = '\n'.join(tmp_logs[:2])
            if key_log not in key_logs:
                key_logs.append(key_log)
                splitted_logs.append(str_log)
            tmp_logs = []
        tmp_logs.append(tmp_log)

    str_log = '\n'.join(tmp_logs)
    key_log = '\n'.join(tmp_logs[:2])
    if key_log not in key_logs:
        key_logs.append(key_log)
        splitted_logs.append(str_log)

    return '\n'.join(splitted_logs)


def get_files(target: Path) -> typing.List[Path]:
    target_files = []

    if target.is_file():
        target_files.append(target)

    for dirpath, _, filenames in os.walk(target):
        target_files += [
            Path(dirpath) / Path(file) for file in filenames
            if Path(file).suffix == '.c'
        ]

    return target_files


def compilation_flags(additional_flags=[]):
    FLAGS = ['-g', '-fno-inline-functions', '-fno-builtin-bcmp']
    return FLAGS + additional_flags


def make_linux_build_command(target, job, compiler_flags=[], make_flags=[]):
    flags = " ".join(compilation_flags(compiler_flags))

    BASE_LINUX_MAKE_COMMAND = [
        'make',
        # '-k',
        '-C',
        target,
        f"-j{job}",
        # "KCFLAGS+=-g",
        # "KCFLAGS+=-fno-inline-functions", "KCFLAGS+=-fno-builtin-bcmp",
        f"KCFLAGS+={flags}",
        "LLVM_IAS=0",
        f'CC={CLANG}',
        f'HOSTCC={CLANG}'
    ]

    return BASE_LINUX_MAKE_COMMAND + make_flags
