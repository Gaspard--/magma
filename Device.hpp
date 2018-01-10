#pragma once

#include "util/HandleTypes.hpp"
#include "magma/VulkanHandler.hpp"

namespace magma
{
  class RenderPassCreateInfo;
  
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

    auto createCommandPool(vk::CommandPoolCreateFlags flags, uint32_t queueFamilyIndex);
    
    // using vk::Device::createSwapchainKHR;
    // using vk::Device::acquireNextImageKHR;
    // using vk::Device::destroySwapchainKHR;
    // using vk::Device::createRenderPass;
    // using vk::Device::destroyRenderPass;

    struct DeviceDeleter
    {
      friend class DeviceImpl;

      void operator()(vk::Device const &device)
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
  using Device = Handle<DeviceImpl, Deleter>;
};
