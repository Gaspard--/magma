#include "SetupMacro.hpp"
#include "magma/CommandBuffer.hpp"

int allocateBuffers(magma::CommandPool<claws::no_delete> commandPool)
{
  magma::CommandBufferGroup<magma::PrimaryCommandBuffer> commandBufferGroup(commandPool.allocatePrimaryCommandBuffers(1));
  magma::CommandBufferGroup<magma::SecondaryCommandBuffer> secondaryCommandBufferGroup(commandPool.allocateSecondaryCommandBuffers(10));

  secondaryCommandBufferGroup[0].begin(vk::CommandBufferUsageFlagBits::eOneTimeSubmit, vk::CommandBufferInheritanceInfo{nullptr, 0, nullptr, false, {}, {}});
  secondaryCommandBufferGroup[0].end();

  commandBufferGroup[0].begin(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
  commandBufferGroup[0].execBuffers(secondaryCommandBufferGroup);
  commandBufferGroup[0].end();
}

int main()
{
  INIT_DEVICE(vk::QueueFlagBits::eGraphics);

  magma::CommandPool<> commandPool(device.createCommandPool(vk::CommandPoolCreateFlagBits::eResetCommandBuffer, selectedQueueFamily));
  magma::CommandPool<> commandPoolNoReset(device.createCommandPool({}, selectedQueueFamily));

  allocateBuffers(commandPool);
  allocateBuffers(commandPoolNoReset);
}
