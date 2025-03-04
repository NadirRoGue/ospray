// Copyright 2009 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

// ospray
#include "DebugRenderer.h"
#include "camera/Camera.h"
#include "common/World.h"
#include "fb/FrameBuffer.h"
#ifndef OSPRAY_TARGET_SYCL
// ispc exports
#include "render/debug/DebugRenderer_ispc.h"
#else
#include "DebugRenderer.ih"
#endif

namespace ospray {

// Helper functions /////////////////////////////////////////////////////////

static ispc::DebugRendererType typeFromString(const std::string &name)
{
  if (name == "rayDir")
    return ispc::DebugRendererType::RAY_DIR;
  else if (name == "eyeLight")
    return ispc::DebugRendererType::EYE_LIGHT;
  else if (name == "Ng")
    return ispc::DebugRendererType::NG;
  else if (name == "Ns")
    return ispc::DebugRendererType::NS;
  else if (name == "color")
    return ispc::DebugRendererType::COLOR;
  else if (name == "texCoord")
    return ispc::DebugRendererType::TEX_COORD;
  else if (name == "backfacing_Ng")
    return ispc::DebugRendererType::BACKFACING_NG;
  else if (name == "backfacing_Ns")
    return ispc::DebugRendererType::BACKFACING_NS;
  else if (name == "dPds")
    return ispc::DebugRendererType::DPDS;
  else if (name == "dPdt")
    return ispc::DebugRendererType::DPDT;
  else if (name == "primID")
    return ispc::DebugRendererType::PRIM_ID;
  else if (name == "geomID")
    return ispc::DebugRendererType::GEOM_ID;
  else if (name == "instID")
    return ispc::DebugRendererType::INST_ID;
  else if (name == "volume")
    return ispc::DebugRendererType::VOLUME;
  else
    return ispc::DebugRendererType::TEST_FRAME;
}

// DebugRenderer definitions ////////////////////////////////////////////////

DebugRenderer::DebugRenderer(api::ISPCDevice &device)
    : AddStructShared(device.getIspcrtDevice(), device)
{}

std::string DebugRenderer::toString() const
{
  return "ospray::DebugRenderer";
}

void DebugRenderer::commit()
{
  Renderer::commit();

  const std::string method = getParam<std::string>("method", "eyeLight");
  getSh()->type = typeFromString(method);
}

void DebugRenderer::renderTasks(FrameBuffer *fb,
    Camera *camera,
    World *world,
    void *perFrameData,
    const utility::ArrayView<uint32_t> &taskIDs
#ifdef OSPRAY_TARGET_SYCL
    ,
    sycl::queue &syclQueue
#endif
) const
{
  auto *rendererSh = getSh();
  auto *fbSh = fb->getSh();
  auto *cameraSh = camera->getSh();
  auto *worldSh = world->getSh();
  const size_t numTasks = taskIDs.size();

#ifdef OSPRAY_TARGET_SYCL
  const uint32_t *taskIDsPtr = taskIDs.data();
  auto event = syclQueue.submit([&](sycl::handler &cgh) {
    const sycl::nd_range<1> dispatchRange = computeDispatchRange(numTasks, 16);
    cgh.parallel_for(dispatchRange, [=](sycl::nd_item<1> taskIndex) {
      if (taskIndex.get_global_id(0) < numTasks) {
        ispc::DebugRenderer_renderTask(&rendererSh->super,
            fbSh,
            cameraSh,
            worldSh,
            perFrameData,
            taskIDsPtr,
            taskIndex.get_global_id(0));
      }
    });
  });
  event.wait_and_throw();
  // For prints we have to flush the entire queue, because other stuff is queued
  syclQueue.wait_and_throw();
#else
  ispc::DebugRenderer_renderTasks(&rendererSh->super,
      fbSh,
      cameraSh,
      worldSh,
      perFrameData,
      taskIDs.data(),
      numTasks);
#endif
}

} // namespace ospray
