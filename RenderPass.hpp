#pragma once

#include "magma/DeviceBasedHandle.hpp"

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

  class RenderPassImpl : protected DeviceBasedHandleImpl<vk::RenderPass>
  {
  private:
    RenderPassImpl(Device<NoDelete> device, vk::RenderPassCreateInfo const &renderPassCreateInfo)
      : DeviceBasedHandleImpl<vk::RenderPass>(device.createRenderPass(renderPassCreateInfo), device)
    {
    }
  protected:

    ~RenderPassImpl() = default;
  public:
    RenderPassImpl() = default;

    RenderPassImpl(Device<NoDelete> device, RenderPassCreateInfo const &renderPassCreateInfo)
      : RenderPassImpl(device, static_cast<vk::RenderPassCreateInfo>(renderPassCreateInfo))
    {
    }

    auto getRenderAreaGranularity() const
    {
      return device.getRenderAreaGranularity(*this);
    }

    auto raw() const noexcept
    {
      return static_cast<vk::RenderPass>(*this);
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

  template<class Deleter = RenderPassImpl::RenderPassDeleter>
  using RenderPass = Handle<RenderPassImpl, Deleter>;
};
