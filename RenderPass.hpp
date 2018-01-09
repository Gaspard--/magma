#pragma once

#include "magma/Device.hpp"

namespace magma
{
  struct RenderPassCreateInfo
  {
    std::vector<vk::AttachmentDescription> attachements;
    std::vector<vk::SubpassDescription> subPasses;
    std::vector<vk::SubpassDependency> subPassDependencies;
    vk::RenderPassCreateFlags flags;

    operator vk::RenderPassCreateInfo() const
    {
      return {
	  flags,
	  static_cast<uint32_t>(attachements.size()), attachements.data(),
	  static_cast<uint32_t>(subPasses.size()), subPasses.data(),
	  static_cast<uint32_t>(subPassDependencies.size()), subPassDependencies.data()
	};
    }

    RenderPassCreateInfo() = default;
    RenderPassCreateInfo(RenderPassCreateInfo const &) = delete;
    RenderPassCreateInfo(RenderPassCreateInfo &&) = default;
    ~RenderPassCreateInfo() = default;
  };

  class RenderPassImpl : protected vk::RenderPass
  {
  private:
    RenderPassImpl(Device<NoDelete> device, vk::RenderPassCreateInfo const &renderPassCreateInfo)
      : vk::RenderPass(device.createRenderPass(renderPassCreateInfo))
      , device(device)
    {
    }
  protected:
    Device<NoDelete> device;

    ~RenderPassImpl() = default;
  public:
    RenderPassImpl()
      : vk::RenderPass(nullptr)
      , device(nullptr)
    {
    }

    RenderPassImpl(Device<NoDelete> device, RenderPassCreateInfo const &renderPassCreateInfo)
      : RenderPassImpl(device, static_cast<vk::RenderPassCreateInfo>(renderPassCreateInfo))
    {
    }

    void swap(RenderPassImpl &other)
    {
      using std::swap;

      swap(static_cast<vk::RenderPass &>(*this), static_cast<vk::RenderPass &>(other));
      swap(device, other.device);
    }

    struct RenderPassDeleter
    {
      friend class RenderPassImpl;

      void operator()(RenderPassImpl const &renderPass)
      {
	if (renderPass)
	  renderPass.device.destroyRenderPass(renderPass);
      }
    };
  };

  inline void swap(RenderPassImpl &lh, RenderPassImpl &rh)
  {
    lh.swap(rh);
  }

  template<class Deleter = RenderPassImpl::RenderPassDeleter>
  using RenderPass = Handle<RenderPassImpl, Deleter>;
};
