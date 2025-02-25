import matplotlib
matplotlib.use('TkAgg')
import matplotlib.pyplot as plt

min_ns = []
det_claculated = 0
times = []
with open("log.txt") as f:
    for line in f:
        if "det" in line:
            det_claculated = len(times)
        if "min_n" in line:
            min_n = int(line.split()[-1])
            min_ns.append(len(times))
        if "ts" not in line:
            continue
        time = int(line.split()[-1])
        times.append(time)

n = len(times)
x = list(range(n))

plt.figure(figsize=(16, 8))
plt.plot(x, times, label="load")
plt.vlines([det_claculated], [0], [max(times)], label="det calc")
n = len(min_ns)
plt.vlines(min_ns, [0] * n, [max(times)] * n, label="min_ns", colors=["#FF0000"] * n)
plt.legend()

plt.show()

print(min_ns)