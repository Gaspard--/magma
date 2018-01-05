#pragma once

#include <array>
#include <vector>
#include <unordered_map>
#include <set>
#include "vulkan/vulkan.hpp"

namespace magma
{
#ifdef DEBUG_LAYERS
  inline VkResult vkCreateDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackCreateInfoEXT const *createInfo, VkAllocationCallbacks const *allocator, VkDebugReportCallbackEXT *callback)
  {
    static auto func = (PFN_vkCreateDebugReportCallbackEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
    if (func)
      return func(instance, createInfo, allocator, callback);
    throw std::runtime_error("vulkan: Failed to load vkCreateDebugReportCallbackEXT");
  }

  inline void vkDestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, VkAllocationCallbacks const *allocator)
  {
    static auto func = (PFN_vkDestroyDebugReportCallbackEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
    if (func)
      return func(instance, callback, allocator);
    throw std::runtime_error("vulkan: Failed to load vkDestroyDebugReportCallbackEXT");
  }
#endif

  template<class... T>
  std::array<char const *, sizeof...(T)> make_const_char_array(T... params)
  {
    return std::array<char const *, sizeof...(T)>{((const char *)params)...};
  }

  class Instance
  {
  private:
  public:
    vk::Instance vkInstance;
#ifdef DEBUG_LAYERS
    vk::DebugReportCallbackEXT callback;
#endif

    template<class DEVICE_FILTER, class COMPARE>
    vk::PhysicalDevice selectDevice(DEVICE_FILTER deviceFilter, COMPARE compare)
    {
      auto physicalDevices(vkInstance.enumeratePhysicalDevices());
      auto end(std::remove_if(physicalDevices.begin(), physicalDevices.end(),
			      [deviceFilter](auto &&a){
				return !deviceFilter(a);
			      }));
      auto it(std::max_element(physicalDevices.begin(), end, compare));

      if (it == end)
	throw std::runtime_error("vulkan : no suitable device found");
      return *it;
    }

    template<class FILTER, class COMPARE>
    std::pair<vk::PhysicalDevice, unsigned int> selectQueue(FILTER filter, COMPARE compare)
    {
      auto physicalDevices(vkInstance.enumeratePhysicalDevices());
      std::vector<std::tuple<vk::QueueFamilyProperties, vk::PhysicalDevice, unsigned int>> queueList;

      for (auto const &physicalDevice : physicalDevices)
	{
	  auto queueFamilyPropertiesList(physicalDevice.getQueueFamilyProperties());

	  for (auto it2(queueFamilyPropertiesList.begin()); it2 != queueFamilyPropertiesList.end(); ++it2)
	    queueList.emplace_back(*it2, physicalDevice, it2 - queueFamilyPropertiesList.begin());
	}
      auto end(std::remove_if(queueList.begin(), queueList.end(),
			      [filter](auto &&a){
				return !filter(std::get<0>(a), std::get<1>(a),  std::get<2>(a));
			      }));
      auto it(std::max_element(queueList.begin(), end, [compare](auto &&a, auto &&b)
			       {
				 return compare(std::get<0>(a), std::get<0>(b));
			       }));

      if (it == end)
	throw std::runtime_error("vulkan : no suitable queue found");
      return {std::get<1>(*it), std::get<2>(*it)};
    }

    Instance(Instance const &) = delete;
    Instance(Instance &&) = delete;

    Instance(std::vector<char const *> &&extensions = {})
      : vkInstance([](std::vector<char const *> &&extensions){
	  vk::ApplicationInfo appInfo("Wasted Prophecies", VK_MAKE_VERSION(1, 0, 0), nullptr,
				      VK_MAKE_VERSION(1, 0, 0), VK_API_VERSION_1_0);
#ifdef DEBUG_LAYERS
	  auto validationLayers = make_const_char_array("VK_LAYER_LUNARG_standard_validation");
	  extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#endif

	  vk::InstanceCreateInfo instanceCreateInfo({}, nullptr,
#ifdef DEBUG_LAYERS
						    validationLayers.size(), validationLayers.data(),
#else
						    0, nullptr,
#endif
						    extensions.size(), extensions.data());

	  return vk::createInstance(instanceCreateInfo);
	}(std::forward<std::vector<char const *>>(extensions)))
#ifdef DEBUG_LAYERS
      , callback([](vk::Instance vkInstance)
		 {
		   vk::DebugReportCallbackCreateInfoEXT createInfo{vk::DebugReportFlagBitsEXT::eError | vk::DebugReportFlagBitsEXT::eWarning,
		       [](unsigned int, // Seems signature is wrong in hpp?
			  VkDebugReportObjectTypeEXT,
			  uint64_t,
			  size_t,
			  int32_t,
			  const char* layerPrefix,
			  const char* msg,
			  void*) -> unsigned int
			 {
			   std::cerr << layerPrefix << ": " << msg << std::endl;
			   return VK_FALSE;
			 }
		   };
		   return vkInstance.createDebugReportCallbackEXT(createInfo);
		 }(vkInstance))
#endif
    {
      if (!vkInstance)
	throw std::runtime_error("vulkan : failed to create instance");
    }

    ~Instance()
    {
#ifdef DEBUG_LAYERS
      vkInstance.destroyDebugReportCallbackEXT(callback);
#endif
      vkInstance.destroy();
    }
  };

  class RenderPass : public vk::RenderPass
  {
    vk::Device device;

  public:
    RenderPass(vk::RenderPass renderPass, vk::Device device)
      : vk::RenderPass(renderPass)
      , device(device)
    {
    }

    RenderPass(RenderPass const &) = delete;
    RenderPass(RenderPass &&) = default;

    ~RenderPass()
    {
      device.destroyRenderPass(*this);
    }
  };

  class CommandPool : public vk::CommandPool
  {
    vk::Device device;

  public:
    CommandPool(vk::CommandPool commandBuffer, vk::Device device)
      : vk::CommandPool(commandBuffer)
      , device(device)
    {
    }

    CommandPool(CommandPool const &) = delete;
    CommandPool(CommandPool &&) = default;
    
    ~CommandPool()
    {
      device.destroyCommandPool(*this);
    }

    void reset(vk::CommandPoolResetFlags flags)
    {
      device.resetCommandPool(*this, flags);
    }

    // class CommandBufferGroup
    // {
    //   Device &device;
    //   uint32_t queueFamilyIndex;
    //   std::vector<vk::CommandBuffer> commandBuffers;

    //   class CommandBuffer
    //   {
    // 	Device &device;
    //  uint32_t queueFamilyIndex;
    // 	vk::CommandBuffer commandBuffer;

    //   public:
    // 	CommandBuffer(Device &device,
    // 		      uint32_t queueFamilyIndex,
    // 		      vk::CommandBuffer commandBuffer)
    // 	  : device(device)
    // 	  , queueFamilyIndex(queueFamilyIndex)
    // 	  , commandBuffer(commandBuffer)
    // 	{
    // 	}
    //   };

    // public:
    //   CommandBufferGroup(Device device, uint32_t queueFamilyIndex, std::vector<vk::CommandBuffer> commandBuffers)
    // 	: device(device)
    // 	, queueFamilyIndex(queueFamilyIndex)
    // 	, commandBuffers(std::move(commandBuffers))
    //   {
    //   }

    //   auto operator[](std::size_t index)
    //   {
    // 	   return CommandBuffer(device, queueFamilyIndex, commandBuffers[index]);
    //   }
    // };

    // auto allocateCommandBuffers(vk::CommandBufferLevel level, uint32_t commandBufferCount)
    // {      
    //   vk::CommandBufferAllocateInfo info{*this, level, commandBufferCount};

    //   return CommandBufferGroup(device, device.allocateCommandBuffers(info);
    // }
  };

  struct RenderPassCreateInfo
  {
    std::vector<vk::AttachmentDescription> attachements;
    std::vector<vk::SubpassDescription> subPasses;
    std::vector<vk::SubpassDependency> subPassDependencies;

    operator vk::RenderPassCreateInfo() const
    {
      return
	{
	  {},
	    static_cast<uint32_t>(attachements.size()),
	      attachements.data(),
	      static_cast<uint32_t>(subPasses.size()),
	      subPasses.data(),
	      static_cast<uint32_t>(subPassDependencies.size()),
	      subPassDependencies.data()
	      };
    }

    RenderPassCreateInfo() = default;
    RenderPassCreateInfo(RenderPassCreateInfo const &) = delete;
    RenderPassCreateInfo(RenderPassCreateInfo &&) = default;
    ~RenderPassCreateInfo() = default;
  };
  
  class Device : private vk::Device
  {

  public:
    Device(vk::PhysicalDevice physicalDevice, std::vector<vk::DeviceQueueCreateInfo> const &deviceQueueCreateInfos, std::vector<char const *> const &extensions = {})
      : vk::Device([](vk::PhysicalDevice physicalDevice, std::vector<vk::DeviceQueueCreateInfo> const &deviceQueueCreateInfos, std::vector<char const *> const &extensions)
		   {
		     vk::DeviceCreateInfo deviceCreateInfo
		     {
		       {},
			 static_cast<unsigned>(deviceQueueCreateInfos.size()), deviceQueueCreateInfos.data(),
			   0, nullptr,
			   static_cast<unsigned>(extensions.size()), extensions.data()
			   };
		   
		     return physicalDevice.createDevice(deviceCreateInfo);
		   }(physicalDevice, deviceQueueCreateInfos, extensions))
    {
    }

    ~Device()
    {
      destroy();
    }

    auto createRenderPass(RenderPassCreateInfo const &renderPassCreateInfo)
    {
      return RenderPass(vk::Device::createRenderPass(renderPassCreateInfo), *this);
    }

    auto createCommandPool(vk::CommandPoolCreateFlags flags, uint32_t queueFamilyIndex)
    {
      vk::CommandPoolCreateInfo createInfo{flags, queueFamilyIndex};

      return vk::Device::createCommandPool(createInfo);
    }

    using vk::Device::createSwapchainKHR;
    using vk::Device::acquireNextImageKHR;
    using vk::Device::destroySwapchainKHR;
  };

};
