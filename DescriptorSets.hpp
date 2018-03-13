#pragma once

#include "magma/Device.hpp"

namespace magma
{

  struct DescriptorSetsDeleter
  {
    Device<claws::NoDelete> device;
    vk::DescriptorPool descriptorPool;

    void operator()(std::vector<vk::DescriptorSet> const &descriptorSets) const
    {
      if (device)
	device.freeDescriptorSets(descriptorPool, descriptorSets);
    }
  };

  template<class Deleter = DescriptorSetsDeleter>
  using DescriptorSets = claws::GroupHandle<vk::DescriptorSet, std::vector<vk::DescriptorSet>, Deleter>;

  template<class T, std::enable_if_t<std::is_same_v<typename T::value_type, vk::WriteDescriptorSet>> *>
  void DeviceImpl::updateDescriptorSets(T const &writeDescriptorSets)
  {
    vk::Device::updateDescriptorSets(static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
  }

  template<class T, std::enable_if_t<std::is_same_v<typename T::value_type, vk::CopyDescriptorSet>> *>
  void DeviceImpl::updateDescriptorSets(T const &copyDescriptorSets)
  {
    vk::Device::updateDescriptorSets(0, nullptr, static_cast<uint32_t>(copyDescriptorSets.size()), copyDescriptorSets.data());
  }

  class DescriptorPoolImpl : public vk::DescriptorPool
  {
  protected:
    Device<claws::NoDelete> device;

    ~DescriptorPoolImpl() = default;
  public:
    DescriptorPoolImpl()
      : vk::DescriptorPool(nullptr)
      , device(nullptr)
    {
    }

    DescriptorPoolImpl(Device<claws::NoDelete> device, vk::DescriptorPool descriptorPool)
      : vk::DescriptorPool(descriptorPool)
      , device(device)
    {
    }

    void reset(vk::DescriptorPoolResetFlags flags) const
    {
      device.resetDescriptorPool(*this, flags);
    }

    auto allocateDescriptorSets(std::vector<vk::DescriptorSetLayout> const &descriptorSetLayout)
    {
      vk::DescriptorSetAllocateInfo info{*this,
	  static_cast<uint32_t>(descriptorSetLayout.size()),
	  descriptorSetLayout.data()};
      return DescriptorSets<>({device, *this}, device.allocateDescriptorSets(info));
    }

    void swap(DescriptorPoolImpl &other)
    {
      using std::swap;

      swap(static_cast<vk::DescriptorPool &>(*this), static_cast<vk::DescriptorPool &>(other));
      swap(device, other.device);
    }

    struct DescriptorPoolDeleter
    {
      friend DescriptorPoolImpl;

      void operator()(DescriptorPoolImpl const &descriptorPool) const
      {
	if (descriptorPool)
	  descriptorPool.device.destroyDescriptorPool(descriptorPool);
      }
    };
  };

  inline void swap(DescriptorPoolImpl &lh, DescriptorPoolImpl &rh)
  {
    lh.swap(rh);
  }

  template<class Deleter = DescriptorPoolImpl::DescriptorPoolDeleter>
  using DescriptorPool = claws::Handle<DescriptorPoolImpl, Deleter>;


  inline auto DeviceImpl::createDescriptorPool(std::uint32_t maxSets, std::vector<vk::DescriptorPoolSize> const &size) const
  {
    return DescriptorPool<>({}, magma::Device<claws::NoDelete>(*this),
			    vk::Device::createDescriptorPool({vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
				  maxSets,
				  static_cast<uint32_t>(size.size()),
				  size.data()}));
  }
};
