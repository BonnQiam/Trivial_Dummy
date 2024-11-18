import re
import matplotlib.pyplot as plt
import matplotlib.patches as patches
import sys
import getopt

def Test_1():
    """
    Test the basic grid generation and visualization with rectangles
    Test the intersection area calculation between rectangles
    """
    # Define the grid size
    GRID_SIZE = 10

    # Define a function to calculate the area of intersection between two rectangles
    # rect1 = (x1, y1, x2, y2) and rect2 = (x1, y1, x2, y2)
    def intersection_area(rect1, rect2):
        x_overlap = max(0, min(rect1[2], rect2[2]) - max(rect1[0], rect2[0]))
        y_overlap = max(0, min(rect1[3], rect2[3]) - max(rect1[1], rect2[1]))
        
        return x_overlap * y_overlap

    # Define the list of rectangle coordinates (x1, y1, x2, y2), x1, y1 are the coordinates of the bottom-left corner, and x2, y2 are the coordinates of the top-right corner
    # Replace this with your actual list of rectangles
    rectangles = [(0.7, 0.3, 2.5, 4.1), (5.6, 2.2, 8.1, 5.1)]

    # Initialize the grid with zeros
    grid = [[0] * GRID_SIZE for _ in range(GRID_SIZE)]

    # Calculate the area of each grid cell
    for i in range(GRID_SIZE):
        for j in range(GRID_SIZE):
            grid_cell_area = 0
            for rect in rectangles:
                # Calculate the area of intersection
                intersect_area = intersection_area(rect, (i, j, i+1, j+1))
                grid_cell_area += intersect_area
            grid[i][j] = grid_cell_area  # Assign the area to the corresponding grid cell
            
    # Plot the rectangles
    fig, ax = plt.subplots()
    for rect in rectangles:
        ax.add_patch(patches.Rectangle((rect[0], rect[1]), rect[2]-rect[0], rect[3]-rect[1], fill=False, edgecolor='blue'))

    # Plot the grid
    for i in range(GRID_SIZE + 1):
        ax.axhline(y=i, color='gray', linestyle='--', linewidth=0.5)
        ax.axvline(x=i, color='gray', linestyle='--', linewidth=0.5)

    # Plot the grid and annotate each grid cell with its area
    for i in range(GRID_SIZE):
        for j in range(GRID_SIZE):
            ax.add_patch(patches.Rectangle((i, j), 1, 1, fill=True, edgecolor='none', facecolor='none'))
            
            #print grid[i][j]
            #if grid[i][j] > 0:
            #    ax.add_patch(patches.Rectangle((i, j), 1, 1, fill=True, edgecolor='none', facecolor='red', alpha=grid[i][j]))
            
            ax.text(i + 0.5, j + 0.5, f'{grid[i][j]:.2f}', ha='center', va='center', color='red')

    plt.xlim(0, GRID_SIZE)
    plt.ylim(0, GRID_SIZE)
    plt.gca().set_aspect('equal', adjustable='box')
    plt.show()

def Test_2():
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
    
    with open(filename, 'r') as f:
        lines = f.readlines()
        # Remove the first two lines and the final line
        lines = lines[2:-1]
        for line in lines:
            # Use regular expression to extract number s
            Coors = re.findall(r'\d+', line)
            
            # Convert the numbers from string to integers
            Coors = [int(coor) for coor in Coors]
            
            # add into rectangels like rectangles = [(0.7, 0.3, 2.5, 4.1), (5.6, 2.2, 8.1, 5.1)]
            rectangles.append((Coors[0], Coors[1])) # Top left corner
            rectangles.append((Coors[2], Coors[3])) # Bottom right corner

    # plot the rectangles
    fig, ax = plt.subplots()
    
    # adjust the range of x/y axis
    plt.xlim(0, 20000)
    plt.ylim(0, 20000)
    
    for i in range(0, len(rectangles), 2):
        ax.add_patch(patches.Rectangle(rectangles[i], rectangles[i+1][0]-rectangles[i][0], rectangles[i+1][1]-rectangles[i][1], fill=True, edgecolor='blue'))
    plt.show()
    
if __name__ == '__main__':
#    Test_1()
    Test_2()