import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import sys


f=np.fromfile("ninfections.dat",dtype=np.int32)
plt.plot(np.diff(f))
plt.title("New cases per time-step")
plt.xlabel("Simulation step")
plt.ylabel("New cases")
plt.savefig("cases.svg")
