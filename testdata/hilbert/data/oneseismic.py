import json
import requests

import numpy as np

from functools import lru_cache
from requests_toolbelt.multipart.decoder import MultipartDecoder


class OneseismicClient:
    """A conveinience client for interacting with the oneseismic API

    oneseismic is a WebAPI for reading partial data from OpenVDS files
    in the cloud. It provides endpoints for:

        - Quickly read metadata from an OpenVDS file
        - Reading a slice in any direction (inline, crossline, depth/time)
        - Reading traces along some path, e.g. a well-path
        - Reading seismic data along a provided horizon
        - Computing attributes along a provided horizon, e.g. RMS.

    This client simply wraps the actuall http requests to the API to make
    the caller code a bit more concise.

    The oneseismic API is not officially a part of OSDU, but you can still
    use it for reading OpenVDS data as long as you have the blobpath and
    a valid sastoken - which we can get from OSDU's seismic APIs
    """
    def __init__(self, host, vds, sas = None):
        self.host = host
        self.vds  = vds
        self.sas  = sas

    @lru_cache
    def metadata(self):
        """Get metadata from the OpenVDS volume"""

        response = requests.post(f'{self.host}/metadata',
            headers = { 'Content-Type' : 'application/json' },
            data    = json.dumps({
                'vds' : self.vds,
                'sas' : self.sas
            })
        )
        if not response.ok: raise RuntimeError(response.text)
        return response.json()

    @lru_cache
    def slice(self, direction, lineno):
        """ Get an seismic slice in any direction

        Parameters
        ----------

        direction: str
            Supported options are: 'inline', 'crossline', 'time', 'depth',
            'sample', 'i', 'j', 'k'. Which of time/depth/sample is availible
            depends on the seismic volume. i, j, k are zero-indexed alternatives
            to inline, crossline, depth/time/sample.
        lineno: int
            Linenumber of the slice
        vds: str
            blob-path to the vds file
        sas: str
            sas-token with read access on "vds".

        Returns
        -------

        data: np.array
            A 2D numpy array containing the slice
        metadata: dict
            metadata about the slice, such as axis-labels and other information
            useful for e.g. plotting
        """
        response = requests.post(f'{self.host}/slice',
            headers = {'Content-Type': 'application/json'},
            data = json.dumps({
                'direction' : direction,
                'lineno'    : lineno,
                'vds'       : self.vds,
                'sas'       : self.sas
            })
        )
        if not response.ok: raise RuntimeError(response.text)
        parts = MultipartDecoder.from_response(response).parts

        meta = json.loads(parts[0].content)

        return np.ndarray(meta['shape'], meta['format'], parts[1].content), meta

    def fence(self, coordinates, system):
        payload = json.dumps({
            'coordinates'      : coordinates,
            'coordinateSystem' : system,
            'vds'              : self.vds,
            'sas'              : self.sas
        })
        return self._send_fence_request(payload)
    
    @lru_cache
    def _send_fence_request(self, data):
        response = requests.post(f'{self.host}/fence',
            headers = {'Content-Type': 'application/json'},
            data = data
        )
        if not response.ok: raise RuntimeError(response.text)
        parts = MultipartDecoder.from_response(response).parts

        meta = json.loads(parts[0].content)
        data = parts[1].content

        return np.ndarray(meta['shape'], meta['format'], data), meta

    def attributes_along_surface(
        self, 
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


    # @lru_cache
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


    def attributes_between_surfaces(
        self,
        primary,
        secondary,
        stepsize,
        attributes,
        interpolation = 'linear'
    ):
        payload = json.dumps({
            'vds'              : self.vds,
            'sas'              : self.sas,
            'primarySurface'   : primary.to_dict(),
            'secondarySurface' : secondary.to_dict(),
            'stepsize'         : stepsize,
            'attributes'       : attributes,
            'interpolation'    : interpolation
        })

        return self._send_attributes_between_surfaces_request(payload)


    # @lru_cache
    def _send_attributes_between_surfaces_request(self, data):
        response = requests.post(f'{self.host}/attributes/surface/between',
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
