// ======================================================================== //
// Copyright 2009-2016 Intel Corporation                                    //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

//ospray
#include "LocalFB.h"
#include "LocalFB_ispc.h"
#include "ospray/common/tasking/parallel_for.h"

// number of floats each task is clearing; must be a a mulitple of 16
#define CLEAR_BLOCK_SIZE (32 * 1024)

namespace ospray {

  LocalFrameBuffer::LocalFrameBuffer(const vec2i &size,
                                     ColorBufferFormat colorBufferFormat,
                                     bool hasDepthBuffer,
                                     bool hasAccumBuffer,
                                     bool hasVarianceBuffer,
                                     void *colorBufferToUse)
    : FrameBuffer(size, colorBufferFormat, hasDepthBuffer, hasAccumBuffer, hasVarianceBuffer)
  {
    Assert(size.x > 0);
    Assert(size.y > 0);
    if (colorBufferToUse)
      colorBuffer = colorBufferToUse;
    else {
      switch(colorBufferFormat) {
      case OSP_RGBA_NONE:
        colorBuffer = NULL;
        break;
      case OSP_RGBA_F32:
        colorBuffer = (vec4f*)alignedMalloc(sizeof(vec4f)*size.x*size.y);
        break;
      case OSP_RGBA_I8:
        colorBuffer = (uint32*)alignedMalloc(sizeof(uint32)*size.x*size.y);
        break;
      default:
        throw std::runtime_error("color buffer format not supported");
      }
    }

    if (hasDepthBuffer)
      depthBuffer = (float*)alignedMalloc(sizeof(float)*size.x*size.y);
    else
      depthBuffer = NULL;

    if (hasAccumBuffer)
      accumBuffer = (vec4f*)alignedMalloc(sizeof(vec4f)*size.x*size.y);
    else
      accumBuffer = NULL;

    tilesx = divRoundUp(size.x, TILE_SIZE);
    tiles = tilesx * divRoundUp(size.y, TILE_SIZE);
    tileAccumID = new int32[tiles];

    if (hasVarianceBuffer) {
      varianceBuffer = (vec4f*)alignedMalloc(sizeof(vec4f)*size.x*size.y);
      tileErrorBuffer = new float[tiles];
    } else {
      varianceBuffer = NULL;
      tileErrorBuffer = NULL;
    }

    ispcEquivalent = ispc::LocalFrameBuffer_create(this,size.x,size.y,
                                                   colorBufferFormat,
                                                   colorBuffer,
                                                   depthBuffer,
                                                   accumBuffer,
                                                   varianceBuffer,
                                                   tileAccumID,
                                                   tileErrorBuffer);
  }

  LocalFrameBuffer::~LocalFrameBuffer()
  {
    alignedFree(depthBuffer);
    alignedFree(colorBuffer);
    alignedFree(accumBuffer);
    alignedFree(varianceBuffer);
    delete[] tileAccumID;
    delete[] tileErrorBuffer;
  }

  std::string LocalFrameBuffer::toString() const
  {
    return "ospray::LocalFrameBuffer";
  }

  void LocalFrameBuffer::clear(const uint32 fbChannelFlags)
  {
    if (fbChannelFlags & OSP_FB_ACCUM) {
      // it is only necessary to reset the accumID,
      // LocalFrameBuffer_accumulateTile takes care of clearing the
      // accumulation buffers
      for (int i = 0; i < tiles; i++)
        tileAccumID[i] = 0;

      // always also also error buffer (if present)
      if (hasVarianceBuffer) {
        for (int i = 0; i < tiles; i++)
          tileErrorBuffer[i] = inf;
      }
    }
  }

  void LocalFrameBuffer::setTile(Tile &tile)
  {
    if (pixelOp)
      pixelOp->preAccum(tile);
    if (accumBuffer)
      ispc::LocalFrameBuffer_accumulateTile(getIE(),(ispc::Tile&)tile);
    if (pixelOp)
      pixelOp->postAccum(tile);
    if (colorBuffer) {
      switch(colorBufferFormat) {
      case OSP_RGBA_I8:
        ispc::LocalFrameBuffer_writeTile_RGBA_I8(getIE(),(ispc::Tile&)tile);
        break;
      case OSP_RGBA_F32:
        ispc::LocalFrameBuffer_writeTile_RGBA_F32(getIE(),(ispc::Tile&)tile);
        break;
      default:
        NOTIMPLEMENTED;
      }
    }
  }

  int32 LocalFrameBuffer::accumID(const vec2i &tile)
  {
    return tileAccumID[tile.y * tilesx + tile.x];
  }

  float LocalFrameBuffer::tileError(const vec2i &tile)
  {
    int idx = tile.y * tilesx + tile.x;
    return hasVarianceBuffer && tileAccumID[idx] > 1 ? tileErrorBuffer[idx] : inf;
  }

  float LocalFrameBuffer::frameError()
  {
    if (hasVarianceBuffer) {
      float maxErr = 0.0f;
      for (int i = 0; i < tiles; i++)
        maxErr = tileAccumID[i] > 1 ? std::max(maxErr, tileErrorBuffer[i]) : inf;
      return maxErr;
    } else
      return inf;
  }

  const void *LocalFrameBuffer::mapDepthBuffer()
  {
    this->refInc();
    return (const void *)depthBuffer;
  }

  const void *LocalFrameBuffer::mapColorBuffer()
  {
    this->refInc();
    return (const void *)colorBuffer;
  }

  void LocalFrameBuffer::unmap(const void *mappedMem)
  {
    Assert(mappedMem == colorBuffer || mappedMem == depthBuffer );
    this->refDec();
  }

} // ::ospray
