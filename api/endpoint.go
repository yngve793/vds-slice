package api

import (
	"encoding/json"
	"fmt"
	"net/http"
	"strings"

	"github.com/gin-gonic/gin"
	"github.com/gin-gonic/gin/binding"

	"github.com/equinor/vds-slice/internal/cache"
	"github.com/equinor/vds-slice/internal/core"
)

func httpStatusCode(err error) int {
	switch err.(type) {
	case *core.InvalidArgument:
		return http.StatusBadRequest
	case *core.InternalError:
		return http.StatusInternalServerError
	default:
		return http.StatusInternalServerError
	}
}

/* Call abortOnError on the context in case of an error
 *
 * This function is designed specifically for our endpoint handler functions
 * and aims at making the error handling as short and concise as possible.
 *
 * If err != nil the error will be mapped to an appropriate http status code
 * through the httpStatusCode mapper, and ctx.AbortWithError will be called
 * with this status and the error itself. It then returns true to indicate that
 * the context have been aborted.
 *
 * If err == nil the ctx is left untouched and this function returns false,
 * indicating that the context was not aborted.
 *
 * The result is a one line error handling:
 *
 *     err, _ := func()
 *     if abortOnError(ctx, err) { return }
 */
func abortOnError(ctx *gin.Context, err error) bool {
	if err == nil {
		return false
	}

	ctx.AbortWithError(httpStatusCode(err), err)

	return true
}

type Endpoint struct {
	MakeVdsConnection core.ConnectionMaker
	Cache             cache.Cache
}

func prepareRequestLogging(ctx *gin.Context, request Stringable) {
	// ignore possible errors as they should not change outcome for the user
	requestString, _ := request.toString()
	ctx.Set("request", requestString)
}

func (e *Endpoint) metadata(ctx *gin.Context, request MetadataRequest) {
	prepareRequestLogging(ctx, request)
	conn, err := e.MakeVdsConnection(request.Vds, request.Sas)
	if abortOnError(ctx, err) {
		return
	}

	handle, err := core.NewVDSHandle([]core.Connection{conn}, "")
	if abortOnError(ctx, err) {
		return
	}
	defer handle.Close()

	buffer, err := handle.GetMetadata()
	if abortOnError(ctx, err) {
		return
	}

	ctx.Data(http.StatusOK, "application/json", buffer)
}

func (e *Endpoint) makeDataRequest(
	ctx *gin.Context,
	request DataRequest,
) {
	prepareRequestLogging(ctx, request)

	vds_url, sas_key := request.credentials()
	var conn []core.Connection

	for i := 0; i < len(vds_url); i++ {
		fmt.Println(vds_url[i])
		conn_t, err := e.MakeVdsConnection(vds_url[i], sas_key[i])
		conn = append(conn, conn_t)
		if abortOnError(ctx, err) {
			return
		}
	}

	cacheKey, err := request.hash()
	if abortOnError(ctx, err) {
		return
	}

	cacheEntry, hit := e.Cache.Get(cacheKey)
	for i := 0; i < len(conn); i++ {
		if hit && conn[i].IsAuthorizedToRead() {
			ctx.Set("cache-hit", true)
			writeResponse(ctx, cacheEntry.Metadata(), cacheEntry.Data())
			return
		}
	}

	handle, err := core.NewVDSHandle(conn, request.cubeFunction())
	if abortOnError(ctx, err) {
		return
	}
	defer handle.Close()

	data, metadata, err := request.execute(handle)
	if abortOnError(ctx, err) {
		return
	}

	e.Cache.Set(cacheKey, cache.NewCacheEntry(data, metadata))

	writeResponse(ctx, metadata, data)
}

