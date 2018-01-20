#pragma once

#include "claws/HandleTypes.hpp"
#include "magma/VulkanHandler.hpp"

namespace magma
{
  class DeviceImpl : public vk::Device
  {
  protected:
    ~DeviceImpl() = default;

  public:
    DeviceImpl()
      : vk::Device(nullptr)
    {
    }

    DeviceImpl(vk::PhysicalDevice physicalDevice, std::vector<vk::DeviceQueueCreateInfo> const &deviceQueueCreateInfos, std::vector<char const *> const &extensions = {})
      : vk::Device([](vk::PhysicalDevice physicalDevice, std::vector<vk::DeviceQueueCreateInfo> const &deviceQueueCreateInfos, std::vector<char const *> const &extensions)
		   {
		     vk::DeviceCreateInfo deviceCreateInfo {
		       {},
			 static_cast<unsigned>(deviceQueueCreateInfos.size()), deviceQueueCreateInfos.data(),
			   0, nullptr,
			   static_cast<unsigned>(extensions.size()), extensions.data()
			   };
		   
		     return physicalDevice.createDevice(deviceCreateInfo);
		   }(physicalDevice, deviceQueueCreateInfos, extensions))
    {
    }

    using vk::Device::Device;

    void swap(DeviceImpl &other)
    {
      using std::swap;

      swap(static_cast<vk::Device &>(*this), static_cast<vk::Device &>(other));
    }

    auto createCommandPool(vk::CommandPoolCreateFlags flags, uint32_t queueFamilyIndex) const;

    auto shaderModuleDestructor() const noexcept;
    auto framebufferDestructor() const noexcept;

    template<class Container>
    auto createShaderModule(Container const &) const;

    auto createShaderModule(std::istream &input) const;

    auto createFramebuffer(claws::Handle<vk::RenderPass, claws::NoDelete> renderPass, std::vector<vk::ImageView> const &attachements, uint32_t width, uint32_t height, uint32_t layers) const;

    auto createFence(vk::FenceCreateFlags flags) const;

    auto createPipeline(vk::GraphicsPipelineCreateInfo const &createInfo) const;

    auto createRenderPass(vk::RenderPassCreateInfo const &renderPassCreateInfo) const;

    auto getRenderAreaGranularity(claws::Handle<vk::RenderPass, claws::NoDelete> renderPass) const;


    template<class... Params>
    decltype(vk::Device::destroy(std::declval<Params>()...)) destroy(Params &&...) = delete;

    struct DeviceDeleter
    {
      friend class DeviceImpl;

      void operator()(vk::Device const &device) const
      {
	if (device)
	  device.destroy();
      }
    };
  };

  void swap(DeviceImpl &lh, DeviceImpl &rh)
  {
    lh.swap(rh);
  }

  template<class Deleter = DeviceImpl::DeviceDeleter>
  using Device = claws::Handle<DeviceImpl, Deleter>;
};
