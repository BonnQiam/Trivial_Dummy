###################################
##########  Test
###################################

./Test \
    /home/qiamking/Github/Trivial_Dummy/Output/Grid_Clean/s/sM2_grid.txt \
    /home/qiamking/Github/Trivial_Dummy/Output/Grid_Clean/s/sM3_grid.txt \
    sM23_overlay.txt \
    sM23_nonoverlay_M2.txt \
    sM23_nonoverlay_M3.txt \
    sM23_overlay_density.txt \
    sM23_nonoverlay_density_M2.txt \
    sM23_nonoverlay_density_M3.txt \
    | tee -a Test_overlay.log