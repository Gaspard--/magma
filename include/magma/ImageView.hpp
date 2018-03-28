#pragma once

#include "magma/Device.hpp"

namespace magma
{
  struct ImageViewDeleter
  {
    Device<claws::no_delete> device;

    void operator()(vk::ImageView const &fence) const
    {
      if (device)
        device.destroyImageView(fence);
    }
  };

  template<class Deleter = ImageViewDeleter>
  using ImageView = claws::handle<vk::ImageView, Deleter>;

  inline auto impl::Device::createImageView(vk::ImageViewCreateFlags flags,
                                          vk::Image image,
                                          vk::ImageViewType type,
                                          vk::Format format,
                                          vk::ComponentMapping components,
                                          vk::ImageSubresourceRange subresourceRange) const
  {
    return ImageView<>(ImageViewDeleter{magma::Device<claws::no_delete>(*this)},
                       vk::Device::createImageView({flags, image, type, format, components, subresourceRange}));
  }
};