func (e *Endpoint) attributesAlong4dSurface(ctx *gin.Context, request AttributeAlong4dSurfaceRequest) {
	prepareRequestLogging(ctx, request)

	err := validateVerticalWindow(request.Above, request.Below, request.Stepsize)
	if abortOnError(ctx, err) {
		return
	}

	conn_A, err := e.MakeVdsConnection(request.Vds, request.Sas)
	if abortOnError(ctx, err) {
		return
	}

	conn_B, err := e.MakeVdsConnection(request.Vds_B, request.Sas_B)
	if abortOnError(ctx, err) {
		return
	}

	cacheKey, err := request.Hash()
	if abortOnError(ctx, err) {
		return
	}

	handle, err := core.NewVDSMultiHandle(conn_A, conn_B, "subtraction")
	if abortOnError(ctx, err) {
		return
	}
	defer handle.Close()

	cacheEntry, hit := e.Cache.Get(cacheKey)
	if hit && conn_A.IsAuthorizedToRead() {
		ctx.Set("cache-hit", true)
		writeResponse(ctx, cacheEntry.Metadata(), cacheEntry.Data())
		return
	}

	cacheEntry_B, hit := e.Cache.Get(cacheKey)
	if hit && conn_B.IsAuthorizedToRead() {
		ctx.Set("cache-hit", true)
		writeResponse(ctx, cacheEntry_B.Metadata(), cacheEntry_B.Data())
		return
	}

	interpolation, err := core.GetInterpolationMethod(request.Interpolation)
	if abortOnError(ctx, err) {
		return
	}

	metadata, err := handle.GetAttributeMetadata(request.Surface.Values)
	if abortOnError(ctx, err) {
		return
	}

	data, err := handle.GetAttributesAlongSurface(
		request.Surface,
		request.Above,
		request.Below,
		request.Stepsize,
		request.Attributes,
		interpolation,
	)
	if abortOnError(ctx, err) {
		return
	}

	e.Cache.Set(cacheKey, cache.NewCacheEntry(data, metadata))

	writeResponse(ctx, metadata, data)
}

func (e *Endpoint) AttributeBetween4dSurfaces(ctx *gin.Context, request AttributeBetween4dSurfacesRequest) {
	prepareRequestLogging(ctx, request)

	conn_A, err := e.MakeVdsConnection(request.Vds, request.Sas)
	if abortOnError(ctx, err) {
		return
	}

	conn_B, err := e.MakeVdsConnection(request.Vds_B, request.Sas_B)
	if abortOnError(ctx, err) {
		return
	}

	cacheKey, err := request.Hash()
	if abortOnError(ctx, err) {
		return
	}

	handle, err := core.NewVDSMultiHandle(conn_A, conn_B, "SUBTRACT")
	if abortOnError(ctx, err) {
		return
	}
	defer handle.Close()

	cacheEntry, hit := e.Cache.Get(cacheKey)
	if hit && conn_A.IsAuthorizedToRead() {
		ctx.Set("cache-hit", true)
		writeResponse(ctx, cacheEntry.Metadata(), cacheEntry.Data())
		return
	}

	cacheEntry_B, hit := e.Cache.Get(cacheKey)
	if hit && conn_B.IsAuthorizedToRead() {
		ctx.Set("cache-hit", true)
		writeResponse(ctx, cacheEntry_B.Metadata(), cacheEntry_B.Data())
		return
	}

	interpolation, err := core.GetInterpolationMethod(request.Interpolation)
	if abortOnError(ctx, err) {
		return
	}

	metadata, err := handle.GetAttributeMetadata(request.PrimarySurface.Values)
	if abortOnError(ctx, err) {
		return
	}

	data, err := handle.GetAttributesBetweenSurfaces(
		request.PrimarySurface,
		request.SecondarySurface,
		request.Stepsize,
		request.Attributes,
		interpolation,
	)
	if abortOnError(ctx, err) {
		return
	}

	e.Cache.Set(cacheKey, cache.NewCacheEntry(data, metadata))

	writeResponse(ctx, metadata, data)
}

