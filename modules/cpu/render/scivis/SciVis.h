// Copyright 2020 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

// ospray
#include "render/Renderer.h"
// ispc shared
#include "SciVisShared.h"

namespace ospray {

struct SciVis : public AddStructShared<Renderer, ispc::SciVis>
{
  SciVis(api::ISPCDevice &device);
  std::string toString() const override;
  void commit() override;
  void *beginFrame(FrameBuffer *, World *) override;

  virtual void renderTasks(FrameBuffer *fb,
      Camera *camera,
      World *world,
      void *perFrameData,
      const utility::ArrayView<uint32_t> &taskIDs
#ifdef OSPRAY_TARGET_SYCL
      ,
      sycl::queue &syclQueue
#endif
  ) const override;
};

} // namespace ospray
