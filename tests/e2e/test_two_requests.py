import http
import requests
import numpy as np
import os
import pytest
import json
import re
import urllib.parse
from utils.cloud import *

from azure.storage import blob
from azure.storage import filedatalake

from requests_toolbelt.multipart import decoder

STORAGE_ACCOUNT_NAME = os.getenv("STORAGE_ACCOUNT_NAME")
STORAGE_ACCOUNT_KEY = os.getenv("STORAGE_ACCOUNT_KEY")
ENDPOINT = os.getenv("ENDPOINT", "http://localhost:8080").rstrip("/")
CONTAINER = "testdata"
VDS = "well_known/well_known_default"
STORAGE_ACCOUNT = f"https://{STORAGE_ACCOUNT_NAME}.blob.core.windows.net"
VDSURL = f"{STORAGE_ACCOUNT}/{CONTAINER}/{VDS}"

SAMPLES10_URL = f"{STORAGE_ACCOUNT}/{CONTAINER}/10_samples/10_samples_default"


def gen_default_sas():
    return generate_container_signature(STORAGE_ACCOUNT_NAME, CONTAINER, STORAGE_ACCOUNT_KEY)


def surface():
    return {
        "xinc": 7.2111,
        "yinc": 3.6056,
        "xori": 2,
        "yori": 0,
        "rotation": 33.69,
    }

def make_slice_payload(vds=VDSURL, direction="inline", lineno=3, sas="sas"):
    return {
        "vds_urls": [vds],
        "direction": direction,
        "lineno": lineno,
        "sas_keys": [sas]
    }

# def make_function_slice_request(vds_1=VDSURL_1, vds_2=VDSURL_2, direction="inline", lineno=3, sas="sas"):
#     return {
#         "vds_urls": [vds_1, vds_2],
#         "direction": direction,
#         "lineno": lineno,
#         "sas_keys": [sas, sas]
#     }


def make_fence_payload(vds=VDSURL, coordinate_system="ij", coordinates=[[0, 0]], sas="sas"):
    return {
        "vds_urls": [vds],
        "coordinateSystem": coordinate_system,
        "coordinates": coordinates,
        "sas_keys": [sas]
    }


def make_metadata_payload(vds=VDSURL, sas="sas"):
    return {
        "vds": vds,
        "sas": sas
    }


def make_attributes_along_surface_payload(
    vds=SAMPLES10_URL,
    surface=surface(),
    values=[[20]],
    above=8,
    below=8,
    attributes=["samplevalue"],
    sas="sas"
):
    regular_surface = {
        "values": values,
        "fillValue": -999.25,
    }
    regular_surface.update(surface)

    request = {
        "surface": regular_surface,
        "interpolation": "nearest",
        "vds_urls": [vds],
        "sas_keys": [sas],
        "above": above,
        "below": below,
        "attributes": attributes
    }
    return request


def make_attributes_between_surfaces_payload(
    primaryValues=[[20]],
    secondaryValues=[[20]],
    vds=SAMPLES10_URL,
    surface=surface(),
    attributes=["max"],
    stepsize=8,
    sas="sas"
):
    fillValue = -999.25
    primary = {
        "values": primaryValues,
        "fillValue": fillValue
    }
    primary.update(surface)
    secondary = {
        "values": secondaryValues,
        "fillValue": fillValue
    }
    secondary.update(surface)
    request = {
        "primarySurface": primary,
        "secondarySurface": secondary,
        "interpolation": "nearest",
        "vds_urls": [vds],
        "sas_keys": [sas],
        "stepsize": stepsize,
        "attributes": attributes
    }
    request.update(surface)
    return request


def test_slice():
    sas = generate_container_signature(STORAGE_ACCOUNT_NAME, CONTAINER, STORAGE_ACCOUNT_KEY)
    payload = make_slice_payload(VDSURL, direction='inline', lineno=5, sas=sas)
    meta, slice_data = make_data_request("two_cubes/slice", "post", payload)

    expected = np.array([[116, 117, 118, 119],
                         [120, 121, 122, 123]])
    assert np.array_equal(slice_data, expected)

    expected_meta = json.loads("""
    {
        "x": {"annotation": "Sample", "max": 16.0, "min": 4.0, "samples" : 4, "stepsize": 4.0, "unit": "ms"},
        "y": {"annotation": "Crossline", "max": 11.0, "min": 10.0, "samples" : 2, "stepsize": 1.0, "unit": "unitless"},
        "shape": [ 2, 4],
        "format": "<f4",
        "geospatial": [[14.0, 8.0], [12.0, 11.0]]
    }
    """)
    assert meta == expected_meta


