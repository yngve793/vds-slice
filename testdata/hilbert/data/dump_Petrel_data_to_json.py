import os
import xtgeo
import json
import codecs
import openvds
import numpy as np
import math
# from oneseismic import OneseismicClient
import oneseismic
import matplotlib.pyplot as plt


def load_Volve_data(url, connection):

    vds = openvds.open(url, connection)
    layout = openvds.getLayout(vds)

    accessManager = openvds.VolumeDataAccessManager(vds)
    axisDescriptors = [layout.getAxisDescriptor(
        dim) for dim in range(layout.getDimensionality())]
    sliceType = 'timeslice'
    sliceIndex = 0
    sliceDimension = 2 if sliceType == 'inline' else 1 if sliceType == 'crossline' else 0 if sliceType == 'timeslice' else 0 if sliceType == 'depthslice' else -1

    min_dim = tuple(sliceIndex + 0 if dim ==
                    sliceDimension else 0 for dim in range(6))
    max_dim = tuple(sliceIndex + 1 if dim ==
                    sliceDimension else layout.getDimensionNumSamples(dim) for dim in range(6))

    req = accessManager.requestVolumeSubset(
        min_dim, max_dim, format=openvds.VolumeDataChannelDescriptor.Format.Format_R32)
    height = max_dim[0] if sliceDimension != 0 else max_dim[1]
    width = max_dim[2] if sliceDimension != 2 else max_dim[1]

    data = req.data.reshape(width, height)

    return data

    # hilbert_data = {}
    # hilbert_data["name"] = file_name
    # hilbert_data["shape"] = data.shape
    # hilbert_data["values"] = data[data_row, :].tolist()

    # json_dump = json.dumps({'hilbert_data': hilbert_data})

    # print(f"Saving json file: {file_name}.json")
    # with open(file_name + '.json', 'w') as fw:
    #     json.dump(json_dump, fw)

def _send_attributes_along_surface_request(self, data):
    response = requests.post(f'{self.host}/attributes/surface/along',
        headers = {'Content-Type': 'application/json'},
        data = data
    )
    if not response.ok: raise RuntimeError(response.text)
    parts = MultipartDecoder.from_response(response).parts
    
    meta = json.loads(parts[0].content)
    
    params = json.loads(data)
    return { 
        name: np.ndarray(meta['shape'], meta['format'], part.content)
        for name, part 
        in zip(params['attributes'], parts[1:])
    }

def attributes_along_surface(
    surface,
    above,
    below,
    stepsize,
    attributes,
    interpolation = 'linear'
):
    payload = json.dumps({
        'vds'           : self.vds,
        'sas'           : self.sas,
        'surface'       : surface.to_dict(),
        'above'         : above,
        'below'         : below,
        'stepsize'      : stepsize,
        'attributes'    : attributes,
        'interpolation' : interpolation
    })

    return self._send_attributes_along_surface_request(payload)

class RegularSurface:
    def __init__(
        self,
        values,
        xori,
        yori,
        xinc,
        yinc,
        rot,
        fillvalue = 999.55
    ):
        self.values = values
        self.xori = xori
        self.yori = yori
        self.xinc = xinc
        self.yinc = yinc
        self.rot = rot
        self.fillvalue = fillvalue

    def to_dict(self):
        return {
            'values'    : self.values.tolist(),
            'xori'      : self.xori,
            'yori'      : self.yori,
            'xinc'      : self.xinc,
            'yinc'      : self.yinc,
            'rotation'  : self.rot,
            'fillValue' : self.fillvalue,
        }



def load_Top_Shetland_data(raw_data_folder):

    surface = xtgeo.surface_from_file(f'{raw_data_folder}/TWT')

    print("Surface", surface.values.shape)

    heights = -1 * np.where(
        surface.values.mask == False, 
        surface.values.data, 
        fillvalue
        )

    print("Heights", heights.shape)

    return heights, surface
    

    # attributes = attributes_along_surface(
    # RegularSurface(
    #     heights,
    #     surface.xori,
    #     surface.yori,
    #     surface.xinc,
    #     surface.yinc * surface.yflip,
    #     surface.rotation,
    #     fillvalue = fillvalue
    # ),
    # above = 20,
    # below = 20,
    # stepsize = 0.1,
    # attributes = ['samplevalue', 'mean', 'min', 'max', 'rms', 'sd'],
    # )




def get_raw_file_info(raw_data_folder):

    files = os.listdir(raw_data_folder)

    data_files = []
    for f in files:
        if len(f) > 4 and f[-4] != "." and f.startswith("Extract value"):
            
            short_name = f[15:-13]
            parts = short_name.split(" ")
            attribute_type = parts[1][1:-1]
            prefix_name = parts[0]

            file_info = {}
            file_info["full_path"] = raw_data_folder + "/" + f
            file_info["prefix_name"] = prefix_name
            file_info["attribute_type"] = attribute_type
            data_files.append(file_info)
    return data_files



