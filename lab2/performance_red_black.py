import subprocess
from time import time

CC = ["icx"]
CFLAGS = ["-Wall", "-Wextra"]
EXTRAFLAGS = ["-lgomp", "-lm"]
OFLAGS = ["-O2", "-march=native", "-ipo", "-flto", "-fvectorize", "-funroll-loops", "-ffast-math"]
FILES = [
    "red-black2/ising.c",
    "red-black2/tiny_ising.c",
    "red-black2/xoshiro256plus.c",
]

PARAMS_LIST = [256, 512, 1024, 2048, 4096]

for line_size in PARAMS_LIST:
    best_performance = 0.0

    cmd = CC + CFLAGS + FILES + EXTRAFLAGS + OFLAGS + [f"-DL={line_size}", "-o", "b.out"]

    compilation_result = subprocess.run(cmd, stderr=subprocess.PIPE, text=True)
    if compilation_result.stderr:
        print(compilation_result.stderr)

    duration = 3 * 60
    start = time()
    while time() - start < duration:
        execution_result = subprocess.run(
            ["./b.out"], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True
        )
        for line in execution_result.stdout.splitlines():
            if line.startswith("# Spins/ms:"):
                best_performance = max(
                    best_performance, float(line.replace("# Spins/ms:", ""))
                )

    print(f"L: {line_size}")
    print(f"Spins/ms: {best_performance}")
