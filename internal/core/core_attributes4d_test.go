package core

import (
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

func TestCubeFunctionsForAttribute4D(t *testing.T) {
	testcases := []struct {
		function int
		expected []float32
	}{
		{
			function: SUBTRACT,
			expected: []float32{
				fillValue, fillValue, fillValue, fillValue, fillValue, fillValue, fillValue,
				fillValue, fillValue, -12.5 - (-12.5), fillValue, 4.5 - (4.5), fillValue, 1.5 - (1.5),
				fillValue, fillValue, 12.5 - (12.5), fillValue, -10.5 - (-10.5), fillValue, -1.5 - (-1.5),
			},
		},
		{
			function: ADDITION,
			expected: []float32{
				fillValue, fillValue, fillValue, fillValue, fillValue, fillValue, fillValue,
				fillValue, fillValue, -12.5 + -12.5, fillValue, 4.5 + 4.5, fillValue, 1.5 + 1.5,
				fillValue, fillValue, 12.5 + 12.5, fillValue, -10.5 + -10.5, fillValue, -1.5 + -1.5,
			},
		},
		{
			function: MULTIPLICATION,
			expected: []float32{
				fillValue, fillValue, fillValue, fillValue, fillValue, fillValue, fillValue,
				fillValue, fillValue, -12.5 * -12.5, fillValue, 4.5 * 4.5, fillValue, 1.5 * 1.5,
				fillValue, fillValue, 12.5 * 12.5, fillValue, -10.5 * -10.5, fillValue, -1.5 * -1.5,
			},
		},
		{
			function: DIVISION,
			expected: []float32{
				fillValue, fillValue, fillValue, fillValue, fillValue, fillValue, fillValue,
				fillValue, fillValue, -12.5 / (-12.5), fillValue, 4.5 / (4.5), fillValue, 1.5 / (1.5),
				fillValue, fillValue, 12.5 / (12.5), fillValue, -10.5 / (-10.5), fillValue, -1.5 / (-1.5),
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

		handle, _ := NewVDSMultiHandle(samples10, samples10, testcase.function)

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
