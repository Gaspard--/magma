#pragma once

#include <functional>
#include <cassert>
#include <optional>

namespace magma
{
  class DynamicBuffer
  {
  public:
    using RangeId = std::pair<uint32_t, uint32_t>;

    static constexpr RangeId nullId{0u, ~0u};
  private:
    struct Range
    {
      uint32_t begin;
      uint32_t end;
    };

    struct Chunk
    {
      struct Err
      {
      };
      DeviceMemory<> deviceMemory;
      Buffer<> buffer;
      uint32_t size;
      std::vector<Range> ranges;

      auto findRange(uint32_t index) const
      {
	return std::lower_bound(ranges.begin(), ranges.end(), index, [](auto const &range, auto index)
				{
				  return range.begin < index;
				});
      }

      auto findRange(uint32_t index)
      {
	return std::lower_bound(ranges.begin(), ranges.end(), index, [](auto const &range, auto index)
				{
				  return range.begin < index;
				});
      }

      uint32_t allocate(uint32_t allocSize)
      {
	if (ranges.empty())
	  {
	    if (allocSize <= size)
	      {
		ranges.push_back({0, allocSize});
		return (ranges.back().begin);
	      }
	  }
	else
	  {
	    if (allocSize < ranges[0].begin)
	      return ranges.insert(ranges.begin(), {0, allocSize})->begin;
	    auto it(ranges.begin());
	    auto prev(it);

	    ++it;
	    for (; it != ranges.end(); prev = it++)
	      {
		if (prev->end + allocSize <= it->begin)
		  return ranges.insert(it, {prev->end, prev->end + allocSize})->begin;
		prev = it;
	      }
	    if (prev->end + allocSize < size)
	      {
		ranges.push_back({prev->end, prev->end + allocSize});
		return (ranges.back().begin);
	      }
	  }
	throw Err{};
      }

      uint32_t removeRange(uint32_t index)
      {
	if (ranges.size() > 1)
	  {
	    auto rangeIt = findRange(index);
	    uint32_t removeSize(rangeIt->end - rangeIt->begin);

	    ranges.erase(rangeIt);
	    return removeSize;
	  }
	uint32_t removeSize(ranges.back().end - ranges.back().begin);

	*this = Chunk{};
	return removeSize;
      }

      void resizeRange(uint32_t index, uint32_t newSize)
      {
	auto rangeIt(findRange(index));

	rangeIt->end = rangeIt->begin + newSize;
      }

      Range const &getRange(uint32_t index) const
      {
	auto rangeIt(findRange(index));

	assert(rangeIt != ranges.end() && rangeIt->begin == index);
	return *rangeIt;
      }
    };

    Device<claws::NoDelete> device;
    vk::PhysicalDevice physicalDevice;
    vk::BufferCreateFlags createFlags;
    vk::BufferUsageFlags usage;
    vk::MemoryPropertyFlags memoryFlags;
    std::optional<std::vector<uint32_t>> queueFamilies;
    uint32_t allocatedSize;
    uint32_t bestChunk;

    std::vector<Chunk> chunks;

  public:
    DynamicBuffer(Device<claws::NoDelete> device,
		  vk::PhysicalDevice physicalDevice,
		  vk::BufferCreateFlags createFlags,
		  vk::BufferUsageFlags usage,
		  vk::MemoryPropertyFlags memoryFlags,
		  std::optional<std::vector<uint32_t>> &&queueFamilies = {})
      : device(device)
      , physicalDevice(physicalDevice)
      , createFlags(createFlags)
      , usage(usage)
      , memoryFlags(memoryFlags)
      , queueFamilies(queueFamilies)
      , allocatedSize(0u)
      , chunks()
    {
    }

    DynamicBuffer() = default;
    DynamicBuffer(DynamicBuffer const &) = delete;
    DynamicBuffer(DynamicBuffer &&) = default;

    DynamicBuffer &operator=(DynamicBuffer const &) = delete;
    DynamicBuffer &operator=(DynamicBuffer &&) = default;
    
    auto getAvailableChunk()
    {
      for (uint32_t i(0u); i < chunks.size(); ++i)
	if (!chunks[i].size)
	  return i;
      chunks.emplace_back();
      return static_cast<uint32_t>(chunks.size() - 1);
    }

    void initChunk(Chunk &newChunk, uint32_t size)
    {
      newChunk.size = size;
      if (queueFamilies)
	newChunk.buffer = device.createBuffer(createFlags, newChunk.size, usage, *queueFamilies);
      else
	newChunk.buffer = device.createBuffer(createFlags, newChunk.size, usage);
      auto memRequirements(device.getBufferMemoryRequirements(newChunk.buffer));

      newChunk.deviceMemory = device.selectAndCreateDeviceMemory(physicalDevice, memRequirements.size, memoryFlags, memRequirements.memoryTypeBits);
      device.bindBufferMemory(newChunk.buffer, newChunk.deviceMemory, 0);
    }

    RangeId allocate(uint32_t size)
    {
      allocatedSize += size;
      try {
	for (uint32_t i(0u); i < chunks.size(); ++i)
	  {
	    try {
	      return {i, chunks[i].allocate(size)};
	    } catch (Chunk::Err const &) {}
	  }

	auto index(getAvailableChunk());
	Chunk &newChunk(chunks[index]);

	try {
	  initChunk(newChunk, allocatedSize);
	  bestChunk = index;
	} catch (...) {
	  initChunk(newChunk, size);
	}
	return {index, newChunk.allocate(size)};
      } catch (...) {
	allocatedSize -= size;
	throw;
      }
    }

    void free(RangeId index)
    {
      if (index == nullId)
	return ;
      if (index.first == chunks.size() - 1 && chunks.back().ranges.size() <= 1)
	{
	  allocatedSize -= chunks.back().ranges.front().end - chunks.back().ranges.front().begin;
	  chunks.resize(chunks.size() - 1);
	}
      else
	allocatedSize -= chunks[index.first].removeRange(index.second);
    }

    template<class PtrType>
    auto getMemory(RangeId index)
    {
      Range const &range(chunks[index.first].getRange(index.second));
      auto deleter([device = this->device, deviceMemory = DeviceMemory<claws::NoDelete>(chunks[index.first].deviceMemory)](auto data)
		   {
		     if (data)
		       device.unmapMemory(deviceMemory);
		   });
      return std::unique_ptr<PtrType, decltype(deleter)>(reinterpret_cast<std::decay_t<PtrType>>(device.mapMemory(chunks[index.first].deviceMemory, range.begin, range.end - range.begin)), deleter);
    }

    void resize(RangeId index, uint32_t size)
    {
      chunks[index.first].resizeRange(index.second, size);
    }

    magma::Buffer<claws::NoDelete> getBuffer(RangeId index)
    {
      return chunks[index.first].buffer;
    }
  };
};
