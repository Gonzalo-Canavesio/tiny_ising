import subprocess
from time import time
import os

CC = ["icx"]
CFLAGS = ["-Wall", "-Wextra"]
EXTRAFLAGS = ["-fopenmp", "-lm"]
OFLAGS = ["-O2", "-march=native", "-ipo", "-flto", "-fvectorize", "-funroll-loops", "-ffast-math"]
FILES = [
    "6_omp/ising.c",
    "6_omp/tiny_ising.c",
    "6_omp/xoshiro256plus.c",
]

PARAMS_LIST = [4096]

for line_size in PARAMS_LIST:
    best_performance = 0.0

    cmd = CC + CFLAGS + FILES + EXTRAFLAGS + OFLAGS + [f"-DL={line_size}"]

    compilation_result = subprocess.run(cmd, stderr=subprocess.PIPE, text=True)
    if compilation_result.stderr:
        print(compilation_result.stderr)

    duration = 2 * 60
    start = time()
    while time() - start < duration:
        execution_result = subprocess.run(
            ["./a.out"], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, 
            env={**os.environ, "OMP_NUM_THREADS": "20"}
        )
        
        for line in execution_result.stdout.splitlines():
            if line.startswith("# Spins/ms:"):
                best_performance = max(
                    best_performance, float(line.replace("# Spins/ms:", ""))
                )

    print(f"L: {line_size}")
    print(f"Spins/ms: {best_performance}")
