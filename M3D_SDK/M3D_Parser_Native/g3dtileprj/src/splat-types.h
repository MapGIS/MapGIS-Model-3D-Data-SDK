#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <vector>

#include "g3dtilegaussian.h"

namespace spz {

typedef struct {
	size_t count;
	float *data;
} SpzFloatBuffer;

typedef struct {
	int32_t numPoints;
	int32_t shDegree;
	bool antialiased;
	SpzFloatBuffer positions;
	SpzFloatBuffer scales;
	SpzFloatBuffer rotations;
	SpzFloatBuffer alphas;
	SpzFloatBuffer colors;
	SpzFloatBuffer sh;
} GaussianCloudData;


inline SpzFloatBuffer copyFloatBuffer(const std::vector<float> &vector) {
  SpzFloatBuffer buffer = {0, nullptr};
  if (!vector.empty()) {
    buffer.count = vector.size();
    buffer.data = new float[buffer.count];
    std::memcpy(buffer.data, vector.data(), buffer.count * sizeof(float));
  }
  return buffer;
}

// A point cloud composed of Gaussians. Each gaussian is represented by:
//   - xyz position
//   - xyz scales (on log scale, compute exp(x) to get scale factor)
//   - xyzw quaternion
//   - alpha (before sigmoid activation, compute sigmoid(a) to get alpha value between 0 and 1)
//   - rgb color (as SH DC component, compute 0.5 + 0.282095 * x to get color value between 0 and 1)
//   - 0 to 45 spherical harmonics coefficients (see comment below)
struct GaussianCloud {
  // Total number of points (gaussians) in this splat.
  int32_t numPoints = 0;

  // Degree of spherical harmonics for this splat.
  int32_t shDegree = 0;

  // Whether the gaussians should be rendered in antialiased mode (mip splatting)
  bool antialiased = false;

  // See block comment above for details
  std::vector<float> positions;
  std::vector<float> scales;
  std::vector<float> rotations;
  std::vector<float> alphas;
  std::vector<float> colors;

  // Spherical harmonics coefficients. The number of coefficients per point depends on shDegree:
  //   0 -> 0
  //   1 -> 9   (3 coeffs x 3 channels)
  //   2 -> 24  (8 coeffs x 3 channels)
  //   3 -> 45  (15 coeffs x 3 channels)
  // The color channel is the inner (fastest varying) axis, and the coefficient is the outer
  // (slower varying) axis, i.e. for degree 1, the order of the 9 values is:
  //   sh1n1_r, sh1n1_g, sh1n1_b, sh10_r, sh10_g, sh10_b, sh1p1_r, sh1p1_g, sh1p1_b
  std::vector<float> sh;

  // The caller is responsible for freeing the pointers in the returned GaussianCloudData
  GaussianCloudData data() const {
    GaussianCloudData data;
    data.numPoints = numPoints;
    data.shDegree = shDegree;
    data.antialiased = antialiased;
    data.positions = copyFloatBuffer(positions);
    data.scales = copyFloatBuffer(scales);
    data.rotations = copyFloatBuffer(rotations);
    data.alphas = copyFloatBuffer(alphas);
    data.colors = copyFloatBuffer(colors);
    data.sh = copyFloatBuffer(sh);
    return data;
  }

  // Convert between two coordinate systems, for example from RDF (ply format) to RUB (used by spz).
  // This is performed in-place.
  void convertCoordinates(MapGIS::Tile::CoordinateSystem from, MapGIS::Tile::CoordinateSystem to) {
	  if (numPoints == 0) {
		  // There is nothing to convert.
		  return;
	  }
	  
	  MapGIS::Tile::CoordinateSystemConverter converter = MapGIS::Tile::CGGaussianDataTrans::GetCoordinateSystemConverter(from, to);

	  for (size_t i = 0; i < positions.size(); i += 3)
	  {
		  std::array<float, 3> rtn = MapGIS::Tile::CGGaussianDataTrans::TransPoints({ positions[i + 0] ,positions[i + 1] ,positions[i + 2] }, converter.match);
		  positions[i + 0] = rtn[0];
		  positions[i + 1] = rtn[1];
		  positions[i + 2] = rtn[2];
	  }


	  for (size_t i = 0; i < rotations.size(); i += 4)
	  {
		  std::array<float, 4> rtn = MapGIS::Tile::CGGaussianDataTrans::TransRotations({ rotations[i + 0] ,rotations[i + 1] ,rotations[i + 2],rotations[i + 3] }, converter.match);
		  rotations[i + 0] = rtn[0];
		  rotations[i + 1] = rtn[1];
		  rotations[i + 2] = rtn[2];
		  rotations[i + 3] = rtn[3];
	  }
	  for (size_t i = 0; i < scales.size(); i += 3)
	  {

		  std::array<float, 3> rtn = MapGIS::Tile::CGGaussianDataTrans::TransScale({ scales[i + 0] ,scales[i + 1] ,scales[i + 2] }, converter.match);
		  scales[i + 0] = rtn[0];
		  scales[i + 1] = rtn[1];
		  scales[i + 2] = rtn[2];
	  }

	  const size_t num = sh.size() / numPoints;
	  std::vector<float> shtemp;
	  std::vector<float> shNew;
	  shtemp.reserve(num);
	  for (int i = 0; i < numPoints; i++)
	  {
		  shtemp.clear();
		  shtemp.insert(shtemp.begin(), sh.begin() + i *num, sh.begin() + (i + 1)*num);

		  std::vector<float> rtn = MapGIS::Tile::CGGaussianDataTrans::TransSh(shtemp, converter.match);

		  shNew.insert(shNew.end(), rtn.begin(), rtn.end());
	  }
	  sh.swap(shNew);
  }

