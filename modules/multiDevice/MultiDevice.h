// Copyright 2009 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <unordered_set>
#include <vector>
#include "ISPCDevice.h"
#include "MultiDeviceLoadBalancer.h"
#include "MultiDeviceObject.h"
#include "MultiDeviceRenderTask.h"

// MultiDevice.h Implements the a multiplexor for other OSPRay devices which
// this simply farms work out to in an image parallel fashion.

namespace ospray {
namespace api {

struct MultiDevice : public Device
{
  MultiDevice() = default;

  /////////////////////////////////////////////////////////////////////////
  // ManagedObject Implementation /////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  void commit() override;

  /////////////////////////////////////////////////////////////////////////
  // Device Implementation ////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  int loadModule(const char *name) override;

  // OSPRay Data Arrays ///////////////////////////////////////////////////

  OSPData newSharedData(const void *sharedData,
      OSPDataType,
      const vec3ul &numItems,
      const vec3l &byteStride) override;

  OSPData newData(OSPDataType, const vec3ul &numItems) override;

  void copyData(const OSPData source,
      OSPData destination,
      const vec3ul &destinationIndex) override;

  // Renderable Objects ///////////////////////////////////////////////////

  OSPLight newLight(const char *type) override;

  OSPCamera newCamera(const char *type) override;

  OSPGeometry newGeometry(const char *type) override;
  OSPVolume newVolume(const char *type) override;

  OSPGeometricModel newGeometricModel(OSPGeometry geom) override;
  OSPVolumetricModel newVolumetricModel(OSPVolume volume) override;

  // Model Meta-Data //////////////////////////////////////////////////////

  OSPMaterial newMaterial(
      const char * /*ignored*/, const char *material_type) override;

  OSPTransferFunction newTransferFunction(const char *type) override;

  OSPTexture newTexture(const char *type) override;

  // Instancing ///////////////////////////////////////////////////////////

  OSPGroup newGroup() override;
  OSPInstance newInstance(OSPGroup group) override;

  // Top-level Worlds /////////////////////////////////////////////////////

  OSPWorld newWorld() override;
  box3f getBounds(OSPObject) override;

  // Object + Parameter Lifetime Management ///////////////////////////////

  void setObjectParam(OSPObject object,
      const char *name,
      OSPDataType type,
      const void *mem) override;

  void removeObjectParam(OSPObject object, const char *name) override;

  void commit(OSPObject object) override;
  void release(OSPObject _obj) override;
  void retain(OSPObject _obj) override;

  // FrameBuffer Manipulation /////////////////////////////////////////////

  OSPFrameBuffer frameBufferCreate(const vec2i &size,
      const OSPFrameBufferFormat mode,
      const uint32 channels) override;

  OSPImageOperation newImageOp(const char *type) override;

  const void *frameBufferMap(
      OSPFrameBuffer fb, const OSPFrameBufferChannel) override;

  void frameBufferUnmap(const void *mapped, OSPFrameBuffer fb) override;

  float getVariance(OSPFrameBuffer) override;

  void resetAccumulation(OSPFrameBuffer _fb) override;

  // Frame Rendering //////////////////////////////////////////////////////

  OSPRenderer newRenderer(const char *type) override;

  OSPFuture renderFrame(
      OSPFrameBuffer, OSPRenderer, OSPCamera, OSPWorld) override;

  int isReady(OSPFuture, OSPSyncEvent) override;
  void wait(OSPFuture, OSPSyncEvent) override;
  void cancel(OSPFuture) override;
  float getProgress(OSPFuture) override;
  float getTaskDuration(OSPFuture) override;

  OSPPickResult pick(
      OSPFrameBuffer, OSPRenderer, OSPCamera, OSPWorld, const vec2f &) override;

 private:
  std::unique_ptr<MultiDeviceLoadBalancer> loadBalancer;
  ISPCDevice hostDevice;
  std::vector<std::unique_ptr<ISPCDevice>> subdevices;
};

extern "C" OSPError OSPRAY_DLLEXPORT ospray_module_init_multi(
    int16_t versionMajor, int16_t versionMinor, int16_t /*versionPatch*/);

} // namespace api
} // namespace ospray