# def test_slice_two():
#     meta, slice = request_slice("post", 5, 'inline')

#     expected = np.array([[116, 117, 118, 119],
#                          [120, 121, 122, 123]])
#     assert np.array_equal(slice, expected)

#     expected_meta = json.loads("""
#     {
#         "x": {"annotation": "Sample", "max": 16.0, "min": 4.0, "samples" : 4, "stepsize": 4.0, "unit": "ms"},
#         "y": {"annotation": "Crossline", "max": 11.0, "min": 10.0, "samples" : 2, "stepsize": 1.0, "unit": "unitless"},
#         "shape": [ 2, 4],
#         "format": "<f4",
#         "geospatial": [[14.0, 8.0], [12.0, 11.0]]
#     }
#     """)
#     assert meta == expected_meta


@pytest.mark.parametrize("method", [
    # ("get"),
    ("post")
])
def test_fence(method):
    meta, fence = request_fence(method, [[3, 10], [1, 11]], 'ilxl')

    expected = np.array([[108, 109, 110, 111],
                         [104, 105, 106, 107]])
    assert np.array_equal(fence, expected)

    expected_meta = json.loads("""
    {
        "shape": [ 2, 4],
        "format": "<f4"
    }
    """)
    assert meta == expected_meta


def test_attributes_along_surface():
    values = [
        [20, 20],
        [20, 20],
        [20, 20]
    ]
    meta, data = request_attributes_along_surface("post", values)

    expected = np.array([[-0.5, 0.5], [-8.5, 6.5], [16.5, -16.5]])
    assert np.array_equal(data, expected)

    expected_meta = json.loads("""
    {
        "shape": [3, 2],
        "format": "<f4"
    }
    """)
    assert meta == expected_meta


def test_attributes_between_surfaces():
    primary = [
        [12, 12],
        [12, 14],
        [22, 12]
    ]
    secondary = [
        [30,   28],
        [27.5, 29],
        [24,   12]
    ]
    meta, data = request_attributes_between_surfaces(
        "post", primary, secondary)

    expected = np.array([[1.5, 2.5], [-8.5, 7.5], [18.5, -8.5]])
    assert np.array_equal(data, expected)

    expected_meta = json.loads("""
    {
        "shape": [3, 2],
        "format": "<f4"
    }
    """)
    assert meta == expected_meta


@pytest.mark.parametrize("path, payload", [
    ("two_cubes/slice",make_slice_payload(vds=VDSURL)),
    ("two_cubes/fence", make_fence_payload()),
    ("two_cubes/attributes/surface/along", make_attributes_along_surface_payload()),
    ("two_cubes/attributes/surface/between",
     make_attributes_between_surfaces_payload()),
])
@pytest.mark.parametrize("sas, allowed_error_messages", [
    (
        "something_not_sassy",
        [
            "409 Public access is not permitted",
            "401 Server failed to authenticate the request"
        ]
    )
])
def test_assure_no_unauthorized_access(path, payload, sas, allowed_error_messages):
    payload.update({"sas_keys": [sas]})
    res = send_request(path, "post", payload)
    assert res.status_code == http.HTTPStatus.INTERNAL_SERVER_ERROR
    error_body = json.loads(res.content)['error']
    assert any([error_msg in error_body for error_msg in allowed_error_messages]), \
        f'error body \'{error_body}\' does not contain any of the valid errors {allowed_error_messages}'


@pytest.mark.parametrize("path, payload", [
    ("two_cubes/slice", make_slice_payload(vds=VDSURL)),
    ("two_cubes/fence", make_fence_payload(vds=VDSURL)),
    ("two_cubes/attributes/surface/along", make_attributes_along_surface_payload()),
    ("two_cubes/attributes/surface/between", make_attributes_between_surfaces_payload()),
])
@pytest.mark.parametrize("token, status, error", [
    (generate_container_signature(
        STORAGE_ACCOUNT_NAME, CONTAINER, STORAGE_ACCOUNT_KEY,
        permission=blob.ContainerSasPermissions(read=True)),
     http.HTTPStatus.OK, None),
    (generate_account_signature(
        STORAGE_ACCOUNT_NAME, STORAGE_ACCOUNT_KEY, permission=blob.AccountSasPermissions(
            read=True), resource_types=blob.ResourceTypes(container=True, object=True)),
     http.HTTPStatus.OK, None),
    (generate_blob_signature(
        STORAGE_ACCOUNT_NAME, CONTAINER, f'{VDS}/VolumeDataLayout', STORAGE_ACCOUNT_KEY,
        permission=blob.BlobSasPermissions(read=True)),
     http.HTTPStatus.INTERNAL_SERVER_ERROR, "403 Server failed to authenticate the request"),
    pytest.param(
        generate_directory_signature(
            STORAGE_ACCOUNT_NAME, CONTAINER, VDS, STORAGE_ACCOUNT_KEY,
            permission=filedatalake.DirectorySasPermissions(read=True)),
        http.HTTPStatus.OK, None,
        marks=pytest.mark.skipif(
            not is_account_datalake_gen2(
                STORAGE_ACCOUNT_NAME, CONTAINER, VDS, STORAGE_ACCOUNT_KEY),
            reason="storage account is not a datalake")
    ),
])
# test makes sense if caching is enabled on the endpoint server
def test_cached_data_access_with_various_sas(path, payload, token, status, error):

    def make_caching_call():
        container_sas = generate_container_signature(
            STORAGE_ACCOUNT_NAME,
            CONTAINER,
            STORAGE_ACCOUNT_KEY,
            permission=blob.ContainerSasPermissions(read=True))
        
        payload.update({"sas_keys": [container_sas]})
        res = send_request(path, "post", payload)
        assert res.status_code == http.HTTPStatus.OK

    make_caching_call()

    payload.update({"sas_keys": [token]})
    res = send_request(path, "post", payload)
    assert res.status_code == status
    if error:
        assert error in json.loads(res.content)['error']


