import matplotlib.pyplot as plt

det_claculated = 0
times = []
with open("log.txt") as f:
    for line in f:
        if "det" in line:
            det_claculated = len(times)
        if "ts" not in line:
            continue
        time = int(line.split()[-1])
        times.append(time)

n = len(times)
x = list(range(n))

width = 1000

plt.figure(figsize=(16, 8))
plt.plot(x, times, label="load")
plt.plot([det_claculated + i for i in range(width)], [max(times)] * width, label="det calc")
plt.legend()

plt.show()