#pragma once

#include "magma/Device.hpp"
#include "magma/Deleter.hpp"

namespace magma
{
  template<class Deleter = Deleter<vk::Buffer>>
  using Buffer = claws::handle<vk::Buffer, Deleter>;

  inline auto impl::Device::createBuffer(vk::BufferCreateFlags flags,
                                         vk::DeviceSize size,
                                         vk::BufferUsageFlags usage,
                                         std::vector<uint32_t> const &queueFamilies) const
  {
    return Buffer<>(Deleter<vk::Buffer>{magma::Device<claws::no_delete>(*this)},
                    vk::Device::createBuffer(
                      {flags, size, usage, vk::SharingMode::eExclusive, static_cast<uint32_t>(queueFamilies.size()), queueFamilies.data()}));
  }

  inline auto impl::Device::createBuffer(vk::BufferCreateFlags flags, vk::DeviceSize size, vk::BufferUsageFlags usage) const
  {
    return Buffer<>(Deleter<vk::Buffer>{magma::Device<claws::no_delete>(*this)},
                    vk::Device::createBuffer({flags, size, usage, vk::SharingMode::eConcurrent, 0, nullptr}));
  }
};
