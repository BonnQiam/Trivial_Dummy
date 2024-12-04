import numpy as np
import matplotlib.pyplot as plt
import sys
import getopt


# filename from command line: -file filename
filename = ''
try:
    opts, args = getopt.getopt(sys.argv[1:], "f:", ["file="])
except getopt.GetoptError:
    print('Grid_Test.py -f <filename>')
    sys.exit(2)
for opt, arg in opts:
    if opt in ("-f", "--file"):
        filename = arg

data = []
#with open('sM1.txt', 'r') as file:
with open(filename, 'r') as file:
    lines = file.readlines()
    for line in lines:
        split_line = line.split()
        # get the third column
        data.append(float(split_line[2]))
        
data = np.array(data).reshape(8, 20)       # for s
#data = np.array(data).reshape(117, 117)    # for b
# data = np.array(data).reshape(325, 53)     # for m

# plot data
plt.imshow(data, cmap='hot', interpolation='nearest')
plt.colorbar()
plt.savefig('Pre_fill.png')
plt.show()