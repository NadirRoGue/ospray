// Copyright 2022 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "Distribution2D.h"
#include "common/BufferShared.h"

namespace ospray {
Distribution2D::Distribution2D(const vec2i &size, api::ISPCDevice &device)
    : AddStructShared(device.getIspcrtDevice(), device)
{
  getSh()->size = size;
  getSh()->rcpSize = vec2f(1.f / size.x, 1.f / size.y);

  cdf_x = make_buffer_shared_unique<float>(
      device.getIspcrtDevice(), size.x * size.y);
  cdf_y = make_buffer_shared_unique<float>(device.getIspcrtDevice(), size.y);

  getSh()->cdf_x = cdf_x->data();
  getSh()->cdf_y = cdf_y->data();
}
} // namespace ospray
