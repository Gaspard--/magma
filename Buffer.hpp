#pragma once

#include "magma/Device.hpp"

namespace magma
{
  struct BufferDeleter
  {
    Device<claws::NoDelete> device;

    void operator()(vk::Buffer const &buffer) const
    {
      if (device)
        device.destroyBuffer(buffer);
    }
  };

  template<class Deleter = BufferDeleter>
  using Buffer = claws::Handle<vk::Buffer, Deleter>;

  inline auto DeviceImpl::createBuffer(vk::BufferCreateFlags flags,
                                       vk::DeviceSize size,
                                       vk::BufferUsageFlags usage,
                                       std::vector<uint32_t> const &queueFamilies) const
  {
    return Buffer<>(BufferDeleter{magma::Device<claws::NoDelete>(*this)},
                    vk::Device::createBuffer(
                      {flags, size, usage, vk::SharingMode::eExclusive, static_cast<uint32_t>(queueFamilies.size()), queueFamilies.data()}));
  }

  inline auto DeviceImpl::createBuffer(vk::BufferCreateFlags flags, vk::DeviceSize size, vk::BufferUsageFlags usage) const
  {
    return Buffer<>(BufferDeleter{magma::Device<claws::NoDelete>(*this)},
                    vk::Device::createBuffer({flags, size, usage, vk::SharingMode::eConcurrent, 0, nullptr}));
  }
};
