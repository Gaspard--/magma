#pragma once

#include "magma/Device.hpp"

namespace magma
{
  template<class Deleter = Deleter<vk::Image>>
  using Image = claws::handle<vk::Image, Deleter>;

  inline auto impl::Device::createImage2D(vk::ImageCreateFlags flags,
                                          vk::Format format,
                                          std::array<uint32_t, 2> extent,
                                          vk::SampleCountFlagBits samples,
                                          vk::ImageTiling tiling,
                                          vk::ImageUsageFlags usage,
                                          std::vector<uint32_t> indices,
                                          vk::ImageLayout layout) const
  {
    return Image<>(Deleter<vk::Image>{magma::Device<claws::no_delete>(*this)},
                   vk::Device::createImage({flags,
                                            vk::ImageType::e2D,
                                            format,
                                            vk::Extent3D{extent[0], extent[1], 1},
                                            1,
                                            1,
                                            samples,
                                            tiling,
                                            usage,
                                            vk::SharingMode::eConcurrent,
                                            static_cast<uint32_t>(indices.size()),
                                            indices.data(),
                                            layout}));
  }

  inline auto impl::Device::createImage2D(vk::ImageCreateFlags flags,
                                          vk::Format format,
                                          std::array<uint32_t, 2> extent,
                                          vk::SampleCountFlagBits samples,
                                          vk::ImageTiling tiling,
                                          vk::ImageUsageFlags usage,
                                          vk::ImageLayout layout) const
  {
    return Image<>(Deleter<vk::Image>{magma::Device<claws::no_delete>(*this)},
                   vk::Device::createImage({flags,
                                            vk::ImageType::e2D,
                                            format,
                                            vk::Extent3D{extent[0], extent[1], 1},
                                            1,
                                            1,
                                            samples,
                                            tiling,
                                            usage,
                                            vk::SharingMode::eExclusive,
                                            0,
                                            nullptr,
                                            layout}));
  }
};
