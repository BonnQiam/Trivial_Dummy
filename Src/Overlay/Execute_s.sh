###################################
##########  Test
###################################

./Test \
    /home/qiamking/Github/Trivial_Dummy/Output/Grid_Clean/s/sM1_grid.txt \
    /home/qiamking/Github/Trivial_Dummy/Output/Grid_Clean/s/sM2_grid.txt \
    sM12_overlay.txt \
    sM12_nonoverlay_M1.txt \
    sM12_nonoverlay_M2.txt \
    sM12_overlay_density.txt \
    sM12_nonoverlay_density_M1.txt \
    sM12_nonoverlay_density_M2.txt \
    | tee -a sM12_overlay.log