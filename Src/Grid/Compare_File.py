#file1 = '/home/qiamking/Github/Trivial_Dummy/Output/Overlay_Area/s/sM23_overlay.txt'
#file2 = '/home/qiamking/Github/Trivial_Dummy/Output/Overlay_Locate/s/sM23_density.txt'

file1 = '/home/qiamking/Github/Trivial_Dummy/Output/Overlay_Area/b/bM23_overlay.txt'
file2 = '/home/qiamking/Github/Trivial_Dummy/Output/Overlay_Locate/b/bM23_density.txt'

#file1 = '/home/qiamking/Github/Trivial_Dummy/Output/Overlay_Area/m/mM12_overlay.txt'
#file2 = '/home/qiamking/Github/Trivial_Dummy/Output/Overlay_Locate/m/mM12_density.txt'

file3 = 'Diff.txt' # output file as the difference of file1 and file2

with open(file1, 'r') as f1, open(file2, 'r') as f2:
    lines1 = f1.readlines()
    lines2 = f2.readlines()

with open(file3, 'w') as f3:
    count = 0
    for line1, line2 in zip(lines1, lines2):
        count += 1
        if line1 != line2:
            diff = float(line2) - float(line1)
            # output the diff into file3, keep the first 6 digits
            f3.write(f'{count}: {diff:.6f}\n')
            ## f3.write(f'{diff:.6f}\n')