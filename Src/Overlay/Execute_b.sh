###################################
##########  Test
###################################

./Test \
    /home/qiamking/Github/Trivial_Dummy/Test/grid_1_overlay.txt \
    /home/qiamking/Github/Trivial_Dummy/Test/grid_2_overlay.txt \
    Test_overlay.txt \
    Test_nonoverlay_M1.txt \
    Test_nonoverlay_M2.txt \
    Test_overlay_density.txt \
    Test_nonoverlay_density_M1.txt \
    Test_nonoverlay_density_M2.txt \
    | tee -a Test_overlay.log

mv \
    Test_overlay.txt \
    Test_nonoverlay_M1.txt \
    Test_nonoverlay_M2.txt \
    Test_overlay_density.txt \
    Test_nonoverlay_density_M1.txt \
    Test_nonoverlay_density_M2.txt \
    /home/qiamking/Github/Trivial_Dummy/Test/log