python make_penobscot_expected.py
python make_penobscot_cube.py
SEGYImport --url "file://." --vdsfile penobscot_xl1155_x2.vds penobscot_xl1155_x2.sgy --crs-wkt="utmXX"
rm penobscot_xl1155_x2.sgy
