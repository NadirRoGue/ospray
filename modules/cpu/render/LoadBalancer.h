// Copyright 2009 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "camera/Camera.h"
#include "common/OSPCommon.h"
#include "common/World.h"
#include "fb/FrameBuffer.h"
#include "render/Renderer.h"
#include "rkcommon/utility/ArrayView.h"

namespace ospray {

struct OSPRAY_SDK_INTERFACE TiledLoadBalancer
{
  virtual ~TiledLoadBalancer() = default;

  virtual std::string toString() const = 0;

  /*! Render the entire framebuffer using the given renderer, camera and
   * world configuration using the load balancer to parallelize the work
   */
  virtual void renderFrame(
      FrameBuffer *fb, Renderer *renderer, Camera *camera, World *world) = 0;
};

// Inlined definitions //////////////////////////////////////////////////////

//! tiled load balancer for local rendering on the given machine
/*! a tiled load balancer that orchestrates (multi-threaded)
  rendering on a local machine, without any cross-node
  communication/load balancing at all (even if there are multiple
  application ranks each doing local rendering on their own)  */
struct OSPRAY_SDK_INTERFACE LocalTiledLoadBalancer : public TiledLoadBalancer
{
  void renderFrame(FrameBuffer *fb,
      Renderer *renderer,
      Camera *camera,
      World *world) override;

  std::string toString() const override;

#ifdef OSPRAY_TARGET_SYCL
  void setQueue(sycl::queue *syclQueue);

 private:
  sycl::queue *syclQueue = nullptr;
#endif
};

} // namespace ospray
