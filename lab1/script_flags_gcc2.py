import subprocess
from time import time

CC = ["gcc-14"]
CFLAGS = ["-std=c11", "-Wall", "-Wextra"]
EXTRAFLAGS = ["-lgomp", "-lm"]
FILES = ["v2/ising.c", "v2/tiny_ising.c", "v2/xoshiro256plus.c"]

PARAMS_LIST = [256, 1024]
O_LIST = [
    "-O2",
    "-O3",
    "-Ofast",
    "-O2 -march=native",
    "-O3 -march=native",
    "-Ofast -march=native",
    "-O2 -funroll-loops",
    "-O3 -funroll-loops",
    "-Ofast -funroll-loops",
    "-O2 -march=native -funroll-loops",
    "-O3 -march=native -funroll-loops",
    "-Ofast -march=native -funroll-loops",
    "-O2 -ffast-math",
    "-O3 -ffast-math",
    "-Ofast -ffast-math",
    "-O2 -march=native -ffast-math",
    "-O3 -march=native -ffast-math",
    "-Ofast -march=native -ffast-math",
    "-O2 -funroll-loops -ffast-math",
    "-O3 -funroll-loops -ffast-math",
    "-Ofast -funroll-loops -ffast-math",
    "-O2 -march=native -funroll-loops -ffast-math",
    "-O3 -march=native -funroll-loops -ffast-math",
    "-Ofast -march=native -funroll-loops -ffast-math",
    "-O2 -flto",
    "-O3 -flto",
    "-Ofast -flto",
    "-O2 -march=native -flto",
    "-O3 -march=native -flto",
    "-Ofast -march=native -flto",
    "-O2 -funroll-loops -flto",
    "-O3 -funroll-loops -flto",
    "-Ofast -funroll-loops -flto",
    "-O2 -march=native -funroll-loops -flto",
    "-O3 -march=native -funroll-loops -flto",
    "-Ofast -march=native -funroll-loops -flto",
    "-O2 -ffast-math -flto",
    "-O3 -ffast-math -flto",
    "-Ofast -ffast-math -flto",
    "-O2 -march=native -ffast-math -flto",
    "-O3 -march=native -ffast-math -flto",
    "-Ofast -march=native -ffast-math -flto",
    "-O2 -funroll-loops -ffast-math -flto",
    "-O3 -funroll-loops -ffast-math -flto",
    "-Ofast -funroll-loops -ffast-math -flto",
    "-O2 -march=native -funroll-loops -ffast-math -flto",
    "-O3 -march=native -funroll-loops -ffast-math -flto",
    "-Ofast -march=native -funroll-loops -ffast-math -flto",
]

for line_size in PARAMS_LIST:
    for o in O_LIST:
        best_performance = 0.0

        cmd = CC + CFLAGS + FILES + EXTRAFLAGS + o.split() + [f"-DL={line_size}"]

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
