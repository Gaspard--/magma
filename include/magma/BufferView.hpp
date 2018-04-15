#pragma once

#include "magma/Deleter.hpp"
#include "magma/Device.hpp"

namespace magma
{
  template<class Deleter = Deleter>
  using BufferView = claws::handle<vk::BufferView, Deleter>;

  inline auto impl::Device::createBufferView(
    vk::BufferViewCreateFlags flags, claws::handle<vk::Buffer, claws::no_delete> buffer, vk::Format format, vk::DeviceSize offset, vk::DeviceSize size) const
  {
    return BufferView<>(Deleter{magma::Device<claws::no_delete>(*this)}, vk::Device::createBufferView({flags, buffer, format, offset, size}));
  }
};
