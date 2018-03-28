#pragma once

#include "claws/handle_types.hpp"
#include "magma/VulkanHandler.hpp"

namespace magma
{
  namespace impl
  {
    class Device : public vk::Device
    {
    protected:
      ~Device() = default;

    public:
      Device()
        : vk::Device(nullptr)
      {}

      Device(vk::PhysicalDevice physicalDevice,
             std::vector<vk::DeviceQueueCreateInfo> const &deviceQueueCreateInfos,
             std::vector<char const *> const &extensions = {})
        : vk::Device([](vk::PhysicalDevice physicalDevice,
                        std::vector<vk::DeviceQueueCreateInfo> const &deviceQueueCreateInfos,
                        std::vector<char const *> const &extensions) {
          vk::DeviceCreateInfo deviceCreateInfo{{},
                                                static_cast<unsigned>(deviceQueueCreateInfos.size()),
                                                deviceQueueCreateInfos.data(),
                                                0,
                                                nullptr,
                                                static_cast<unsigned>(extensions.size()),
                                                extensions.data()};

          return physicalDevice.createDevice(deviceCreateInfo);
        }(physicalDevice, deviceQueueCreateInfos, extensions))
      {}

      using vk::Device::Device;

      void swap(Device &other)
      {
        using std::swap;

        swap(static_cast<vk::Device &>(*this), static_cast<vk::Device &>(other));
      }

      auto createCommandPool(vk::CommandPoolCreateFlags flags, uint32_t queueFamilyIndex) const;

      template<class Container>
      auto createShaderModule(Container const &) const;

      auto createShaderModule(std::istream &input) const;

      auto createFramebuffer(claws::handle<vk::RenderPass, claws::no_delete> renderPass,
                             std::vector<vk::ImageView> const &attachements,
                             uint32_t width,
                             uint32_t height,
                             uint32_t layers) const;

      auto createFence(vk::FenceCreateFlags flags) const;

      using vk::Device::getFenceStatus;
      using vk::Device::resetFences;
      using vk::Device::waitForFences;

      auto createSemaphore() const;

      auto createEvenv() const;

      using vk::Device::getEventStatus;
      using vk::Device::setEvent;
      using vk::Device::resetEvent;

      auto createPipeline(vk::GraphicsPipelineCreateInfo const &createInfo) const;

      auto createRenderPass(vk::RenderPassCreateInfo const &renderPassCreateInfo) const;

      auto getRenderAreaGranularity(claws::handle<vk::RenderPass, claws::no_delete> renderPass) const;

      auto createImageView(vk::ImageViewCreateFlags flags,
                           vk::Image image,
                           vk::ImageViewType type,
                           vk::Format format,
                           vk::ComponentMapping components,
                           vk::ImageSubresourceRange subresourceRange) const;

      auto createPipelineLayout(vk::PipelineLayoutCreateFlags flags,
                                std::vector<vk::DescriptorSetLayout> const &setLayouts,
                                std::vector<vk::PushConstantRange> const &pushConstantRanges) const;
      auto createImage2D(vk::ImageCreateFlags flags,
                         vk::Format format,
                         std::array<uint32_t, 2> extent,
                         vk::SampleCountFlagBits samples,
                         vk::ImageTiling tiling,
                         vk::ImageUsageFlags usage,
                         std::vector<uint32_t> indices,
                         vk::ImageLayout layout) const;

      auto createImage2D(vk::ImageCreateFlags flags,
                         vk::Format format,
                         std::array<uint32_t, 2> extent,
                         vk::SampleCountFlagBits samples,
                         vk::ImageTiling tiling,
                         vk::ImageUsageFlags usage,
                         vk::ImageLayout layout) const;

      auto createBuffer(vk::BufferCreateFlags flags, vk::DeviceSize size, vk::BufferUsageFlags usage, std::vector<uint32_t> const &queueFamilies) const;
      auto createBuffer(vk::BufferCreateFlags flags, vk::DeviceSize size, vk::BufferUsageFlags usage) const;

      auto createBufferView(vk::BufferViewCreateFlags flags,
                            claws::handle<vk::Buffer, claws::no_delete> buffer,
                            vk::Format format,
                            vk::DeviceSize offset,
                            vk::DeviceSize size) const;

      auto createDeviceMemory(vk::DeviceSize size, uint32_t typeIndex) const;
      auto selectAndCreateDeviceMemory(vk::PhysicalDevice physicalDevice,
                                       vk::DeviceSize size,
                                       vk::MemoryPropertyFlags memoryFlags,
                                       uint32_t memoryTypeIndexMask) const;

      auto createDescriptorSetLayout(std::vector<vk::DescriptorSetLayoutBinding> const &bindings) const;
      auto createDescriptorPool(std::uint32_t maxSets, std::vector<vk::DescriptorPoolSize> const &size) const;
      auto createDescriptorSets(vk::DescriptorPool descriptorPool, std::vector<vk::DescriptorSetLayout> const &setLayout) const;

      template<class T, std::enable_if_t<std::is_same_v<typename T::value_type, vk::WriteDescriptorSet>> * = nullptr>
      void updateDescriptorSets(T const &writeDescriptorSets);
      template<class T, std::enable_if_t<std::is_same_v<typename T::value_type, vk::CopyDescriptorSet>> * = nullptr>
      void updateDescriptorSets(T const &copyDescriptorSets);

      auto createSampler(vk::Filter magFilter,
                         vk::Filter minFilter,
                         vk::SamplerMipmapMode mipmapMode,
                         vk::SamplerAddressMode adressModeU,
                         vk::SamplerAddressMode adressModeV,
                         vk::SamplerAddressMode adressModeW,
                         float mipLoadBias,
                         vk::Bool32 anisotropyEnable,
                         float maxAnisotropy,
                         vk::Bool32 compareEnable,
                         vk::CompareOp compareOp,
                         float minLod,
                         float maxLod,
                         vk::BorderColor borderColor,
                         vk::Bool32 unnormalizedCoordinates) const;

      auto createEvent(void) const;

      using vk::Device::operator bool;
      using vk::Device::operator!;

      template<class... Params>
      decltype(std::declval<vk::Device>().destroy(std::declval<Params>()...)) destroy(Params &&...) = delete;

      struct DeviceDeleter
      {
        void operator()(vk::Device const &device) const
        {
          if (device)
            device.destroy();
        }
      };
    };

    static inline void swap(impl::Device &lh, impl::Device &rh) noexcept
    {
      lh.swap(rh);
    }
  }

  template<class Deleter = impl::Device::DeviceDeleter>
  using Device = claws::handle<impl::Device, Deleter>;
};
