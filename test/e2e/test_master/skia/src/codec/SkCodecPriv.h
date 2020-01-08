/*
 * Copyright 2015 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkCodecPriv_DEFINED
#  define SkCodecPriv_DEFINED
#  include "include/codec/SkEncodedOrigin.h"
#  include "include/core/SkImageInfo.h"
#  include "include/core/SkTypes.h"
#  include "include/private/SkColorData.h"
#  include "include/private/SkEncodedInfo.h"
#  include "src/codec/SkColorTable.h"
#  ifdef SK_PRINT_CODEC_MESSAGES
#    define SkCodecPrintf	SkDebugf
#  else 
#    define SkCodecPrintf	(...)
#  endif
bool sk_select_xform_format(SkColorType colorType, bool forColorTable, skcms_PixelFormat* outFormat);
// FIXME: Consider sharing with dm, nanbench, and tools.
static float get_scale_from_sample_size(int sampleSize)
{
  return 1.0f / ((float) sampleSize);
}
static bool is_valid_subset(const SkIRect& subset, const SkISize& imageDims)
{
  return SkIRect::MakeSize(imageDims).contains(subset);
}
static int get_scaled_dimension(int srcDimension, int sampleSize)
{
  if (sampleSize > srcDimension)
  {
    return 1;
  }
  return srcDimension / sampleSize;
}
static int get_start_coord(int sampleFactor)
{
  return sampleFactor / 2;
}
/*
 * Given a coordinate in the original image, this returns the corresponding
 * coordinate in the scaled image.  This function is meaningless if
 * IsCoordNecessary returns false.
 * The output can be interpreted as an x-coordinate or a y-coordinate.
 *
 * This does not need to be called and is not called when sampleFactor == 1.
 */
static int get_dst_coord(int srcCoord, int sampleFactor)
{
  return srcCoord / sampleFactor;
}
/*
 * When scaling, we will discard certain y-coordinates (rows) and
 * x-coordinates (columns).  This function returns true if we should keep the
 * coordinate and false otherwise.
 * The inputs may be x-coordinates or y-coordinates.
 *
 * This does not need to be called and is not called when sampleFactor == 1.
 */
static bool is_coord_necessary(int srcCoord, int sampleFactor, int scaledDim)
{
    // Get the first coordinate that we want to keep
  int startCoord = get_start_coord(sampleFactor);
    // Return false on edge cases
  if (srcCoord < startCoord || get_dst_coord(srcCoord, sampleFactor) >= scaledDim)
  {
    return false;
  }
    // Every sampleFactor rows are necessary
  return ((srcCoord - startCoord) % sampleFactor) == 0;
}
static bool valid_alpha(SkAlphaType dstAlpha, bool srcIsOpaque)
{
  if (kUnknown_SkAlphaType == dstAlpha)
  {
    return false;
  }
  if (srcIsOpaque)
  {
    if (kOpaque_SkAlphaType != dstAlpha)
    {
      SkCodecPrintf("Warning: an opaque image should be decoded as opaque "
                          "- it is being decoded as non-opaque, which will draw slower\n");
    }
    return true;
  }
  return dstAlpha != kOpaque_SkAlphaType;
}
static const SkPMColor* get_color_ptr(SkColorTable* colorTable)
{
  return nullptr != colorTable ? colorTable->readColors() : nullptr;
}
static size_t compute_row_bytes_ppb(int width, uint32_t pixelsPerByte)
{
  return (width + pixelsPerByte - 1) / pixelsPerByte;
}
static size_t compute_row_bytes_bpp(int width, uint32_t bytesPerPixel)
{
  return width * bytesPerPixel;
}
static size_t compute_row_bytes(int width, uint32_t bitsPerPixel)
{
  if (bitsPerPixel < 16)
  {
    SkASSERT(0 == 8 % bitsPerPixel);
    const uint32_t pixelsPerByte = 8 / bitsPerPixel;
    return compute_row_bytes_ppb(width, pixelsPerByte);
  }
  else 
  {
    SkASSERT(0 == bitsPerPixel % 8);
    const uint32_t bytesPerPixel = bitsPerPixel / 8;
    return compute_row_bytes_bpp(width, bytesPerPixel);
  }
}
static uint8_t get_byte(uint8_t* buffer, uint32_t i)
{
  return buffer[i];
}
static uint16_t get_short(uint8_t* buffer, uint32_t i)
{
  uint16_t result;
  memcpy(&result, &(buffer[i]), 2);
#  ifdef SK_CPU_BENDIAN
  return SkEndianSwap16(result);
#  else 
  return result;
#  endif
}
static uint32_t get_int(uint8_t* buffer, uint32_t i)
{
  uint32_t result;
  memcpy(&result, &(buffer[i]), 4);
#  ifdef SK_CPU_BENDIAN
  return SkEndianSwap32(result);
#  else 
  return result;
#  endif
}
static bool is_valid_endian_marker(const uint8_t* data, bool* isLittleEndian)
{
    // II indicates Intel (little endian) and MM indicates motorola (big endian).
  if (('I' != data[0] || 'I' != data[1]) && ('M' != data[0] || 'M' != data[1]))
  {
    return false;
  }
  *isLittleEndian = ('I' == data[0]);
  return true;
}
static uint16_t get_endian_short(const uint8_t* data, bool littleEndian)
{
  if (littleEndian)
  {
    return (data[1] << 8) | (data[0]);
  }
  return (data[0] << 8) | (data[1]);
}
static SkPMColor premultiply_argb_as_rgba(U8CPU a, U8CPU r, U8CPU g, U8CPU b)
{
  if (a != 255)
  {
    r = SkMulDiv255Round(r, a);
    g = SkMulDiv255Round(g, a);
    b = SkMulDiv255Round(b, a);
  }
  return SkPackARGB_as_RGBA(a, r, g, b);
}
static SkPMColor premultiply_argb_as_bgra(U8CPU a, U8CPU r, U8CPU g, U8CPU b)
{
  if (a != 255)
  {
    r = SkMulDiv255Round(r, a);
    g = SkMulDiv255Round(g, a);
    b = SkMulDiv255Round(b, a);
  }
  return SkPackARGB_as_BGRA(a, r, g, b);
}
static bool is_rgba(SkColorType colorType)
{
#  ifdef SK_PMCOLOR_IS_RGBA
  return (kBGRA_8888_SkColorType != colorType);
#  else 
  return (kRGBA_8888_SkColorType == colorType);
#  endif
}
typedef uint32_t (*PackColorProc) (U8CPU a, U8CPU r, U8CPU g, U8CPU b);
static PackColorProc choose_pack_color_proc(bool isPremul, SkColorType colorType)
{
  bool isRGBA = is_rgba(colorType);
  if (isPremul)
  {
    if (isRGBA)
    {
      return &premultiply_argb_as_rgba;
    }
    else 
    {
      return &premultiply_argb_as_bgra;
    }
  }
  else 
  {
    if (isRGBA)
    {
      return &SkPackARGB_as_RGBA;
    }
    else 
    {
      return &SkPackARGB_as_BGRA;
    }
  }
}
bool is_orientation_marker(const uint8_t* data, size_t data_length, SkEncodedOrigin* orientation);
#endif