func (request SliceRequest) execute(
	handle core.VDSHandle,
) (data [][]byte, metadata []byte, err error) {
	axis, err := core.GetAxis(strings.ToLower(request.Direction))
	if err != nil {
		return
	}

	metadata, err = handle.GetSliceMetadata(
		*request.Lineno,
		axis,
		request.Bounds,
	)
	if err != nil {
		return
	}

	res, err := handle.GetSlice(*request.Lineno, axis, request.Bounds)
	if err != nil {
		return
	}
	data = [][]byte{res}

	return data, metadata, nil
}

func (request SliceMultiRequest) execute(
	handle core.VDSHandle,
) (data [][]byte, metadata []byte, err error) {
	axis, err := core.GetAxis(strings.ToLower(request.Direction))
	if err != nil {
		return
	}

	metadata, err = handle.GetSliceMetadata(
		*request.Lineno,
		axis,
		request.Bounds,
	)
	if err != nil {
		return
	}

	res, err := handle.GetSlice(*request.Lineno, axis, request.Bounds)
	if err != nil {
		return
	}
	data = [][]byte{res}

	return data, metadata, nil
}

func (request FenceRequest) execute(
	handle core.VDSHandle,
) (data [][]byte, metadata []byte, err error) {
	coordinateSystem, err := core.GetCoordinateSystem(
		strings.ToLower(request.CoordinateSystem),
	)
	if err != nil {
		return
	}

	interpolation, err := core.GetInterpolationMethod(request.Interpolation)
	if err != nil {
		return
	}

	metadata, err = handle.GetFenceMetadata(request.Coordinates)
	if err != nil {
		return
	}

	res, err := handle.GetFence(
		coordinateSystem,
		request.Coordinates,
		interpolation,
		request.FillValue,
	)
	if err != nil {
		return
	}
	data = [][]byte{res}

	return data, metadata, nil
}

func (request FenceMultiRequest) execute(
	handle core.VDSHandle,
) (data [][]byte, metadata []byte, err error) {

	coordinateSystem, err := core.GetCoordinateSystem(
		strings.ToLower(request.CoordinateSystem),
	)
	if err != nil {
		return
	}

	interpolation, err := core.GetInterpolationMethod(request.Interpolation)
	if err != nil {
		return
	}

	metadata, err = handle.GetFenceMetadata(request.Coordinates)
	if err != nil {
		return
	}
	res, err := handle.GetFence(
		coordinateSystem,
		request.Coordinates,
		interpolation,
		request.FillValue,
	)
	if err != nil {
		return
	}
	data = [][]byte{res}
	return data, metadata, nil
}

func validateVerticalWindow(above float32, below float32, stepSize float32) error {
	const lowerBound = 0
	const upperBound = 250

	if above < lowerBound || above >= upperBound {
		return core.NewInvalidArgument(fmt.Sprintf(
			"'above' out of range! Must be within [%d, %d], was %f",
			lowerBound,
			upperBound,
			above,
		))
	}

	if below < lowerBound || below >= upperBound {
		return core.NewInvalidArgument(fmt.Sprintf(
			"'below' out of range! Must be within [%d, %d], was %f",
			lowerBound,
			upperBound,
			below,
		))
	}

	if stepSize < lowerBound {
		return core.NewInvalidArgument(fmt.Sprintf(
			"'stepsize' out of range! Must be bigger than %d, was %f",
			lowerBound,
			stepSize,
		))
	}

	return nil
}

func (request AttributeAlongSurfaceRequest) execute(
	handle core.VDSHandle,
) (data [][]byte, metadata []byte, err error) {
	err = validateVerticalWindow(request.Above, request.Below, request.Stepsize)
	if err != nil {
		return
	}

	interpolation, err := core.GetInterpolationMethod(request.Interpolation)
	if err != nil {
		return
	}

	metadata, err = handle.GetAttributeMetadata(request.Surface.Values)
	if err != nil {
		return
	}

	data, err = handle.GetAttributesAlongSurface(
		request.Surface,
		request.Above,
		request.Below,
		request.Stepsize,
		request.Attributes,
		interpolation,
	)
	if err != nil {
		return
	}

	return data, metadata, nil
}

