#pragma once

#include "magma/Device.hpp"

namespace magma
{
  struct RenderPassCreateInfo
  {
    vk::RenderPassCreateFlags flags;
    std::vector<vk::AttachmentDescription> attachements;
    std::vector<vk::SubpassDescription> subPasses;
    std::vector<vk::SubpassDependency> subPassDependencies;

    RenderPassCreateInfo(vk::RenderPassCreateFlags flags)
      : flags(flags)
    {}

    operator vk::RenderPassCreateInfo() const
    {
      return {flags,
              static_cast<uint32_t>(attachements.size()),
              attachements.data(),
              static_cast<uint32_t>(subPasses.size()),
              subPasses.data(),
              static_cast<uint32_t>(subPassDependencies.size()),
              subPassDependencies.data()};
    }

    RenderPassCreateInfo() = default;
    RenderPassCreateInfo(RenderPassCreateInfo const &) = delete;
    RenderPassCreateInfo(RenderPassCreateInfo &&) = default;
    ~RenderPassCreateInfo() = default;
  };

  struct RenderPassDeleter
  {
    Device<claws::NoDelete> device;

    void operator()(vk::RenderPass const &renderPass)
    {
      if (device)
        device.destroyRenderPass(renderPass);
    }
  };

  template<class Deleter = RenderPassDeleter>
  using RenderPass = claws::Handle<vk::RenderPass, Deleter>;

  inline auto DeviceImpl::createRenderPass(vk::RenderPassCreateInfo const &renderPassCreateInfo) const
  {
    return RenderPass<>(RenderPassDeleter{magma::Device<claws::NoDelete>(*this)}, vk::Device::createRenderPass(renderPassCreateInfo));
  }

  inline auto DeviceImpl::getRenderAreaGranularity(RenderPass<claws::NoDelete> renderPass) const
  {
    return vk::Device::getRenderAreaGranularity(renderPass);
  }
};
