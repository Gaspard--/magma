#pragma once

#include "magma/Device.hpp"

#define INIT_DEVICE(REQUIRED_FLAGS)					\
  magma::Instance instance{};						\
  magma::Device<> device;						\
									\
  auto [physicalDevice, selectedQueueFamily] = instance.selectQueue([](auto const &queueFamilyProperties, auto const &physicalDevice, auto queueIndex) \
								    {	\
								      return (queueFamilyProperties.queueFlags & REQUIRED_FLAGS); \
								    },	\
								    [](auto const &, auto const &) { return true; }); \
  {									\
    float priority[1]{1.0f};						\
    vk::DeviceQueueCreateInfo deviceQueueCreateInfo{{}, selectedQueueFamily, 1, priority}; \
    device = magma::Device<>(physicalDevice,				\
			     std::vector<vk::DeviceQueueCreateInfo>({deviceQueueCreateInfo}), \
			     std::vector<char const *>({VK_KHR_SWAPCHAIN_EXTENSION_NAME})); \
  }									\

