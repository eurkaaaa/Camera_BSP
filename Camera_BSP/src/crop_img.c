 /**
 ******************************************************************************
 * @file    crop_img.c
 * @author  GPM Application Team
 *
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
#include "crop_img.h"
#include <assert.h>

void img_crop(uint8_t *src_image, uint8_t *dst_img, const uint32_t src_stride,
              const uint16_t dst_width, const uint16_t height,
              const uint16_t dst_bpp)
{
  /* This interface has no source height. Use the current 320x240 RGB565
   * camera frame; the existing caller passes a pointer inside its crop.
   * Keep these dimensions in sync with FRAME_W / FRAME_H in main.h.
   */
  extern uint8_t frame_buffer0[];
  const uint32_t frame_width = 320U;
  const uint32_t frame_height = 240U;
  const uint32_t frame_bytes = frame_width * frame_height * 2U;
  const uintptr_t frame_address = (uintptr_t)frame_buffer0;
  const uintptr_t source_address = (uintptr_t)src_image;

  if ((src_image == NULL) || (dst_img == NULL) ||
      (dst_width == 0U) || (height == 0U) || (dst_bpp != 2U) ||
      (src_stride != frame_width * 2U) ||
      (source_address < frame_address) ||
      (source_address - frame_address >= frame_bytes) ||
      ((uint64_t)dst_width * height * dst_bpp > SIZE_MAX))
  {
    return;
  }

  /* Recover the whole frame, including the borders outside crop_start. */
  const uint8_t *pIn = frame_buffer0;
  const uint32_t x_step = (frame_width << 16) / dst_width;
  const uint32_t y_step = (frame_height << 16) / height;

  for (uint32_t y = 0U; y < height; ++y)
  {
    /* Map pixel centers with Q16 coordinates and clamp at image edges. */
    int32_t sy = (int32_t)(y * y_step + y_step / 2U) - 32768;
    if (sy < 0) sy = 0;
    if ((uint32_t)sy > ((frame_height - 1U) << 16))
      sy = (int32_t)((frame_height - 1U) << 16);

    const uint32_t y0 = (uint32_t)sy >> 16;
    const uint32_t y1 = (y0 + 1U < frame_height) ? y0 + 1U : y0;
    const uint32_t fy = ((uint32_t)sy >> 8) & 255U;
    const uint8_t *row0 = pIn + y0 * src_stride;
    const uint8_t *row1 = pIn + y1 * src_stride;
    uint8_t *pOut = dst_img + (size_t)y * dst_width * 2U;

    for (uint32_t x = 0U; x < dst_width; ++x)
    {
      int32_t sx = (int32_t)(x * x_step + x_step / 2U) - 32768;
      if (sx < 0) sx = 0;
      if ((uint32_t)sx > ((frame_width - 1U) << 16))
        sx = (int32_t)((frame_width - 1U) << 16);

      const uint32_t x0 = (uint32_t)sx >> 16;
      const uint32_t x1 = (x0 + 1U < frame_width) ? x0 + 1U : x0;
      const uint32_t fx = ((uint32_t)sx >> 8) & 255U;
      const uint32_t w00 = (256U - fx) * (256U - fy);
      const uint32_t w01 = fx * (256U - fy);
      const uint32_t w10 = (256U - fx) * fy;
      const uint32_t w11 = fx * fy;

      /* RGB565 is stored low byte first, as expected by QuantizeInput. */
      const uint32_t p00 = row0[2U * x0] | ((uint32_t)row0[2U * x0 + 1U] << 8);
      const uint32_t p01 = row0[2U * x1] | ((uint32_t)row0[2U * x1 + 1U] << 8);
      const uint32_t p10 = row1[2U * x0] | ((uint32_t)row1[2U * x0 + 1U] << 8);
      const uint32_t p11 = row1[2U * x1] | ((uint32_t)row1[2U * x1 + 1U] << 8);

      /* Interpolate channels separately to avoid mixing packed color bits. */
      const uint32_t r = (((p00 >> 11) * w00 + (p01 >> 11) * w01 +
                           (p10 >> 11) * w10 + (p11 >> 11) * w11) + 32768U) >> 16;
      const uint32_t g = (((((p00 >> 5) & 63U) * w00) + (((p01 >> 5) & 63U) * w01) +
                           (((p10 >> 5) & 63U) * w10) + (((p11 >> 5) & 63U) * w11)) + 32768U) >> 16;
      const uint32_t b = ((((p00 & 31U) * w00 + (p01 & 31U) * w01 +
                           (p10 & 31U) * w10 + (p11 & 31U) * w11)) + 32768U) >> 16;
      const uint16_t pixel = (uint16_t)((r << 11) | (g << 5) | b);

      pOut[2U * x] = (uint8_t)pixel;
      pOut[2U * x + 1U] = (uint8_t)(pixel >> 8);
    }
  }
}