func (request AttributeBetweenSurfacesRequest) execute(
	handle core.VDSHandle,
) (data [][]byte, metadata []byte, err error) {
	interpolation, err := core.GetInterpolationMethod(request.Interpolation)
	if err != nil {
		return
	}

	metadata, err = handle.GetAttributeMetadata(request.PrimarySurface.Values)
	if err != nil {
		return
	}

	data, err = handle.GetAttributesBetweenSurfaces(
		request.PrimarySurface,
		request.SecondarySurface,
		request.Stepsize,
		request.Attributes,
		interpolation,
	)
	if err != nil {
		return
	}

	return data, metadata, nil
}

func parseGetRequest(ctx *gin.Context, v Normalizable) error {
	query, status := ctx.GetQuery("query")
	if (!status){
		return core.NewInvalidArgument(
			"GET request to specified endpoint requires a 'query' parameter",
		)
	}
	if err := json.Unmarshal([]byte(query), v); err != nil {
		msg := "Please ensure that the supplied query is valid " +
			"and conforms to the expected swagger Request specification: %v"
		return core.NewInvalidArgument(
			fmt.Sprintf(msg, err.Error()))
	}

	if err := binding.Validator.ValidateStruct(v); err != nil {
		return core.NewInvalidArgument(err.Error())
	}

	return v.NormalizeConnection()
}

func parsePostRequest(ctx *gin.Context, v Normalizable) error {
	if err := ctx.ShouldBind(v); err != nil {
		return core.NewInvalidArgument(err.Error())
	}
	return v.NormalizeConnection()
}

func (e *Endpoint) Health(ctx *gin.Context) {

	ctx.HTML(http.StatusOK, "index.html", gin.H{})
}

// MetadataGet godoc
// @Summary  Return volumetric metadata about the VDS
// @description.markdown metadata
// @Tags     metadata
// @Param    query  query  string  True  "Urlencoded/escaped MetadataRequest"
// @Produce  json
// @Success  200 {object} core.Metadata
// @Failure  400 {object} ErrorResponse "Request is invalid"
// @Failure  500 {object} ErrorResponse "openvds failed to process the request"
// @Router   /metadata  [get]
func (e *Endpoint) MetadataGet(ctx *gin.Context) {
	var request MetadataRequest
	err := parseGetRequest(ctx, &request)
	if abortOnError(ctx, err) {
		return
	}

	e.metadata(ctx, request)
}

// MetadataPost godoc
// @Summary  Return volumetric metadata about the VDS
// @description.markdown metadata
// @Tags     metadata
// @Param    body  body  MetadataRequest  True  "Request parameters"
// @Produce  json
// @Success  200 {object} core.Metadata
// @Failure  400 {object} ErrorResponse "Request is invalid"
// @Failure  500 {object} ErrorResponse "openvds failed to process the request"
// @Router   /metadata  [post]
func (e *Endpoint) MetadataPost(ctx *gin.Context) {
	var request MetadataRequest
	err := parsePostRequest(ctx, &request)
	if abortOnError(ctx, err) {
		return
	}

	e.metadata(ctx, request)
}

// SliceGet godoc
// @Summary  Fetch a slice from a VDS
// @description.markdown slice
// @Tags     slice
// @Param    query  query  string  True  "Urlencoded/escaped SliceRequest"
// @Produce  multipart/mixed
// @Success  200 {object} core.SliceMetadata "(Example below only for metadata part)"
// @Failure  400 {object} ErrorResponse "Request is invalid"
// @Failure  500 {object} ErrorResponse "openvds failed to process the request"
// @Router   /slice  [get]
func (e *Endpoint) SliceGet(ctx *gin.Context) {
	var request SliceRequest
	err := parseGetRequest(ctx, &request)
	if abortOnError(ctx, err) {
		return
	}

	e.makeDataRequest(ctx, request)
}

