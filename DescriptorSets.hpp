#pragma once

#include "magma/Device.hpp"

namespace magma
{
  struct DescriptorSetDeleter
  {
    Device<claws::NoDelete> device;
    vk::DescriptorPool descriptorPool;

    void operator()(std::vector<vk::DescriptorSet> const &descriptorSets) const
    {
      if (device)
	device.freeDescriptorSets(descriptorPool, descriptorSets);
    }
  };

  template<class Deleter = DescriptorSetDeleter>
  using DescriptorSets = claws::Handle<std::vector<vk::DescriptorSet>, Deleter>;

  class DescriptorSet : protected vk::DescriptorSet
  {
  protected:

  public:
    DescriptorSet(vk::DescriptorSet descriptorSet)
      : vk::DescriptorSet(descriptorSet)
    {
    }

    auto &raw()
    {
      return static_cast<vk::DescriptorSet &>(*this);
    }

  };

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
