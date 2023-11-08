package core

import (
	"fmt"
	"testing"

	"github.com/stretchr/testify/require"
)

// CGO not supported in tests yet.
// Hardcoding the cube function values
const (
	SUBTRACT       = 0
	ADDITION       = 1
	MULTIPLICATION = 2
	DIVISION       = 3
)

func TestCubeFunctionsForAttributeAlong4D(t *testing.T) {
	testcases := []struct {
		function_name string
		function      int
		expected      []float32
	}{
		{
			function_name: "SUBTRACT",
			function:      SUBTRACT,
			expected: []float32{
				fillValue, fillValue, fillValue, fillValue, fillValue, fillValue, fillValue,
				fillValue, fillValue, -12.5, fillValue, 4.5, fillValue, 1.5,
				fillValue, fillValue, 12.5, fillValue, -10.5, fillValue, -1.5,
			},
		},
		{
			function_name: "ADDITION",
			function:      ADDITION,
			expected: []float32{
				fillValue, fillValue, fillValue, fillValue, fillValue, fillValue, fillValue,
				fillValue, fillValue, (2 + 1) * -12.5, fillValue, (2 + 1) * 4.5, fillValue, (2 + 1) * 1.5,
				fillValue, fillValue, (2 + 1) * 12.5, fillValue, (2 + 1) * -10.5, fillValue, (2 + 1) * -1.5,
			},
		},
		{
			function_name: "MULTIPLICATION",
			function:      MULTIPLICATION,
			expected: []float32{
				fillValue, fillValue, fillValue, fillValue, fillValue, fillValue, fillValue,
				fillValue, fillValue, 2 * (-12.5 * -12.5), fillValue, 2 * (4.5 * 4.5), fillValue, 2 * (1.5 * 1.5),
				fillValue, fillValue, 2 * (12.5 * 12.5), fillValue, 2 * (-10.5 * -10.5), fillValue, 2 * (-1.5 * -1.5),
			},
		},
		{
			function_name: "DIVISION",
			function:      DIVISION,
			expected: []float32{
				fillValue, fillValue, fillValue, fillValue, fillValue, fillValue, fillValue,
				fillValue, fillValue, 2 * -12.5 / (-12.5), fillValue, 2 * 4.5 / (4.5), fillValue, 2 * 1.5 / (1.5),
				fillValue, fillValue, 2 * 12.5 / (12.5), fillValue, 2 * -10.5 / (-10.5), fillValue, 2 * -1.5 / (-1.5),
			},
		},
	}

	for _, testcase := range testcases {
		const above = float32(4.0)
		const below = float32(4.0)
		const stepsize = float32(4.0)
		var targetAttributes = []string{"samplevalue"}

		values := [][]float32{
			{fillValue, fillValue, fillValue, fillValue, fillValue, fillValue, fillValue},
			{fillValue, fillValue, 16, fillValue, 16, fillValue, 16},
			{fillValue, fillValue, 16, fillValue, 16, fillValue, 16},
		}

		rotation := samples10_grid.rotation + 270
		xinc := samples10_grid.xinc / 2.0
		yinc := -samples10_grid.yinc
		xori := float32(16)
		yori := float32(18)

		surface := RegularSurface{
			Values:    values,
			Rotation:  &rotation,
			Xori:      &xori,
			Yori:      &yori,
			Xinc:      xinc,
			Yinc:      yinc,
			FillValue: &fillValue,
		}

		interpolationMethod, _ := GetInterpolationMethod("nearest")

		handle, _ := NewVDSMultiHandle(samples10_2x, samples10, testcase.function)

		defer handle.Close()
		buf, err := handle.GetAttributesAlongSurface(
			surface,
			above,
			below,
			stepsize,
			targetAttributes,
			interpolationMethod,
		)
		require.Len(t, buf, len(targetAttributes), "Wrong number of attributes")
		require.NoErrorf(t, err, "Failed to fetch horizon")
		result, err := toFloat32(buf[0])
		require.NoErrorf(t, err, "Failed to covert to float32 buffer")
		require.Equalf(t, testcase.expected, *result, "Horizon not as expected")
	}
}