@pytest.mark.parametrize("path, payload", [
    ("two_cubes/slice", make_slice_payload()),
    ("two_cubes/fence", make_fence_payload()),
    # ("metadata", make_metadata_request()),
    ("two_cubes/attributes/surface/along", make_attributes_along_surface_payload()),
    ("two_cubes/attributes/surface/between", make_attributes_between_surfaces_payload()),
])
def test_assure_only_allowed_storage_accounts(path, payload):
    payload.update({
        "vds_urls": ["https://dummy.blob.core.windows.net/container/blob"],
    })
    res = send_request(path, "post", payload)
    assert res.status_code == http.HTTPStatus.BAD_REQUEST
    body = json.loads(res.content)
    assert "unsupported storage account" in body['error']


@pytest.mark.parametrize("path, payload, error_code, error", [
    (
        "two_cubes/slice",
        make_slice_payload(direction="inline", lineno=4),
        http.HTTPStatus.BAD_REQUEST,
        "Invalid lineno: 4, valid range: [1.00:5.00:2.00]"
    ),
    (
        "two_cubes/fence",
        {"param": "irrelevant"},
        http.HTTPStatus.BAD_REQUEST,
        "Error:Field validation for"
    ),
    (
        "two_cubes/attributes/surface/along",
        make_attributes_along_surface_payload(surface={}),
        http.HTTPStatus.BAD_REQUEST,
        "Error:Field validation for"
    ),
    (
        "two_cubes/attributes/surface/between",
        make_attributes_between_surfaces_payload(
            primaryValues=[[1]], secondaryValues=[[1], [1, 1]]),
        http.HTTPStatus.BAD_REQUEST,
        "Surface rows are not of the same length"
    ),
])
def test_errors(path, payload, error_code, error):
    sas = generate_container_signature(
        STORAGE_ACCOUNT_NAME, CONTAINER, STORAGE_ACCOUNT_KEY)
    payload.update({"sas_keys": [sas]})
    res = send_request(path, "post", payload)
    assert res.status_code == error_code

    body = json.loads(res.content)
    assert error in body['error']


def send_request(path, method, payload):
    if method == "get":
        json_payload = json.dumps(payload)
        encoded_payload = urllib.parse.quote(json_payload)
        data = requests.get(f'{ENDPOINT}/{path}?query={encoded_payload}')
    elif method == "post":
        data = requests.post(f'{ENDPOINT}/{path}', json=payload)
    else:
        raise ValueError(f'Unknown method {method}')
    return data

def make_data_request(path, method, payload):
    requested_data = send_request("two_cubes/slice", method, payload)
    requested_data.raise_for_status()

    multipart_data = decoder.MultipartDecoder.from_response(requested_data)
    assert len(multipart_data.parts) == 2
    metadata = json.loads(multipart_data.parts[0].content)
    data = multipart_data.parts[1].content

    data = np.ndarray(metadata['shape'], metadata['format'], data)
    return metadata, data



def request_slice(method, lineno, direction):
    sas = generate_container_signature(
        STORAGE_ACCOUNT_NAME, CONTAINER, STORAGE_ACCOUNT_KEY)

    payload = make_slice_payload(VDSURL, direction, lineno, sas)
    rdata = send_request("two_cubes/slice", method, payload)
    rdata.raise_for_status()

    multipart_data = decoder.MultipartDecoder.from_response(rdata)
    assert len(multipart_data.parts) == 2
    metadata = json.loads(multipart_data.parts[0].content)
    data = multipart_data.parts[1].content

    data = np.ndarray(metadata['shape'], metadata['format'], data)
    return metadata, data

