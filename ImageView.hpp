#pragma once

#include "magma/Device.hpp"

namespace magma
{
  struct ImageViewDeleter
  {
    Device<claws::NoDelete> device;

    void operator()(vk::ImageView const &fence) const
    {
      if (device)
	device.destroyImageView(fence);
    }
  };
  
  template<class Deleter = ImageViewDeleter>
  using ImageView = claws::Handle<vk::ImageView, Deleter>;

  inline auto DeviceImpl::createImageView(vk::ImageViewCreateFlags flags, vk::Image image, vk::ImageViewType type, vk::Format format, vk::ComponentMapping components, vk::ImageSubresourceRange subresourceRange) const
  {
    return ImageView<>(ImageViewDeleter{magma::Device<claws::NoDelete>(*this)}, vk::Device::createImageView({flags, image, type, format, components, subresourceRange}));
  }
};
