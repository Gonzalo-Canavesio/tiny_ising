import subprocess
from time import time

CC = ["gcc-14"]
CFLAGS = ["-std=c11", "-Wall", "-Wextra"]
EXTRAFLAGS = ["-lgomp", "-lm"]
FILES_VERSIONS = ["v0/ising.c", "v0/tiny_ising.c"]


for files in FILES_VERSIONS:
    for line_size in [256, 384, 512, 1024]:
        best_performance = 0.0

        cmd = CC + CFLAGS + files + EXTRAFLAGS + [f"-DL={line_size}"]

        compilation_result = subprocess.run(
            cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True
        )
        if compilation_result.stderr:
            print(files)
            print(compilation_result.stdout)
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

        print(f"Version: {files[0]}")
        print(f"L: {line_size}")
        print(f"Spins/ms: {best_performance}")
