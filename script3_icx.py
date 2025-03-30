import subprocess
from time import time

CC = ["icx"]
CFLAGS = ["-std=c11", "-Wall", "-Wextra"]
EXTRAFLAGS = ["-lgomp", "-lm"]
FILES = ["v3/ising.c", "v3/tiny_ising.c", "v3/xoshiro256plus.c"]

PARAMS_LIST = [256, 384, 512, 1024, 2048, 4096]
OPTIMIZATIONS = ["-Ofast", "-march=native", "-ipo"]

for line_size in PARAMS_LIST:
    best_performance = 0.0

    cmd = CC + CFLAGS + FILES + EXTRAFLAGS + OPTIMIZATIONS + [f"-DL={line_size}"]

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

    print(f"L: {line_size}")
    print(f"Spins/ms: {best_performance}")
