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

  template<class Deleter = Deleter<vk::RenderPass>>
  using RenderPass = claws::handle<vk::RenderPass, Deleter>;

  inline auto impl::Device::createRenderPass(vk::RenderPassCreateInfo const &renderPassCreateInfo) const
  {
    return RenderPass<>(Deleter<vk::RenderPass>{magma::Device<claws::no_delete>(*this)}, vk::Device::createRenderPass(renderPassCreateInfo));
  }

  inline auto impl::Device::getRenderAreaGranularity(RenderPass<claws::no_delete> renderPass) const
  {
    return vk::Device::getRenderAreaGranularity(renderPass);
  }
};