// SlicePost godoc
// @Summary  Fetch a slice from a VDS
// @description.markdown slice
// @Tags     slice
// @Param    body  body  SliceRequest  True  "Query Parameters"
// @Accept   application/json
// @Produce  multipart/mixed
// @Success  200 {object} core.SliceMetadata "(Example below only for metadata part)"
// @Failure  400 {object} ErrorResponse "Request is invalid"
// @Failure  500 {object} ErrorResponse "openvds failed to process the request"
// @Router   /slice  [post]
func (e *Endpoint) SlicePost(ctx *gin.Context) {
	var request SliceRequest
	err := parsePostRequest(ctx, &request)
	if abortOnError(ctx, err) {
		return
	}

	e.makeDataRequest(ctx, request)
}

// FenceGet godoc
// @Summary  Returns traces along an arbitrary path, such as a well-path
// @description.markdown fence
// @Tags     fence
// @Param    query  query  string  True  "Urlencoded/escaped FenceResponse"
// @Accept   application/json
// @Produce  multipart/mixed
// @Success  200 {object} core.FenceMetadata "(Example below only for metadata part)"
// @Failure  400 {object} ErrorResponse "Request is invalid"
// @Failure  500 {object} ErrorResponse "openvds failed to process the request"
// @Router   /fence  [get]
func (e *Endpoint) FenceGet(ctx *gin.Context) {
	var request FenceRequest
	err := parseGetRequest(ctx, &request)
	if abortOnError(ctx, err) {
		return
	}

	e.makeDataRequest(ctx, request)
}

// FencePost godoc
// @Summary  Returns traces along an arbitrary path, such as a well-path
// @description.markdown fence
// @Tags     fence
// @Param    body  body  FenceRequest  True  "Request Parameters"
// @Accept   application/json
// @Produce  multipart/mixed
// @Success  200 {object} core.FenceMetadata "(Example below only for metadata part)"
// @Failure  400 {object} ErrorResponse "Request is invalid"
// @Failure  500 {object} ErrorResponse "openvds failed to process the request"
// @Router   /fence  [post]
func (e *Endpoint) FencePost(ctx *gin.Context) {
	var request FenceRequest
	err := parsePostRequest(ctx, &request)
	if abortOnError(ctx, err) {
		return
	}

	e.makeDataRequest(ctx, request)
}

// AttributesAlongSurfacePost godoc
// @Summary  Returns horizon attributes along the surface
// @description.markdown attribute_along
// @Tags     attributes
// @Param    body  body  AttributeAlongSurfaceRequest  True  "Request Parameters"
// @Accept   application/json
// @Produce  multipart/mixed
// @Success  200 {object} core.AttributeMetadata "(Example below only for metadata part)"
// @Failure  400 {object} ErrorResponse "Request is invalid"
// @Failure  500 {object} ErrorResponse "openvds failed to process the request"
// @Router   /attributes/surface/along  [post]
func (e *Endpoint) AttributesAlongSurfacePost(ctx *gin.Context) {
	var request AttributeAlongSurfaceRequest
	err := parsePostRequest(ctx, &request)
	if abortOnError(ctx, err) {
		return
	}

	e.makeDataRequest(ctx, request)
}

// AttributesBetweenSurfacesPost godoc
// @Summary  Returns horizon attributes between provided surfaces
// @description.markdown attribute_between
// @Tags     attributes
// @Param    body  body  AttributeBetweenSurfacesRequest  True  "Request Parameters"
// @Accept   application/json
// @Produce  multipart/mixed
// @Success  200 {object} core.AttributeMetadata "(Example below only for metadata part)"
// @Failure  400 {object} ErrorResponse "Request is invalid"
// @Failure  500 {object} ErrorResponse "openvds failed to process the request"
// @Router   /attributes/surface/between  [post]
func (e *Endpoint) AttributesBetweenSurfacesPost(ctx *gin.Context) {
	var request AttributeBetweenSurfacesRequest
	err := parsePostRequest(ctx, &request)
	if abortOnError(ctx, err) {
		return
	}

	e.makeDataRequest(ctx, request)
}

