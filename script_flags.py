import subprocess
from time import time

CC = ["gcc-14"]
CFLAGS = ["-std=c11", "-Wall", "-Wextra"]
EXTRAFLAGS = ["-qopenmp", "-lm"]
FILES = ["v2/ising.c", "v2/tiny_ising.c", "v2/xoshiro256plus.c"]

PARAMS_LIST = [256, 1024]
O_LIST = ["-O1", "-O2", "-O3", "-Ofast", "-Os"]

for line_size in PARAMS_LIST:
    for o in O_LIST:

            best_performance = 0.0

            cmd = CC + CFLAGS + FILES + EXTRAFLAGS + [f"{o}"] + [f"-DL={line_size}"]

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
            print(f"Optimization: {o}")
            print(f"Spins/ms: {best_performance}")

    best_performance = 0.0
