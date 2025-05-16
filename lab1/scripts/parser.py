

best_time = 0

with open("results/512___1742847380.txt", "r") as f:
    for line in f:
        if line.startswith("# Spins/ms:"):
            best_time = max(float(line.replace("# Spins/ms:", "")),best_time)

print(best_time)