// AttributesAlong4dSurfacePost godoc
// @Summary  Returns horizon attributes along the surface of the difference between two cubes
// @description.markdown attribute_along_4d
// @Tags     attributes
// @Param    body  body  AttributeAlong4dSurfaceRequest  True  "Request Parameters"
// @Accept   application/json
// @Produce  multipart/mixed
// @Success  200 {object} core.AttributeMetadata "(Example below only for metadata part)"
// @Failure  400 {object} ErrorResponse "Request is invalid"
// @Failure  500 {object} ErrorResponse "openvds failed to process the request"
// @Router   /attributes/surface/along4d  [post]
func (e *Endpoint) AttributesAlong4dSurfacePost(ctx *gin.Context) {
	var request AttributeAlong4dSurfaceRequest
	err := parsePostRequest(ctx, &request)
	if abortOnError(ctx, err) {
		return
	}

	e.attributesAlong4dSurface(ctx, request)
}

// AttributesBetween4dSurfacesPost godoc
// @Summary  Returns horizon attributes between provided surfaces of the difference between two cubes
// @description.markdown attribute_between_4d
// @Tags     attributes
// @Param    body  body  AttributeBetween4dSurfacesRequest  True  "Request Parameters"
// @Accept   application/json
// @Produce  multipart/mixed
// @Success  200 {object} core.AttributeMetadata "(Example below only for metadata part)"
// @Failure  400 {object} ErrorResponse "Request is invalid"
// @Failure  500 {object} ErrorResponse "openvds failed to process the request"
// @Router   /attributes/surface/between4d  [post]
func (e *Endpoint) AttributesBetween4dSurfacesPost(ctx *gin.Context) {
	var request AttributeBetween4dSurfacesRequest
	err := parsePostRequest(ctx, &request)
	if abortOnError(ctx, err) {
		return
	}

	e.AttributeBetween4dSurfaces(ctx, request)
}

// FenceMultiPost godoc
// @Summary  Returns traces along an arbitrary path in the difference between two cubes, such as a well-path
// @description.markdown fence
// @Tags     multi_cube
// @Param    body  body  FenceMultiRequest  True  "Request Parameters"
// @Accept   application/json
// @Produce  multipart/mixed
// @Success  200 {object} core.FenceMetadata "(Example below only for metadata part)"
// @Failure  400 {object} ErrorResponse "Request is invalid"
// @Failure  500 {object} ErrorResponse "openvds failed to process the request"
// @Router   /multi_cube/fence  [post]
func (e *Endpoint) FenceMultiPost(ctx *gin.Context) {
	var request FenceMultiRequest
	err := parsePostRequest(ctx, &request)
	if abortOnError(ctx, err) {
		return
	}

	e.makeDataRequest(ctx, request)
}

// SliceMultiPost godoc
// @Summary  Fetch a slice from a VDS in the difference between two cubes
// @description.markdown slice
// @Tags     multi_cube
// @Param    body  body  SliceMultiRequest  True  "Query Parameters"
// @Accept   application/json
// @Produce  multipart/mixed
// @Success  200 {object} core.SliceMetadata "(Example below only for metadata part)"
// @Failure  400 {object} ErrorResponse "Request is invalid"
// @Failure  500 {object} ErrorResponse "openvds failed to process the request"
// @Router   /multi_cube/slice  [post]
func (e *Endpoint) SliceMultiPost(ctx *gin.Context) {
	var request SliceMultiRequest
	err := parsePostRequest(ctx, &request)
	if abortOnError(ctx, err) {
		return
	}

	e.makeDataRequest(ctx, request)
}
