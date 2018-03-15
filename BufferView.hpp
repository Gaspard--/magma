#pragma once

#include "magma/Device.hpp"

namespace magma
{
  struct BufferViewDeleter
  {
    Device<claws::no_delete> device;

    void operator()(vk::BufferView const &bufferView) const
    {
      if (device)
        device.destroyBufferView(bufferView);
    }
  };

  template<class Deleter = BufferViewDeleter>
  using BufferView = claws::handle<vk::BufferView, Deleter>;

  inline auto DeviceImpl::createBufferView(
    vk::BufferViewCreateFlags flags, claws::handle<vk::Buffer, claws::no_delete> buffer, vk::Format format, vk::DeviceSize offset, vk::DeviceSize size) const
  {
    return BufferView<>(BufferViewDeleter{magma::Device<claws::no_delete>(*this)}, vk::Device::createBufferView({flags, buffer, format, offset, size}));
  }
};
