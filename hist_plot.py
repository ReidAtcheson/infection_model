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


if output_nodes:
    for i in range(0,max_print_nodes):
        print("Plotting histogram {}".format(i))
        xs=np.fromfile("inf_x_{}.dat".format(i))
        ys=np.fromfile("inf_y_{}.dat".format(i))
        nbins=20
        xbins=np.linspace(xleft,xright,nbins)
        ybins=np.linspace(ybottom,ytop,nbins)
        H, xedges, yedges = np.histogram2d(xs, ys,bins=(xbins,ybins),normed=True)
        plt.imshow(H,interpolation="bicubic")
        plt.savefig("{}.svg".format(i))
        plt.close()
else:
    print("File outputs was not enabled for this run, set output_nodes=True and run again")
