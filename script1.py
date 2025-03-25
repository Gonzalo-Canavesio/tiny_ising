import subprocess
import os
from time import time

CC = ["gcc-14"]
CFLAGS = ["-std=c11", "-Wall", "-Wextra"]
EXTRAFLAGS = ["-lomp", "-lm"]
FILES = ["ising.c", "tiny_ising.c"]

VERSIONS = ["v0", "v1", "v2"]

for v in VERSIONS:
    best_performance = 0.0

    os.chdir(v)

    xoshiro_file = ["xoshiro256plus.c"] if v == "v2" else []

    cmd = CC + CFLAGS + FILES + xoshiro_file + EXTRAFLAGS + ["-DL=512"]

    compilation_result = subprocess.run(cmd, stderr=subprocess.PIPE, text=True)
    if compilation_result.stderr:
        print(compilation_result.stderr)

    duration = 5 * 60
    start = time()
    while time() - start < duration:
        execution_result = subprocess.run(
            ["./a.out"], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True
        )
        for line in execution_result.stdout.splitlines():
            if line.startswith("# Spins/ms:"):
                best_performance = max(
                    best_performance, float(line.replace("# Spins/ms:", ""))
                )

    print(f"Version: {v}")
    print(f"Spins/ms: {best_performance}")

    os.chdir("..")
