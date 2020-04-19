import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import sys

#Open up the parameter file, which
#is legal Python code
with open(sys.argv[1], 'r') as file:
    data = file.read()
#Execute the python code to get the variables
exec(data)


if npeople>5000:
    print("Warning: more than 5000 people of data generated, scatter plots may be very large and visually difficult to interpret")

if output_nodes:
    for i in range(0,max_print_nodes):
        print("Making image {}".format(i))
        xs=np.fromfile("x_{}.dat".format(i))
        ys=np.fromfile("y_{}.dat".format(i))
        inf=np.fromfile("inf_{}.dat".format(i),dtype=np.int32)
        print(np.sum(inf))
        colors=[]
        sizes=[]
        for d in range(0,len(inf)):
            if inf[d]>0:
                colors.append("red")
                sizes.append(50)
            else:
                colors.append("blue")
                sizes.append(1)

        plt.scatter(xs,ys,color=colors,s=sizes)
        plt.xlim((xleft,xright))
        plt.ylim((ybottom,ytop))
        plt.savefig("{}.svg".format(i))
        plt.close()
else:
    print("File outputs was not enabled for this run, set output_nodes=True and run again")
