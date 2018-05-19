#pragma once

#include "vulkan/vulkan.hpp"

#include "claws/iterator_util.hpp"
#include <array>
#include <vector>
#include <unordered_map>
#include <set>
#include <iostream>

#ifdef DEBUG_LAYERS
inline VkResult vkCreateDebugReportCallbackEXT(VkInstance instance,
                                               VkDebugReportCallbackCreateInfoEXT const *createInfo,
                                               VkAllocationCallbacks const *allocator,
                                               VkDebugReportCallbackEXT *callback)
{
  static auto func = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT"));
  if (func)
    return func(instance, createInfo, allocator, callback);
  throw std::runtime_error("vulkan: Failed to load vkCreateDebugReportCallbackEXT");
}

inline void vkDestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, VkAllocationCallbacks const *allocator)
{
  static auto func = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT"));
  if (func)
    return func(instance, callback, allocator);
  throw std::runtime_error("vulkan: Failed to load vkDestroyDebugReportCallbackEXT");
}
#endif

#include "vulkan/vulkan.hpp"

namespace magma
{
  template<class... T>
  std::array<char const *, sizeof...(T)> make_const_char_array(T... params)
  {
    return std::array<char const *, sizeof...(T)>{static_cast<const char *>(params)...};
  }

  class Instance
  {
  private:
  public:
    vk::Instance vkInstance;
#ifdef DEBUG_LAYERS
    vk::DebugReportCallbackEXT callback;
#endif

    /// \breif selects a device based on the returned score by `mapToScore`
    ///
    /// @param mapToScore maps a device to a score. The returned type must be comparable with `operator<`
    /// @return a `std::pair` of the select `vk::PhysicalDevice` and the corresponding biggest value returned by mapToScore.
    /// @throw Any exception thrown by `mapToScore`, or by vulkan-hpp through `enumeratePhysicalDevices`.
    template<class SCORE_MAPPER>
    auto selectDevice(SCORE_MAPPER mapToScore)
    {
      auto physicalDevices(vkInstance.enumeratePhysicalDevices());

      claws::container_view scoreList(vkInstance.enumeratePhysicalDevices(), mapToScore);

      if (physicalDevices.size() == 1)
	return std::pair{physicalDevices[0], scoreList[0]};

      std::array<std::pair<vk::PhysicalDevice, decltype(scoreList[0])>, 2u> best_and_next{
	std::pair{physicalDevices[0], scoreList[0]},
	  std::pair{physicalDevices[1], scoreList[1]}
      };
      bool swapped(best_and_next[0].second < best_and_next[1].second);
      for (size_t i = 2; i < physicalDevices.size(); ++i)
	{
	  best_and_next[!swapped] = std::pair{physicalDevices[i], scoreList[i]};
	  swapped = best_and_next[0].second < best_and_next[1].second;
	}
      return best_and_next[swapped];
    }

    template<class DEVICE_FILTER, class COMPARE>
    vk::PhysicalDevice selectDevice(DEVICE_FILTER deviceFilter, COMPARE compare)
    {
      auto physicalDevices(vkInstance.enumeratePhysicalDevices());
      auto end(std::remove_if(physicalDevices.begin(), physicalDevices.end(), [deviceFilter](auto &&a) { return !deviceFilter(a); }));
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
      auto end(std::remove_if(queueList.begin(), queueList.end(), [filter](auto &&a) { return !filter(std::get<0>(a), std::get<1>(a), std::get<2>(a)); }));
      auto it(std::max_element(queueList.begin(), end, [compare](auto &&a, auto &&b) { return compare(std::get<0>(a), std::get<0>(b)); }));

      if (it == end)
        throw std::runtime_error("vulkan : no suitable queue found");
      return {std::get<1>(*it), std::get<2>(*it)};
    }

    Instance(std::vector<char const *> &&extensions = {})
      : vkInstance([](std::vector<char const *> &&extensions) {
        vk::ApplicationInfo appInfo("Wasted Prophecies", VK_MAKE_VERSION(1, 0, 0), nullptr, VK_MAKE_VERSION(1, 0, 0), VK_API_VERSION_1_0);
#ifdef DEBUG_LAYERS
        auto validationLayers = make_const_char_array("VK_LAYER_LUNARG_standard_validation");
        extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#endif

        vk::InstanceCreateInfo instanceCreateInfo({},
                                                  nullptr,
#ifdef DEBUG_LAYERS
                                                  static_cast<uint32_t>(validationLayers.size()),
                                                  validationLayers.data(),
#else
                                                  0,
                                                  nullptr,
#endif
                                                  static_cast<uint32_t>(extensions.size()),
                                                  extensions.data());

        return vk::createInstance(instanceCreateInfo);
	}(std::move(extensions)))
#ifdef DEBUG_LAYERS
      , callback([](vk::Instance vkInstance) {
        vk::DebugReportCallbackCreateInfoEXT createInfo{vk::DebugReportFlagBitsEXT::eError | vk::DebugReportFlagBitsEXT::eWarning,
                                                        [](unsigned int, // Seems signature is wrong in hpp?
                                                           VkDebugReportObjectTypeEXT,
                                                           uint64_t,
                                                           size_t,
                                                           int32_t,
                                                           const char *layerPrefix,
                                                           const char *msg,
                                                           void *) -> unsigned int {
                                                          std::cerr << layerPrefix << ": " << msg << std::endl;
                                                          return VK_FALSE;
                                                        }};
        return vkInstance.createDebugReportCallbackEXT(createInfo);
      }(vkInstance))
#endif
    {
      if (!vkInstance)
        throw std::runtime_error("vulkan : failed to create instance");
    }

    Instance(Instance const &) = delete;
    Instance(Instance &&other)
      : vkInstance(other.vkInstance)
#ifdef DEBUG_LAYERS
      , callback(other.callback)
#endif
    {
      other.vkInstance = nullptr;
    }



    ~Instance()
    {
      if (vkInstance)
	{
#ifdef DEBUG_LAYERS
	  vkInstance.destroyDebugReportCallbackEXT(callback);
#endif
	  vkInstance.destroy();
	}
    }
  };
};
