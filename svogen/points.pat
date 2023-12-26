// ImHex pattern for *-points.bin

#include <std/io.pat>

using Float3;
fn f3_format(Float3 f3) {
  return std::format("({}; {}; {})", f3.x, f3.y, f3.z);
};

struct Float3 { float x, y, z; } [[format("f3_format")]];

struct Data {
  Float3 min, max;
  u32 n_points;
  Float3 points[n_points];
};

Data data @ 0x00;
