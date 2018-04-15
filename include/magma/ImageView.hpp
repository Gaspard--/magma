#pragma once

#include "magma/Deleter.hpp"
#include "magma/Device.hpp"

namespace magma
{
  template<class Deleter = Deleter<vk::ImageView>>
  using ImageView = claws::handle<vk::ImageView, Deleter>;

  inline auto impl::Device::createImageView(vk::ImageViewCreateFlags flags,
                                            vk::Image image,
                                            vk::ImageViewType type,
                                            vk::Format format,
                                            vk::ComponentMapping components,
                                            vk::ImageSubresourceRange subresourceRange) const
  {
    return ImageView<>(Deleter<vk::ImageView>{magma::Device<claws::no_delete>(*this)},
                       vk::Device::createImageView({flags, image, type, format, components, subresourceRange}));
  }
};
