#pragma once

#include "util/ContextfulContainer.hpp"
#include "vulkan/vulkan.hpp"

namespace magma
{
  struct CommandBufferContext
  {
    vk::Device device;
    vk::CommandPool commandPool;
    uint32_t queueFamilyIndex;
  };

  template<class CommandBufferType>
  using CommandBufferGroupContainer = ContextfulContainer<CommandBufferType, std::vector<vk::CommandBuffer>, CommandBufferContext>;

  template<class CommandBufferType>
  class CommandBufferGroup : public CommandBufferGroupContainer<CommandBufferType>
  {
  public:
    CommandBufferGroup(CommandBufferContext const &context, std::vector<vk::CommandBuffer> &&commandBuffers)
      : CommandBufferGroupContainer<CommandBufferType>{context, std::move(commandBuffers)}
    {
    }

    CommandBufferGroup(CommandBufferGroup const &) = delete;
    CommandBufferGroup(CommandBufferGroup &&) = default;

    ~CommandBufferGroup()
    {
      if (CommandBufferGroupContainer<CommandBufferType>::container.size())
	CommandBufferGroupContainer<CommandBufferType>::context.device.freeCommandBuffers(CommandBufferGroupContainer<CommandBufferType>::context.commandPool, CommandBufferGroupContainer<CommandBufferType>::container.size(), CommandBufferGroupContainer<CommandBufferType>::container.data());
    }
  };

  class CommandBuffer
  {
  protected:
    vk::CommandBuffer commandBuffer;

  public:
    CommandBuffer(CommandBufferContext,
		  vk::CommandBuffer commandBuffer)
      : commandBuffer(commandBuffer)
    {
    }

    void end()
    {
      commandBuffer.end();
    }

    void reset(vk::CommandBufferResetFlags flags)
    {
      commandBuffer.reset(flags);
    }
  };

  class SecondaryCommandBuffer : public CommandBuffer
  {
    void begin(vk::CommandBufferUsageFlags flags,
	       vk::CommandBufferInheritanceInfo const &pInheritanceInfo)
    {
      commandBuffer.begin(vk::CommandBufferBeginInfo{flags, &pInheritanceInfo});
    }    
  };

  class PrimaryCommandBuffer : public CommandBuffer
  {
    void begin(vk::CommandBufferUsageFlags flags)
    {
      commandBuffer.begin(vk::CommandBufferBeginInfo{flags, nullptr});
    }

    void execBuffers(CommandBufferGroup<SecondaryCommandBuffer> &secondaryCommands)
    {
      commandBuffer.executeCommands(secondaryCommands.container);
    }
  };

  class CommandPool : public vk::CommandPool
  {
    vk::Device device;
    uint32_t queueFamilyIndex;
  public:
    CommandPool(vk::Device device, uint32_t queueFamilyIndex, vk::CommandPool commandBuffer)
      : vk::CommandPool(commandBuffer)
      , device(device)
      , queueFamilyIndex(queueFamilyIndex)
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

    auto allocatePrimaryCommandBuffers(uint32_t commandBufferCount)
    {
      vk::CommandBufferAllocateInfo info{*this, vk::CommandBufferLevel::ePrimary, commandBufferCount};

      return CommandBufferGroup<PrimaryCommandBuffer>{{device, *this, queueFamilyIndex}, device.allocateCommandBuffers(info)};
    }

    auto allocateSecondaryCommandBuffers(uint32_t commandBufferCount)
    {
      vk::CommandBufferAllocateInfo info{*this, vk::CommandBufferLevel::eSecondary, commandBufferCount};

      return CommandBufferGroup<SecondaryCommandBuffer>{{device, *this, queueFamilyIndex}, device.allocateCommandBuffers(info)};
    }
  };
};