  float medianVolume() const {
    if (numPoints == 0) {
      return 0.01f;
    }
    // The volume of an ellipsoid is 4/3 * pi * x * y * z, where x, y, and z are the radii on each
    // axis. Scales are stored on a log scale, and exp(x) * exp(y) * exp(z) = exp(x + y + z). So we
    // can sort by value = (x + y + z) and compute volume = 4/3 * pi * exp(value) later.
    std::vector<float> scaleSums;
    for (int32_t i = 0; i < scales.size(); i += 3) {
      float sum = scales[i] + scales[i + 1] + scales[i + 2];
      scaleSums.push_back(sum);
    }
    std::sort(scaleSums.begin(), scaleSums.end());
    float median = scaleSums[(int32_t)(scaleSums.size() / 2)];
    return (PI * 4 / 3) * exp(median);
  }
};

// SPZ Splat math helpers, lightweight implementations of vector and quaternion math.
using Vec3f = std::array<float, 3>;   // x, y, z
using Quat4f = std::array<float, 4>;  // w, x, y, z
using Half = uint16_t;

// Half-precision helpers.
float halfToFloat(Half h);
Half floatToHalf(float f);

// Vector helpers.
Vec3f normalized(const Vec3f &v);
float norm(const Vec3f &a);

// Quaternion helpers.
float norm(const Quat4f &q);
Quat4f normalized(const Quat4f &v);
Quat4f axisAngleQuat(const Vec3f &scaledAxis);

// Constexpr helpers.
constexpr Vec3f vec3f(const float *data) { return {data[0], data[1], data[2]}; }

constexpr float dot(const Vec3f &a, const Vec3f &b) {
  return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

constexpr float squaredNorm(const Vec3f &v) { return dot(v, v); }

constexpr Quat4f quat4f(const float *data) { return {data[0], data[1], data[2], data[3]}; }

inline Vec3f times(const Quat4f &q, const Vec3f &p) {
  float x2 = q[1] + q[1];
  float y2 = q[2] + q[2];
  float z2 = q[3] + q[3];
  float wx2 = q[0] * x2;
  float wy2 = q[0] * y2;
  float wz2 = q[0] * z2;
  float xx2 = q[1] * x2;
  float xy2 = q[1] * y2;
  float xz2 = q[1] * z2;
  float yy2 = q[2] * y2;
  float yz2 = q[2] * z2;
  float zz2 = q[3]* z2;
  return {
    p[0] * (1.0f - (yy2 + zz2)) + p[1] * (xy2 - wz2) + p[2] * (xz2 + wy2),
    p[0] * (xy2 + wz2) + p[1] * (1.0f - (xx2 + zz2)) + p[2] * (yz2 - wx2),
    p[0] * (xz2 - wy2) + p[1] * (yz2 + wx2) + p[2] * (1.0f - (xx2 + yy2))};
}

inline Quat4f times(const Quat4f &a, const Quat4f &b) {
  return normalized(std::array<float, 4>{
    a[0] * b[0] - a[1] * b[1] - a[2] * b[2] - a[3] * b[3],
    a[0] * b[1] + a[1] * b[0] + a[2] * b[3] - a[3] * b[2],
    a[0] * b[2] - a[1] * b[3] + a[2] * b[0] + a[3] * b[1],
    a[0] * b[3] + a[1] * b[2] - a[2] * b[1] + a[3] * b[0]});
}

constexpr Quat4f times(const Quat4f &a, float s) {
  return {a[0] * s, a[1] * s, a[2] * s, a[3] * s};
}

constexpr Quat4f plus(const Quat4f &a, const Quat4f &b) {
  return {a[0] + b[0], a[1] + b[1], a[2] + b[2], a[3] + b[3]};
}

constexpr Vec3f times(const Vec3f &v, float s) { return {v[0] * s, v[1] * s, v[2] * s}; }

constexpr Vec3f plus(const Vec3f &a, const Vec3f &b) {
  return {a[0] + b[0], a[1] + b[1], a[2] + b[2]};
}

constexpr Vec3f times(const Vec3f &a, const Vec3f &b) {
  return {a[0] * b[0], a[1] * b[1], a[2] * b[2]};
}

}  // namespace spz
