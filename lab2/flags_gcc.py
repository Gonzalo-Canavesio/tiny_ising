import subprocess
from time import time

CC = ["gcc-14"]
CFLAGS = ["-Wall", "-Wextra"]
EXTRAFLAGS = ["-lgomp", "-lm"]
FILES = [
    "red-black2/ising.c",
    "red-black2/tiny_ising.c",
    "red-black2/xoshiro256plus.c",
]

PARAMS_LIST = [512, 2048]
O_LIST = [
    "-Ofast -march=native -flto -ftree-vectorize -funroll-loops -ffast-math",
]

for line_size in PARAMS_LIST:
    for o in O_LIST:
        best_performance = 0.0

        cmd = CC + CFLAGS + FILES + EXTRAFLAGS + o.split() + [f"-DL={line_size}", "-o", "c.out"]

        compilation_result = subprocess.run(cmd, stderr=subprocess.PIPE, text=True)
        if compilation_result.stderr:
            print(compilation_result.stderr)

        duration = 3 * 60
        start = time()
        while time() - start < duration:
            execution_result = subprocess.run(
                ["./c.out"], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True
            )
            for line in execution_result.stdout.splitlines():
                if line.startswith("# Spins/ms:"):
                    best_performance = max(
                        best_performance, float(line.replace("# Spins/ms:", ""))
                    )

        print(f"L: {line_size}")
        print(f"Optimization: {o}")
        print(f"Spins/ms: {best_performance}")

    best_performance = 0.0