raw_data_folder = "raw"
# row_nr = 150
col_nr = 150
# Raw data can be found at:
# "https://onevdstest.blob.core.windows.net/testdata/Hilbert_attributes/Complex_Attributes_Top_Shetland.zip
server = 'http://127.0.0.1:8080'
vds = "https://vdsbenchmark.blob.core.windows.net/volve/ST0202ZDC12-PZ-PSDM-KIRCH-FULL-T.MIG_FIN.POST_STACK.3D.JS-017534/RLE_64"
# connection = "Suffix=?sv=2023-01-03&st=2024-08-23T07%3A22%3A40Z&se=2024-08-24T07%3A22%3A40Z&sr=c&sp=rl&sig=ywp0q%2FCvlQYjjctNsE5nHJiY%2FLmwVXaY0KUhTex7Zx0%3D"
sas = "?sv=2021-10-04&st=2024-08-26T10%3A35%3A11Z&se=2024-08-27T10%3A35%3A11Z&sr=c&sp=rl&sig=TN16EEDVorgACVEMkKLyWUK88NLHtzb9hCwv3xYrAz8%3D"
data_row = 150
fillvalue = -999.25


if __name__ == "__main__":


    cube = oneseismic.OneseismicClient(server, vds, sas)
    heights, surface = load_Top_Shetland_data(raw_data_folder)

    attributes = cube.attributes_along_surface(
        oneseismic.RegularSurface(
            heights,
            surface.xori,
            surface.yori,
            surface.xinc,
            surface.yinc * surface.yflip,
            surface.rotation,
            fillvalue = fillvalue
        ),
        above = 20,
        below = 20,
        stepsize = 0.1,
        attributes = ['samplevalue'],
    )

    surfaces = {}
    for name, data in attributes.items():
        surfaces[name] = xtgeo.RegularSurface(
            ncol = surface.ncol, 
            nrow = surface.nrow, 
            xori = surface.xori, 
            yori = surface.yori, 
            xinc = surface.xinc, 
            yinc = surface.yinc,
            rotation = surface.rotation, 
            values = np.ma.masked_where(data == fillvalue, data),
            yflip = surface.yflip
        )

    print("Surfaces", surfaces)

    fig, ax = plt.subplots(1, 1, figsize=(10, 10))
    ax.imshow(surfaces['samplevalue'].values.data, cmap='jet')

    plt.show()


    # top_shetland = surfaces['samplevalue'].data

    # print("Shape of top shetland", top_shetland.shape)
    # print("Top shetland", top_shetland)



    exit()



    data_files = get_raw_file_info(raw_data_folder)

    top_shetland = load_Top_Shetland_data(server, url, sas, raw_data_folder)

    all_data = {}
    for f in data_files:
        print(f)


    exit()



#         data = xtgeo.surface_from_file(f["full_path"])
#         data_row = data.values[:, col_nr]

#         print(f"Loading raw file: {f['full_path']}", data_row.shape)

#         # file_data = {}
#         # file_data["shape"] = data_row.shape
#         # file_data["values"] = data_row.tolist()
#         # file_data["attribute_type"] = f["attribute_type"]
#         # file_data["prefix_name"] = f["prefix_name"]
#         # file_data["row_nr"] = row_nr
#         # file_data["file_name"] = f["full_path"]
#         # all_data[f["attribute_type"]] = file_data

#         all_data[f["attribute_type"]] = data_row.tolist()

#     json_dump = json.dumps({'hilbert_data': all_data})

#     data = load_Volve_data(url, connection)
#     data_row = data[: col_nr]

#     print(url, data_row.shape)

#     all_data["raw"] = data_row.tolist()

#     print(f"Saving json file: hilbert_data.json")

#     json.dump(
#         all_data,
#         codecs.open("petrel_attribute_data.json", 'w', encoding='utf-8'),
#         separators=(',', ':'),
#         sort_keys=True,
#         indent=4)


#     # with open('petrel_attribute_data.json', 'w') as fw:
#     #     json.dump(json_dump, fw)
        
# exit()

    


# def read_petrel_data(full_file_name, prefix_name, suffix_name, data_row):

#     data = xtgeo.surface_from_file(full_file_name)

#     return data[data_row, :]

#     hilbert_data = {}
#     hilbert_data["name"] = short_name
#     hilbert_data["shape"] = data.values.shape
#     hilbert_data["values"] = data.values[data_row, :].tolist()

#     json_dump = json.dumps({'hilbert_data': hilbert_data})

#     print(f"Saving json file: {short_name}.json")
#     with open(short_name + '.json', 'w') as fw:
#         json.dump(json_dump, fw)






# files = os.listdir("raw")

# data_files = []
# for f in files:
#     if len(f) > 4 and f[-4] != "." and f.startswith("Extract value"):
        
#         short_name = f[15:-13]
#         parts = short_name.split(" ")
#         print()
#         print(short_name)
#         print(parts)


#         data_files.append([f, parts[0], parts[1][1:-1]])

# print()
# for f in data_files:
#     print(f)

# exit()

# hilbert_data = {}
# hilbert_data["name"] = short_name


# exit()

# for f in data_files:
#     print()
#     short_name = f[15:-13]
#     full_name = folder_name + "/" + f
#     print(f"Load raw file: {full_name}")
#     data = xtgeo.surface_from_file(full_name)

    
#     hilbert_data["shape"] = data.values.shape
#     hilbert_data["values"] = data.values[data_row, :].tolist()

#     json_dump = json.dumps({'hilbert_data': hilbert_data})

#     print(f"Saving json file: {short_name}.json")
#     with open(short_name + '.json', 'w') as fw:
#         json.dump(json_dump, fw)
