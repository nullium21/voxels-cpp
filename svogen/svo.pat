#pragma pattern_limit 1048576

#include <std/io.pat>

bitfield SvoNodeType {
  is_filled : 1;
  sign_x : 1;
  sign_y : 1;
  sign_z : 1;
  _pad : 4 [[hidden]];
};

using Float3;
fn f3_format(Float3 f3) {
  return std::format("({}; {}; {})", f3.x, f3.y, f3.z);
};

struct Float3 { float x, y, z; } [[format("f3_format")]];

struct SvoNode {
  SvoNodeType type;
  u8 _pad[3] [[hidden]];
  s32 prev, next;
  s32 child;
};

struct Data {
  Float3 min_coords, max_coords;
  u32 n_subdiv;
  u32 n_nodes;
  SvoNode nodes[n_nodes];
};

Data data @ 0x00;
