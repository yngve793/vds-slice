import json
import openvds

file_name = "ST0202ZDC12-PZ-PSDM-KIRCH-FULL-T.MIG_FIN.POST_STACK.3D.JS-017534"
url = "azureSAS://vdsbenchmark.blob.core.windows.net/volve/" + file_name + "/RLE_64"
connection = "Suffix=?sv=2023-01-03&st=2024-08-23T07%3A22%3A40Z&se=2024-08-24T07%3A22%3A40Z&sr=c&sp=rl&sig=ywp0q%2FCvlQYjjctNsE5nHJiY%2FLmwVXaY0KUhTex7Zx0%3D"
data_row = 150

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

data = req.data.reshape(width, height).transpose()

hilbert_data = {}
hilbert_data["name"] = file_name
hilbert_data["shape"] = data.shape
hilbert_data["values"] = data[data_row, :].tolist()

json_dump = json.dumps({'hilbert_data': hilbert_data})

print(f"Saving json file: {file_name}.json")
with open(file_name + '.json', 'w') as fw:
    json.dump(json_dump, fw)
