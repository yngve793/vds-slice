ACCOUNT_NAME=
SAS=

python make_intersecting_cubes.py
VDSCopy "regular_8x3_cube.vds" "azureSAS://$ACCOUNT_NAME.blob.core.windows.net/testdata/cube_intersection/regular_8x3_cube" --compression-method=None -d "Suffix=?$SAS"
VDSCopy "shift_4_8x3_cube.vds" "azureSAS://$ACCOUNT_NAME.blob.core.windows.net/testdata/cube_intersection/shift_4_8x3_cube" --compression-method=None -d "Suffix=?$SAS"
VDSCopy "big_shift_8_32x3_cube.vds" "azureSAS://$ACCOUNT_NAME.blob.core.windows.net/testdata/cube_intersection/big_shift_8_32x3_cube" --compression-method=None -d "Suffix=?$SAS"