def request_fence(method, coordinates, coordinate_system):
    sas = generate_container_signature(
        STORAGE_ACCOUNT_NAME, CONTAINER, STORAGE_ACCOUNT_KEY)

    payload = make_fence_payload(VDSURL, coordinate_system, coordinates, sas)
    rdata = send_request("two_cubes/fence", method, payload)
    rdata.raise_for_status()

    multipart_data = decoder.MultipartDecoder.from_response(rdata)
    assert len(multipart_data.parts) == 2
    metadata = json.loads(multipart_data.parts[0].content)
    data = multipart_data.parts[1].content

    data = np.ndarray(metadata['shape'], metadata['format'], data)
    return metadata, data


def request_metadata(method):
    sas = generate_container_signature(
        STORAGE_ACCOUNT_NAME, CONTAINER, STORAGE_ACCOUNT_KEY)

    payload = make_metadata_payload(VDSURL, sas)
    rdata = send_request("metadata", method, payload)
    rdata.raise_for_status()

    return rdata.json()


def request_attributes_along_surface(method, values):
    sas = generate_container_signature(
        STORAGE_ACCOUNT_NAME, CONTAINER, STORAGE_ACCOUNT_KEY)

    payload = make_attributes_along_surface_payload(values=values, sas=sas)
    rdata = send_request("two_cubes/attributes/surface/along", method, payload)
    rdata.raise_for_status()

    multipart_data = decoder.MultipartDecoder.from_response(rdata)
    assert len(multipart_data.parts) == 2
    metadata = json.loads(multipart_data.parts[0].content)
    data = multipart_data.parts[1].content

    data = np.ndarray(metadata['shape'], metadata['format'], data)
    return metadata, data


def request_attributes_between_surfaces(method, primary, secondary):
    sas = generate_container_signature(
        STORAGE_ACCOUNT_NAME, CONTAINER, STORAGE_ACCOUNT_KEY)

    payload = make_attributes_between_surfaces_payload(
        primary, secondary, sas=sas)
    rdata = send_request(
        "two_cubes/attributes/surface/between", method, payload)
    rdata.raise_for_status()

    multipart_data = decoder.MultipartDecoder.from_response(rdata)
    assert len(multipart_data.parts) == 2
    metadata = json.loads(multipart_data.parts[0].content)
    data = multipart_data.parts[1].content

    data = np.ndarray(metadata['shape'], metadata['format'], data)
    return metadata, data


@pytest.mark.parametrize("path, payload", [
    ("two_cubes/slice",   make_slice_payload()),
    ("two_cubes/fence",   make_fence_payload()),
    ("two_cubes/attributes/surface/along", make_attributes_along_surface_payload()),
    ("two_cubes/attributes/surface/between",
     make_attributes_between_surfaces_payload()),
])
@pytest.mark.parametrize("vds, sas, expected", [
    (f'{SAMPLES10_URL}?{gen_default_sas()}', '', http.HTTPStatus.OK),
    (f'{SAMPLES10_URL}?invalid_sas', gen_default_sas(), http.HTTPStatus.OK),
    (SAMPLES10_URL, '', http.HTTPStatus.BAD_REQUEST)
])
def test_sas_list_token_in_url(path, payload, vds, sas, expected):
    payload.update({
        "vds_urls": [vds],
        "sas_keys": [sas]
    })
    res = send_request(path, "post", payload)
    assert res.status_code == expected
    gen_default_sas()


@pytest.mark.parametrize("path, payload", [
    ("two_cubes/slice",   make_slice_payload()),
    ("two_cubes/fence",   make_fence_payload()),
    ("two_cubes/attributes/surface/along", make_attributes_along_surface_payload()),
    ("two_cubes/attributes/surface/between",
     make_attributes_between_surfaces_payload()),
])
@pytest.mark.parametrize("vds, sas, expected", [
    (f'{SAMPLES10_URL}?{gen_default_sas()}', '', http.HTTPStatus.OK),
    (f'{SAMPLES10_URL}?invalid_sas', gen_default_sas(), http.HTTPStatus.OK),
    (SAMPLES10_URL, '', http.HTTPStatus.BAD_REQUEST)
])
def test_sas_token_in_url(path, payload, vds, sas, expected):
    payload.update({
        "vds_urls": [vds],
        "sas_keys": [sas]
    })
    res = send_request(path, "post", payload)
    assert res.status_code == expected
    gen_default_sas()
