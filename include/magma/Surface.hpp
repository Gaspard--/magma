#pragma once

namespace magma
{
  class Surface
  {
  public:
    vk::Instance instance;
    vk::SurfaceKHR vkSurface;

    Surface()
      : instance(nullptr)
      , vkSurface(nullptr)
    {}

    Surface(Instance const &instance, vk::SurfaceKHR vkSurface)
      : instance(instance.vkInstance)
      , vkSurface(vkSurface)
    {}

    Surface(Surface const &) = delete;

    ~Surface()
    {
      if (instance)
        instance.destroySurfaceKHR(vkSurface);
    }

    bool isQueueFamilySuitable(vk::PhysicalDevice physicalDevice, uint32_t queueIndex) const
    {
      return physicalDevice.getSurfaceSupportKHR(queueIndex, vkSurface);
    }
  };
}
