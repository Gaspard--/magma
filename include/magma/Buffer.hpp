#pragma once

#include "magma/Device.hpp"

namespace magma
{
  struct BufferDeleter
  {
    Device<claws::no_delete> device;

    void operator()(vk::Buffer const &buffer) const
    {
      if (device)
        device.destroyBuffer(buffer);
    }
  };

  template<class Deleter = BufferDeleter>
  using Buffer = claws::handle<vk::Buffer, Deleter>;

  inline auto impl::Device::createBuffer(vk::BufferCreateFlags flags,
                                       vk::DeviceSize size,
                                       vk::BufferUsageFlags usage,
                                       std::vector<uint32_t> const &queueFamilies) const
  {
    return Buffer<>(BufferDeleter{magma::Device<claws::no_delete>(*this)},
                    vk::Device::createBuffer(
                      {flags, size, usage, vk::SharingMode::eExclusive, static_cast<uint32_t>(queueFamilies.size()), queueFamilies.data()}));
  }

  inline auto impl::Device::createBuffer(vk::BufferCreateFlags flags, vk::DeviceSize size, vk::BufferUsageFlags usage) const
  {
    return Buffer<>(BufferDeleter{magma::Device<claws::no_delete>(*this)},
                    vk::Device::createBuffer({flags, size, usage, vk::SharingMode::eConcurrent, 0, nullptr}));
  }
};
