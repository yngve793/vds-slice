import os
import xtgeo
import json

# Raw data can be found at:
# "https://onevdstest.blob.core.windows.net/testdata/Hilbert_attributes/Complex_Attributes_Top_Shetland.zip

folder_name = "raw"
files = os.listdir("raw")
data_row = 150

data_files = []
for f in files:
    if len(f) > 4 and f[-4] != "." and f.startswith("Extract value"):
        short_name = f
        data_files.append(short_name)

for f in data_files:
    print()
    short_name = f[15:-13]
    full_name = folder_name + "/" + f
    print(f"Load raw file: {full_name}")
    data = xtgeo.surface_from_file(full_name)

    hilbert_data = {}
    hilbert_data["name"] = short_name
    hilbert_data["shape"] = data.values.shape
    hilbert_data["values"] = data.values[data_row, :].tolist()

    json_dump = json.dumps({'hilbert_data': hilbert_data})

    print(f"Saving json file: {short_name}.json")
    with open(short_name + '.json', 'w') as fw:
        json.dump(json_dump, fw)
