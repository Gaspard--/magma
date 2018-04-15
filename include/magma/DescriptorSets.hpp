#pragma once

#include "magma/Device.hpp"

namespace magma
{
  struct DescriptorSetsDeleter
  {
    Device<claws::no_delete> device;
    vk::DescriptorPool descriptorPool;

    void operator()(std::vector<vk::DescriptorSet> const &descriptorSets) const
    {
      if (device)
        device.freeDescriptorSets(descriptorPool, descriptorSets);
    }
  };

  template<class Deleter = DescriptorSetsDeleter>
  using DescriptorSets = claws::group_handle<vk::DescriptorSet, std::vector<vk::DescriptorSet>, Deleter>;

  template<class T, std::enable_if_t<std::is_same_v<typename T::value_type, vk::WriteDescriptorSet>> *>
  void impl::Device::updateDescriptorSets(T const &writeDescriptorSets)
  {
    vk::Device::updateDescriptorSets(static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
  }

  template<class T, std::enable_if_t<std::is_same_v<typename T::value_type, vk::CopyDescriptorSet>> *>
  void impl::Device::updateDescriptorSets(T const &copyDescriptorSets)
  {
    vk::Device::updateDescriptorSets(0, nullptr, static_cast<uint32_t>(copyDescriptorSets.size()), copyDescriptorSets.data());
  }

  namespace impl
  {
    class DescriptorPool : public vk::DescriptorPool
    {
    protected:
      magma::Device<claws::no_delete> device;

      ~DescriptorPool() = default;

    public:
      DescriptorPool()
        : vk::DescriptorPool(nullptr)
        , device{}
      {}

      DescriptorPool(magma::Device<claws::no_delete> device, vk::DescriptorPool descriptorPool)
        : vk::DescriptorPool(descriptorPool)
        , device(device)
      {}

      void reset(vk::DescriptorPoolResetFlags flags) const
      {
        device.resetDescriptorPool(*this, flags);
      }

      auto allocateDescriptorSets(std::vector<vk::DescriptorSetLayout> const &descriptorSetLayout)
      {
        vk::DescriptorSetAllocateInfo info{*this, static_cast<uint32_t>(descriptorSetLayout.size()), descriptorSetLayout.data()};
        return DescriptorSets<>({device, *this}, device.allocateDescriptorSets(info));
      }

      void swap(DescriptorPool &other)
      {
        using std::swap;

        swap(static_cast<vk::DescriptorPool &>(*this), static_cast<vk::DescriptorPool &>(other));
        swap(device, other.device);
      }

    };

    inline void swap(DescriptorPool &lh, DescriptorPool &rh)
    {
      lh.swap(rh);
    }
  }

  template<class Deleter = Deleter<vk::DescriptorPool>>
  using DescriptorPool = claws::handle<impl::DescriptorPool, Deleter>;

  inline auto impl::Device::createDescriptorPool(std::uint32_t maxSets, std::vector<vk::DescriptorPoolSize> const &size) const
  {
    return magma::DescriptorPool<>({},
                                   magma::Device<claws::no_delete>(*this),
                                   vk::Device::createDescriptorPool(
                                     {vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, maxSets, static_cast<uint32_t>(size.size()), size.data()}));
  }
};
