"""
This code is used to visualize the rectangles in the grid:
- input is the index of the grid
- output is the visualization of the rectangles in the grid
- the method reading file is read line by line rather than read all lines at once
"""


import re
import matplotlib.pyplot as plt
import matplotlib.patches as patches
import sys
import getopt

def WindowVisualize(index):
    """
    Read the file and plot the rectangles
    """
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
            
    rectangles = []
    x_min = 1e9
    x_max = 0
    y_min = 1e9
    y_max = 0
    
    count = 0
    
    FileHandler = open(filename, 'r')
    
    while True:
        line = FileHandler.readline()
        
        # Check if the line is empty
        if not line:
            break
        
        # Check the line == '<grid>'
        if line == '<grid>\n':
            count += 1
            if count == index:
                break
            
    if count == index:
        print("Now starting load rectangles in the " + str(index) + "-th grid")
        
        # skip the first line
        FileHandler.readline()
        #FileHandler.readline()
        #FileHandler.readline()
                
        while True:
            line = FileHandler.readline()
            
            # check line == '</grid>'
            if line == '</grid>\n' or line == '</grid>' or (not line):
                break
            
            # Use regular expression to extract number s
            Coors = re.findall(r'\d+', line)
            
            # Convert the numbers from string to integers
            Coors = [int(coor) for coor in Coors]
            
            # add into rectangels like rectangles = [(0.7, 0.3, 2.5, 4.1), (5.6, 2.2, 8.1, 5.1)]
            rectangles.append((Coors[0], Coors[1])) # Top left corner
            rectangles.append((Coors[2], Coors[3])) # Bottom right corner
            
            # update the x_min, x_max, y_min, y_max
            if Coors[0] < x_min:
                x_min = Coors[0]
            if Coors[1] > y_max:
                y_max = Coors[1]
            if Coors[2] > x_max:
                x_max = Coors[2]
            if Coors[3] < y_min:
                y_min = Coors[3]
        
        FileHandler.close()
        
        # plot the rectangles
        fig, ax = plt.subplots()
            
        # adjust the range of x/y axis        
        #plt.xlim(x_min, x_max)
        #plt.ylim(y_min, y_max)
        plt.xlim(20000*0, 20000*1)
        plt.ylim(20000*1, 20000*2)
        
        Area = 0
            
        for i in range(0, len(rectangles), 2):
            ax.add_patch(patches.Rectangle(rectangles[i], rectangles[i+1][0]-rectangles[i][0], rectangles[i+1][1]-rectangles[i][1], fill=True, edgecolor='blue'))
            
            Area += (rectangles[i+1][0]-rectangles[i][0]) * (rectangles[i+1][1]-rectangles[i][1])
        
        Density = Area / (20000*20000)
        
        print('The Density of the grid is: ' + str(Density))
              
        plt.show()
    else:

        FileHandler.close()
        print('Invalid index')
        
        
if __name__ == '__main__':
    WindowVisualize(1)