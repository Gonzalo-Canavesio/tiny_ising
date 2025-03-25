import subprocess
from time import time

CC = ["gcc"]
CFLAGS = ["-std=c11", "-Wall", "-Wextra"]
EXTRAFLAGS = ["-lgomp", "-lm"]
FILES = ["ising.c", "tiny_ising.c"]

PARAMS_LIST = [
    {"line_size": 2, "reps": 100000},
    {"line_size": 4, "reps": 10000},
    {"line_size": 8, "reps": 10000},
    {"line_size": 16, "reps": 1000},
    {"line_size": 32, "reps": 1000},
    {"line_size": 64, "reps": 100},
    {"line_size": 128, "reps": 10},
    {"line_size": 217, "reps": 10},
    {"line_size": 256, "reps": 5},
    {"line_size": 384, "reps": 5},
    {"line_size": 512, "reps": 3},
    {"line_size": 1024, "reps": 1},
    {"line_size": 1031, "reps": 1},
    {"line_size": 2047, "reps": 1},
    {"line_size": 2048, "reps": 1},
    {"line_size": 4096, "reps": 1},
]


for params in PARAMS_LIST:
    cmd = CC + CFLAGS + FILES + EXTRAFLAGS + [f"-DL={params['line_size']}"]

    compilation_result = subprocess.run(cmd, stderr=subprocess.PIPE, text=True)
    if compilation_result.stderr:
        print(compilation_result.stderr)

    with open(f"results/{params['line_size']}___{round(time())}.txt", "x") as f:
        for _ in range(params["reps"]):
            execution_result = subprocess.run(
                ["./a.out"], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True
            )
            f.write(execution_result.stdout)
            f.write(execution_result.stderr)
