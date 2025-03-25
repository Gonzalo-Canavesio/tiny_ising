import subprocess
from time import time

CC = ["clang"]
CFLAGS = ["-std=c11", "-Wall", "-Wextra"]
EXTRAFLAGS = ["-lgomp", "-lm"]
FILES = ["ising.c", "tiny_ising.c", "xoshiro256plus.c"]

PARAMS_LIST = [256, 512]
O_LIST = ["O1", "O2", "O3", "Ofast", "Os"]

for line_size in PARAMS_LIST:
    for o in O_LIST:

        for m in ["-march=native", ""]:
            best_performance = 0.0

            cmd = CC + CFLAGS + FILES + EXTRAFLAGS + [f"-{o}"] + [m] + [f"-DL={line_size}"]

            compilation_result = subprocess.run(cmd, stderr=subprocess.PIPE, text=True)
            if compilation_result.stderr:
                print(compilation_result.stderr)

            duration = 3 * 60
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
            print(f"Optimization: {o} {m}")
            print(f"Spins/ms: {best_performance}")
