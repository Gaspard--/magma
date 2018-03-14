#pragma once

#include "magma/Device.hpp"

namespace magma
{
  struct BufferViewDeleter
  {
    Device<claws::NoDelete> device;

    void operator()(vk::BufferView const &bufferView) const
    {
      if (device)
        device.destroyBufferView(bufferView);
    }
  };

  template<class Deleter = BufferViewDeleter>
  using BufferView = claws::Handle<vk::BufferView, Deleter>;

  inline auto DeviceImpl::createBufferView(
    vk::BufferViewCreateFlags flags, claws::Handle<vk::Buffer, claws::NoDelete> buffer, vk::Format format, vk::DeviceSize offset, vk::DeviceSize size) const
  {
    return BufferView<>(BufferViewDeleter{magma::Device<claws::NoDelete>(*this)}, vk::Device::createBufferView({flags, buffer, format, offset, size}));
  }
};
