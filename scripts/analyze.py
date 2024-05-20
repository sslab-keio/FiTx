import datetime
import multiprocessing
import os
import subprocess
import time
from pathlib import Path

import click

from FiTx import utils
from FiTx.constants import FITX_ROOT, LINUX_ROOT, LOG_DIR, BUILD_DIR, DETECTOR_PATH

### CONSTANT VARIABLES ###
FITX_ROOT     = os.environ.get("FITX_ROOT", "/FiTx")
LINUX_ROOT    = os.environ.get("LINUX_ROOT", "/linux")
LOG_DIR       = os.path.abspath("/tmp/log")
BUILD_DIR     = os.path.join(FITX_ROOT, 'build')
DETECTOR_PATH = os.path.join(BUILD_DIR, 'detector', 'all_detector',
                             'libAllDetectorMod.so')

@click.group()
def commands():
    pass


@commands.command()
@click.option("--target", "-t", default=LINUX_ROOT)
@click.option("--file", "-f", default=None)
@click.option("--measure", "-m", is_flag=True)
def linux(target, file, measure):
    print(f"Start running analyzer")
    tmplog = os.path.join(LOG_DIR, "tmplog")
    current = datetime.datetime.now().strftime('%Y_%m_%d_%H:%M')
    log = os.path.join(LOG_DIR, f"{current}.log")
    with open(tmplog, 'w+') as f:
        compiler_flags = ["-Xclang", "-load", "-Xclang", DETECTOR_PATH]
        make_flags = []

        if measure:
            compiler_flags += ["-mllvm", "-measure"]

        if file:
            target_file = os.path.join(target, file)
            if os.path.exists(target_file):
                subprocess.run(['rm', target_file])
            make_flags.append(file)

        command = utils.make_linux_build_command(
                target, multiprocessing.cpu_count(), compiler_flags, make_flags)

        # Start analysis
        start = time.time()
        result = subprocess.run(command, stderr=subprocess.PIPE)
        end = time.time()
        # Finish Analysis

        f.write(result.stderr.decode('utf-8'))

    logfiles = [tmplog] + utils.get_log_files(Path(target))

    log_output = ''
    measure_output = ''
    for logfile in logfiles:
        result = subprocess.run(['awk', "$0 ~/ERROR/ || $0 ~/LOG/", logfile],
                                stdout=subprocess.PIPE)
        log_output += utils.remove_redundant_log(result.stdout.decode('utf-8'))

        measure_result = subprocess.run(['awk', "$0 ~/Elapsed/", logfile], stdout=subprocess.PIPE)
        measure_output += measure_result.stdout.decode('utf-8')
    with open(log, 'w+') as f:
        f.write(log_output)


    if measure:
        measure_log = os.path.join(LOG_DIR, f"{current}_time.log")
        with open(measure_log, 'w+') as f:
        #     subprocess.run(['awk', "$0 ~/Elapsed/", tmplog], stdout=f)
            # f.write(result.stdout.decode('utf-8'))
            f.write(measure_output)
        print(f"Logged time measure to {measure_log}")


    print(f"Done running command (Runtime - {end - start} sec): {command}")
    print(f"Logged to file {log}")


@commands.command()
@click.argument("target", type=click.Path(exists=True))
def test(target):
    print(f"Running test on {target}")
    target_files = utils.get_files(Path(target))
    additional_flags = ["-Xclang", "-load", "-Xclang", DETECTOR_PATH]

    print(f"Found {len(target_files)} tests")
    for target in target_files:
        print(f"[Running] {target}\r", end="")
        compile_command = ["clang", target, "-o", "/dev/null"
                           ] + utils.compilation_flags(additional_flags)
        result = subprocess.run(compile_command, stderr=subprocess.PIPE)
        logs = utils.remove_redundant_log(result.stderr.decode('utf-8'))

        if logs:
            # Add newline so that we do not override the "Running" tag
            print("")
            print(logs.strip())


if __name__ == "__main__":
    commands()
