#pragma once

#include <algorithm>

#include "VulkanFormatsHandler.hpp"
#include "VulkanHandler.hpp"

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

    Surface(Instance &instance, vk::SurfaceKHR vkSurface)
      : instance(instance.vkInstance)
      , vkSurface(vkSurface)
    {}

    ~Surface()
    {
      if (instance)
        instance.destroySurfaceKHR(vkSurface);
    }

    bool isQueueFamilySuitable(vk::PhysicalDevice physicalDevice, uint32_t queueIndex)
    {
      return physicalDevice.getSurfaceSupportKHR(queueIndex, vkSurface);
    }
  };

  namespace impl
  {
    class Swapchain
    {
    protected:
      magma::Device<claws::no_delete> device;
      vk::Format format;
      vk::Extent2D currentExtent;

    public:
      ~Swapchain() = default;

      vk::SwapchainKHR vkSwapchain;

      template<class CONTAINER>
      static auto chooseImageFormat(Surface const &surface, vk::PhysicalDevice physicalDevice, CONTAINER &&formatChoices)
      {
        auto formats(physicalDevice.getSurfaceFormatsKHR(surface.vkSurface));

        if (formats[0].format == vk::Format::eUndefined)
          {
            formats[0].format = vk::Format::eR8G8B8Srgb; // yay!
            return formats[0];
          }
        FormatGroup possibillities = ([&formatChoices](FormatGroup possibillities) {
          for (auto const &pick : formatChoices)
            {
              if (auto best = (possibillities & pick))
                return best;
            }
          throw std::runtime_error("Vulkan Swapchain: No suitable format found.");
        })(FormatGroup{claws::container_view(formats.begin(), formats.end(), [](auto const &format) noexcept { return format.format; })});
        return *std::find_if(formats.begin(), formats.end(), [&possibillities](auto const &format) noexcept { return possibillities[format.format]; });
      }

      Swapchain()
        : device{}
        , format{}
        , currentExtent{}
        , vkSwapchain(nullptr)
      {}

      // TODO: refactor the fuck out of this.
      Swapchain(Surface const &surface,
                magma::Device<claws::no_delete> device,
                vk::PhysicalDevice physicalDevice,
                claws::handle<Swapchain, claws::no_delete> old)
        : device(device)
      {
        constexpr std::array<magma::FormatGroup, 3> preferedFormatRanking{
          {((vulkanFormatGroups::R8G8B8 | vulkanFormatGroups::B8G8R8) & vulkanFormatGroups::Srgb),
           ((vulkanFormatGroups::R8G8B8A8 | vulkanFormatGroups::B8G8R8A8) & vulkanFormatGroups::Srgb),
           (vulkanFormatGroups::R8G8B8A8 | vulkanFormatGroups::B8G8R8A8 | vulkanFormatGroups::R8G8B8A8 | vulkanFormatGroups::B8G8R8A8)}};
        auto format(chooseImageFormat(surface, physicalDevice, preferedFormatRanking));
        auto capabilities(physicalDevice.getSurfaceCapabilitiesKHR(surface.vkSurface));
        auto presentModes(physicalDevice.getSurfacePresentModesKHR(surface.vkSurface));

        this->format = format.format;

        constexpr std::array<vk::PresentModeKHR, 2> order{{vk::PresentModeKHR::eFifoRelaxed, vk::PresentModeKHR::eFifo}};
        auto resultIt(std::find_if(order.begin(), order.end(), [&presentModes](auto presentMode) noexcept {
          return (std::find(presentModes.begin(), presentModes.end(), presentMode) != presentModes.end());
        }));
        if (resultIt == order.end())
          throw std::runtime_error("Vulkan Swapchain: No suitable presentation mode found.");
        auto presentMode(*resultIt);

        if (!(capabilities.supportedUsageFlags & vk::ImageUsageFlagBits::eTransferDst))
          throw std::runtime_error("Vulkan Swapchain: Mising eTransferDst for clear operation.");

        this->currentExtent =
          (capabilities.currentExtent == vk::Extent2D{0xFFFFFFFFu, 0xFFFFFFFFu} ? capabilities.maxImageExtent
                                                                                : capabilities.currentExtent); // choose current or biggest extent possible;

        vk::SwapchainCreateInfoKHR createInfo({},
                                              surface.vkSurface,
                                              std::min(std::max(capabilities.minImageCount, 3u), capabilities.maxImageCount - !capabilities.maxImageCount),
                                              format.format,
                                              format.colorSpace,
                                              this->currentExtent,
                                              1,
                                              vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eColorAttachment,
                                              vk::SharingMode::eExclusive, // next 2 params unused because eExclusive
                                              0,                           // unused
                                              nullptr,                     // unused
                                              capabilities.currentTransform,
                                              vk::CompositeAlphaFlagBitsKHR::eOpaque,
                                              presentMode,
                                              true,
                                              old.vkSwapchain); // old swapchain

        vkSwapchain = device.createSwapchainKHR(createInfo);
      }

      auto const &getExtent() const
      {
        return this->currentExtent;
      }

      auto getImages()
      {
        return device.getSwapchainImagesKHR(vkSwapchain);
      }

      auto getFormat() const
      {
        return this->format;
      }

      auto getImageIndex(vk::Semaphore sem, uint64_t timeout, vk::Fence fence) const
      {
        uint32_t index;

        auto result = vkAcquireNextImageKHR(device, vkSwapchain, timeout, sem, fence, &index);
        return std::make_tuple(vk::Result{result}, index);
      }

      auto &raw()
      {
        return (vkSwapchain);
      }

      void swap(Swapchain &other)
      {
        using std::swap;

        swap(device, other.device);
        swap(format, other.format);
        swap(vkSwapchain, other.vkSwapchain);
        swap(currentExtent, other.currentExtent);
      }

      operator bool()
      {
        return vkSwapchain;
      }
    };

    inline void swap(Swapchain &lh, Swapchain &rh)
    {
      lh.swap(rh);
    }
  }

  template<class Deleter = Deleter<vk::SwapchainKHR>>
  using Swapchain = claws::handle<impl::Swapchain, Deleter>;
};
