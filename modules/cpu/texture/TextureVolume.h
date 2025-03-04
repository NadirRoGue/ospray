// Copyright 2009 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
#ifdef OSPRAY_ENABLE_VOLUMES

#pragma once

#include "Texture.h"
#include "volume/Volume.h"
#include "volume/VolumetricModel.h"
#include "volume/transferFunction/TransferFunction.h"
// ispc shared
#include "TextureVolumeShared.h"

namespace ospray {

// A Texture defined through a volume
struct OSPRAY_SDK_INTERFACE TextureVolume
    : public AddStructShared<Texture, ispc::TextureVolume>
{
  TextureVolume(api::ISPCDevice &device)
      : AddStructShared(device.getIspcrtDevice(), device)
  {}

  virtual std::string toString() const override;

  virtual void commit() override;

 private:
  Ref<Volume> volume;
  Ref<TransferFunction> transferFunction;

  // Deprecated interface.
  Ref<VolumetricModel> volumetricModel;
};

} // namespace ospray
#endif
