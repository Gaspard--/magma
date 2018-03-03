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

  class DescriptorSet : protected vk::DescriptorSet
  {
  protected:
    Device<claws::NoDelete> device;

  public:
    DescriptorSet(vk::DescriptorSet descriptorSet)
      : vk::DescriptorSet(descriptorSet)
      , device(nullptr)
    {
    }

    
    void updateDescriptorSets(vk::DescriptorSet srcSet, uint32_t srcBinding, uint32_t srcArrayElement, uint32_t dstBinding, uint32_t dstArrayElement, uint32_t descriptorCount)
    {
      vk::CopyDescriptorSet info{ srcSet, srcBinding, srcArrayElement,
    	  *this, dstBinding, dstArrayElement, descriptorCount
    	  };
      device.updateDescriptorSets(0, nullptr, 1, &info);
    }

    void updateDescriptorSets(uint32_t dstBinding, uint32_t dstArrayElement, uint32_t descriptorCount, vk::DescriptorType descriptorType, vk::DescriptorImageInfo const *pImageInfo, vk::DescriptorBufferInfo const *pBufferInfo, vk::BufferView const *pTexelBufferView)
    {
      vk::WriteDescriptorSet info{ *this, dstBinding, dstArrayElement,
	  descriptorCount, descriptorType,
	  // pImageInfo.data(), pBufferInfo.data(), pTexelBufferView.data()
	  pImageInfo, pBufferInfo, pTexelBufferView
    	  };
      device.updateDescriptorSets(1, &info, 0, nullptr);
    }

    auto &raw()
    {
      return static_cast<vk::DescriptorSet &>(*this);
    }

  };

  template<class Deleter = DescriptorSetsDeleter>
  using DescriptorSets = claws::GroupHandle<DescriptorSet, std::vector<vk::DescriptorSet>, Deleter>;

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

    void updateDescriptorSets(std::vector<vk::WriteDescriptorSet> writeDescriptorSets)
    {
      device.updateDescriptorSets(static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
    }

    void updateDescriptorSets(std::vector<vk::CopyDescriptorSet> copyDescriptorSets)
    {
      device.updateDescriptorSets(0, nullptr, static_cast<uint32_t>(copyDescriptorSets.size()), copyDescriptorSets.data());
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