func TestCubeFunctionsForAttributeBetween4D(t *testing.T) {

	testcases := []struct {
		function_name string
		function      int
		expected      map[string][]float32
	}{
		{
			function_name: "SUBTRACT",
			function:      SUBTRACT,
			expected: map[string][]float32{
				"min":       {-1.5, -0.5, -8.5, 5.5, fillValue, -24.5, fillValue, fillValue},
				"min_at":    {16, 24, 20, 18, fillValue, 28, fillValue, fillValue},
				"max":       {2.5, 0.5, -8.5, 5.5, fillValue, -8.5, fillValue, fillValue},
				"max_at":    {32, 20, 20, 18, fillValue, 12, fillValue, fillValue},
				"maxabs":    {2.5, 0.5, 8.5, 5.5, fillValue, 24.5, fillValue, fillValue},
				"maxabs_at": {32, 20, 20, 18, fillValue, 28, fillValue, fillValue},
				"mean":      {0.5, 0, -8.5, 5.5, fillValue, -16.5, fillValue, fillValue},
				"meanabs":   {1.3, 0.5, 8.5, 5.5, fillValue, 16.5, fillValue, fillValue},
				"meanpos":   {1.5, 0.5, 0, 5.5, fillValue, 0, fillValue, fillValue},
				"meanneg":   {-1, -0.5, -8.5, 0, fillValue, -16.5, fillValue, fillValue},
				"median":    {0.5, 0, -8.5, 5.5, fillValue, -16.5, fillValue, fillValue},
				"rms":       {1.5, 0.5, 8.5, 5.5, fillValue, 17.442764, fillValue, fillValue},
				"var":       {2, 0.25, 0, 0, fillValue, 32, fillValue, fillValue},
				"sd":        {1.4142135, 0.5, 0, 0, fillValue, 5.656854, fillValue, fillValue},
				"sumpos":    {4.5, 0.5, 0, 5.5, fillValue, 0, fillValue, fillValue},
				"sumneg":    {-2, -0.5, -8.5, 0, fillValue, -82.5, fillValue, fillValue},
			},
		},
		{
			function_name: "ADDITION",
			function:      ADDITION,
			expected: map[string][]float32{
				"min":       {3 * -1.5, 3 * -0.5, 3 * -8.5, 3 * 5.5, fillValue, 3 * -24.5, fillValue, fillValue},
				"min_at":    {16, 24, 20, 18, fillValue, 28, fillValue, fillValue},
				"max":       {3 * 2.5, 3 * 0.5, 3 * -8.5, 3 * 5.5, fillValue, 3 * -8.5, fillValue, fillValue},
				"max_at":    {32, 20, 20, 18, fillValue, 12, fillValue, fillValue},
				"maxabs":    {3 * 2.5, 3 * 0.5, 3 * 8.5, 3 * 5.5, fillValue, 3 * 24.5, fillValue, fillValue},
				"maxabs_at": {32, 20, 20, 18, fillValue, 28, fillValue, fillValue},
				"mean":      {3 * 0.5, 3 * 0, 3 * -8.5, 3 * 5.5, fillValue, 3 * -16.5, fillValue, fillValue},
				"meanabs":   {3 * 1.3, 3 * 0.5, 3 * 8.5, 3 * 5.5, fillValue, 3 * 16.5, fillValue, fillValue},
				"meanpos":   {3 * 1.5, 3 * 0.5, 3 * 0, 3 * 5.5, fillValue, 3 * 0, fillValue, fillValue},
				"meanneg":   {3 * -1, 3 * -0.5, 3 * -8.5, 0, fillValue, 3 * -16.5, fillValue, fillValue},
				"median":    {3 * 0.5, 3 * 0, 3 * -8.5, 3 * 5.5, fillValue, 3 * -16.5, fillValue, fillValue},
				"rms":       {3 * 1.5, 3 * 0.5, 3 * 8.5, 3 * 5.5, fillValue, 3 * 17.4427633, fillValue, fillValue},
				"var":       {18, 2.25, 0, 0, fillValue, 288, fillValue, fillValue},
				"sd":        {3 * 1.4142135, 3 * 0.5, 3 * 0, 3 * 0, fillValue, 3 * 5.656854, fillValue, fillValue},
				"sumpos":    {3 * 4.5, 3 * 0.5, 3 * 0, 3 * 5.5, fillValue, 3 * 0, fillValue, fillValue},
				"sumneg":    {3 * -2, 3 * -0.5, 3 * -8.5, 3 * 0, fillValue, 3 * -82.5, fillValue, fillValue},
			},
		},
		{
			function_name: "MULTIPLICATION",
			function:      MULTIPLICATION,
			expected: map[string][]float32{
				// Need to_string functions to verify these values
				"min":       {0.5, 0.5, 144.5, 60.4276, fillValue, 144.5, fillValue, fillValue},
				"min_at":    {20, 20, 20, 18, fillValue, 12, fillValue, fillValue},
				"max":       {12.5, 0.5, 144.5, 60.4276, fillValue, 1200.5, fillValue, fillValue},
				"max_at":    {32, 20, 20, 18, fillValue, 28, fillValue, fillValue},
				"maxabs":    {12.5, 0.5, 144.5, 60.4276, fillValue, 1200.5, fillValue, fillValue},
				"maxabs_at": {32, 20, 20, 18, fillValue, 28, fillValue, fillValue},
				"mean":      {4.5, 0.5, 144.5, 60.4276, fillValue, 608.5, fillValue, fillValue},
				"meanabs":   {4.5, 0.5, 144.5, 60.4276, fillValue, 608.5, fillValue, fillValue},
				"meanpos":   {4.5, 0.5, 144.5, 60.4276, fillValue, 608.5, fillValue, fillValue},
				"meanneg":   {0, 0, 0, 0, fillValue, 0, fillValue, fillValue},
				"median":    {4.5, 0.5, 144.5, 60.4276, fillValue, 544.5, fillValue, fillValue},
				"rms":       {6.2809234, 0.5, 144.5, 60.4276, fillValue, 715.913, fillValue, fillValue},
				"var":       {19.2, 0, 0, 0, fillValue, 142259.2, fillValue, fillValue},
				"sd":        {4.3817806, 0, 0, 0, fillValue, 377.17264, fillValue, fillValue},
				"sumpos":    {22.5, 1, 144.5, 60.4276, fillValue, 3042.5, fillValue, fillValue},
				"sumneg":    {0, 0, 0, 0, fillValue, 0, fillValue, fillValue},
			},
		},
		{
			function_name: "DIVISION",
			function:      DIVISION,
			expected: map[string][]float32{
				"min":       {2, 2, 2, 2, fillValue, 2, fillValue, fillValue},
				"min_at":    {16, 20, 20, 18, fillValue, 12, fillValue, fillValue},
				"max":       {2, 2, 2, 2, fillValue, 2, fillValue, fillValue},
				"max_at":    {16, 20, 20, 18, fillValue, 12, fillValue, fillValue},
				"maxabs":    {2, 2, 2, 2, fillValue, 2, fillValue, fillValue},
				"maxabs_at": {16, 20, 20, 18, fillValue, 12, fillValue, fillValue},
				"mean":      {2, 2, 2, 2, fillValue, 2, fillValue, fillValue},
				"meanabs":   {2, 2, 2, 2, fillValue, 2, fillValue, fillValue},
				"meanpos":   {2, 2, 2, 2, fillValue, 2, fillValue, fillValue},
				"meanneg":   {0, 0, 0, 0, fillValue, 0, fillValue, fillValue},
				"median":    {2, 2, 2, 2, fillValue, 2, fillValue, fillValue},
				"rms":       {2, 2, 2, 2, fillValue, 2, fillValue, fillValue},
				"var":       {0, 0, 0, 0, fillValue, 0, fillValue, fillValue},
				"sd":        {0, 0, 0, 0, fillValue, 0, fillValue, fillValue},
				"sumpos":    {10, 4, 2, 2, fillValue, 10, fillValue, fillValue},
				"sumneg":    {0, 0, 0, 0, fillValue, 0, fillValue, fillValue},
			},
		},
	}

	topValues := [][]float32{
		{16, 20},
		{20, 18},
		{14, 12},
		{12, 12}, // Out-of-bounds
	}
	bottomValues := [][]float32{
		{32, 24},
		{20, 18},
		{fillValue, 28},
		{28, 28}, // Out-of-bounds
	}
	const stepsize = float32(4.0)

	topSurface := samples10Surface(topValues)
	bottomSurface := samples10Surface(bottomValues)

	for _, testcase := range testcases {

		targetAttributes := make([]string, 0, len(testcase.expected))
		// targetAttributesNames := make([]string, 0, len(testcase.expected))
		for attr := range testcase.expected {
			fmt.Sprintf("BlobEndpoint=%s", attr)
			// fmt.Sprintf("%s\n", attr)
			targetAttributes = append(targetAttributes, attr)
			//targetAttributesNames = append(targetAttributesNames, attr+" "+testcase.function_name)
		}

		interpolationMethod, _ := GetInterpolationMethod("nearest")

		handle, _ := NewVDSMultiHandle(samples10_2x, samples10, testcase.function)

		defer handle.Close()
		buf, err := handle.GetAttributesBetweenSurfaces(
			topSurface,
			bottomSurface,
			stepsize,
			targetAttributes,
			interpolationMethod,
		)
		require.NoErrorf(t, err, "Failed to calculate attributes, err %v", err)
		require.Len(t, buf, len(targetAttributes),
			"Incorrect number of attributes returned",
		)

		for i, attr := range buf {
			result, err := toFloat32(attr)
			require.NoErrorf(t, err, "Couldn't convert to float32")

			require.InDeltaSlicef(
				t,
				testcase.expected[targetAttributes[i]],
				*result,
				0.000001,
				"[%s]\nExpected: %v\nActual:   %v",
				targetAttributes[i]+" "+testcase.function_name,
				testcase.expected[targetAttributes[i]],
				*result,
			)
		}
	}
}
