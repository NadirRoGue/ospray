// Copyright 2009 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "OBJ.h"
#ifndef OSPRAY_TARGET_SYCL
// ispc
#include "render/materials/OBJ_ispc.h"
#endif

namespace ospray {
namespace pathtracer {

OBJMaterial::OBJMaterial(api::ISPCDevice &device)
    : AddStructShared(device.getIspcrtDevice(), device)
{
#ifndef OSPRAY_TARGET_SYCL
  getSh()->super.getBSDF =
      reinterpret_cast<ispc::Material_GetBSDFFunc>(ispc::OBJ_getBSDF_addr());
  getSh()->super.getTransparency =
      reinterpret_cast<ispc::Material_GetTransparencyFunc>(
          ispc::OBJ_getTransparency_addr());
#endif
}

std::string OBJMaterial::toString() const
{
  return "ospray::pathtracer::obj";
}

void OBJMaterial::commit()
{
  MaterialParam1f d = getMaterialParam1f("d", 1.f);
  MaterialParam3f Kd = getMaterialParam3f("kd", vec3f(0.8f));
  MaterialParam3f Ks = getMaterialParam3f("ks", vec3f(0.f));
  MaterialParam1f Ns = getMaterialParam1f("ns", 10.f);

  vec3f Tf = getParam<vec3f>("tf", vec3f(0.f));
  ispc::TextureParam bumpTex = getTextureParam("map_bump");
  linear2f bumpRot =
      ((linear2f *)(&bumpTex.xform2f.l))->orthogonal().transposed();

  const float color_total = reduce_max(Kd.factor + Ks.factor + Tf);
  if (color_total > 1.f) {
    postStatusMsg(OSP_LOG_DEBUG) << "#osp:PT OBJ material produces energy "
                                 << "(kd + ks + tf = " << color_total
                                 << ", should be <= 1). Scaling down to 1.";
    Kd.factor /= color_total;
    Ks.factor /= color_total;
    Tf /= color_total;
  }

  getSh()->dMap = d.tex;
  getSh()->d = d.factor;
  getSh()->KdMap = Kd.tex;
  getSh()->Kd = Kd.factor;
  getSh()->KsMap = Ks.tex;
  getSh()->Ks = Ks.factor;
  getSh()->NsMap = Ns.tex;
  getSh()->Ns = Ns.factor;
  getSh()->Tf = Tf;
  getSh()->bumpMap = bumpTex;
  getSh()->bumpRot = bumpRot;
}

} // namespace pathtracer
} // namespace ospray
