#pragma once

#include <type_traits>
#include "vulkan/vulkan.hpp"
#include "Vect.hpp"
#include "IteratorUtil.hpp"

struct FormatGroup
{
  static constexpr uint32_t format_count{(uint32_t)vk::Format::eAstc12x12SrgbBlock + 1u}; // 184 + 1

  Vect<(format_count >> 6u) + 1u, uint64_t> bits;

#define FORMAT_GROUP_OP(OP)						\
  constexpr FormatGroup &operator OP##=(FormatGroup const &other)	\
  {									\
    bits OP##= other.bits;						\
    return *this;							\
  };									\
									\
  constexpr FormatGroup operator OP(FormatGroup const &other) const	\
  {									\
    FormatGroup result{*this};						\
									\
    result OP##= other;							\
    return result;							\
  }

  FORMAT_GROUP_OP(|);
  FORMAT_GROUP_OP(&);
  FORMAT_GROUP_OP(^);

  constexpr FormatGroup()
  : bits{}
  {
  }

  constexpr FormatGroup(FormatGroup const &other)
  : bits(other.bits)
  {
  }

  constexpr FormatGroup operator=(FormatGroup const &other)
  {
    this->bits = other.bits;
    return *this;
  }

  template<class CONTAINER>
  constexpr FormatGroup(CONTAINER const &formats)
  : FormatGroup{}
  {
    for (auto format : formats)
      (*this)[format] = true;
  }

  constexpr FormatGroup operator~() const
  {
    FormatGroup result{*this};

    result.bits = ~result.bits;
    return result;
  }

  constexpr bool operator[](uint32_t format) const
  {
    return (bits[format >> 6] >> (format & 63)) & 1u;
  }

  constexpr bool operator[](vk::Format format) const
  {
    return (*this)[(uint32_t)format];
  }

  constexpr auto operator[](uint32_t format)
  {
    struct BoolProxy
    {
      FormatGroup &formatGroup;
      uint32_t format;

      constexpr BoolProxy &operator=(uint32_t val)
      {
	val = !!val;
	// x ^ y == (x | y) & ~(x & y)
	// a ^ (~a & b) = (a | (~a & b)) & ~(a & (~a & b)) = (a | ~a & b) = a | b
	// a ^ (a & b) = (a | (a & b)) & ~(a & (a & b)) = (a | (a & b)) & (~a | ~a | ~b) = (a | (a & b)) & ~(a & b) = a & ~(a & b) = a & ~b
	formatGroup.bits[format >> 6] ^= (val * ~(uint64_t)0 ^ formatGroup.bits[format >> 6]) & ((uint64_t)1 << (format & 63));
	return *this;
      }

      constexpr operator bool()
      {
	return (formatGroup.bits[format >> 6] >> (format & 63)) & 1u;
      }
    };

    return BoolProxy{*this, format};
  }

  constexpr auto operator[](vk::Format format)
  {
    return (*this)[(uint32_t)format];
  }

  operator bool() const
  {
    return !bits.equals(decltype(bits){});
  }
};

namespace vulkanFormatGroups
{
  constexpr FormatGroup makeFormatGroupFromIncludedRange(vk::Format min, vk::Format max)
  {
    FormatGroup result{};

    for (uint32_t i((uint32_t)min); i != (uint32_t)max + 1u; ++i)
      result[i] = true;
    return result;
  }

  constexpr FormatGroup makeFormatGroupFromIncludedRange(vk::Format min, vk::Format max, uint32_t skip)
  {
    FormatGroup result{};

    for (uint32_t i((uint32_t)min); i <= (uint32_t)max; i += skip)
      result[i] = true;
    return result;
  }

  constexpr FormatGroup Srgb(makeFormatGroupFromIncludedRange(vk::Format::eR8Srgb, vk::Format::eB8G8R8A8Srgb, 7));
  
#define FORMAT_GROUP_8_AND_16_BIT_NUMERIC_FORMAT(NAME)			\
  constexpr FormatGroup NAME(makeFormatGroupFromIncludedRange(vk::Format::eR8##NAME, vk::Format::eB8G8R8A8##NAME, 7) \
			     | makeFormatGroupFromIncludedRange(vk::Format::eR16##NAME, vk::Format::eR16G16B16A16##NAME, 7))

  FORMAT_GROUP_8_AND_16_BIT_NUMERIC_FORMAT(Unorm);
  FORMAT_GROUP_8_AND_16_BIT_NUMERIC_FORMAT(Snorm);
  FORMAT_GROUP_8_AND_16_BIT_NUMERIC_FORMAT(Uscaled);
  FORMAT_GROUP_8_AND_16_BIT_NUMERIC_FORMAT(Sscaled);
  
#define FORMAT_GROUP_8_16_32_AND_64_BIT_NUMERIC_FORMAT(NAME)		\
  constexpr FormatGroup NAME(makeFormatGroupFromIncludedRange(vk::Format::eR8##NAME, vk::Format::eB8G8R8A8##NAME, 7) \
			     | makeFormatGroupFromIncludedRange(vk::Format::eR16##NAME, vk::Format::eR16G16B16A16##NAME, 7) \
			     | makeFormatGroupFromIncludedRange(vk::Format::eR32##NAME, vk::Format::eR32G32B32A32##NAME, 3) \
			     | makeFormatGroupFromIncludedRange(vk::Format::eR64##NAME, vk::Format::eR64G64B64A64##NAME, 3))

  FORMAT_GROUP_8_16_32_AND_64_BIT_NUMERIC_FORMAT(Uint);
  FORMAT_GROUP_8_16_32_AND_64_BIT_NUMERIC_FORMAT(Sint);

  constexpr FormatGroup Sfloat(makeFormatGroupFromIncludedRange(vk::Format::eR16Sfloat, vk::Format::eR16G16B16A16Sfloat, 7)
			       | makeFormatGroupFromIncludedRange(vk::Format::eR32Sfloat, vk::Format::eR32G32B32A32Sfloat, 3)
			       | makeFormatGroupFromIncludedRange(vk::Format::eR64Sfloat, vk::Format::eR64G64B64A64Sfloat, 3));

#define FORMAT_GROUP_8BIT_FORMAT(FMT) constexpr FormatGroup FMT(makeFormatGroupFromIncludedRange(vk::Format::e##FMT##Unorm, vk::Format::e##FMT##Srgb))

  FORMAT_GROUP_8BIT_FORMAT(R8);
  FORMAT_GROUP_8BIT_FORMAT(R8G8);
  FORMAT_GROUP_8BIT_FORMAT(R8G8B8);
  FORMAT_GROUP_8BIT_FORMAT(B8G8R8);
  FORMAT_GROUP_8BIT_FORMAT(R8G8B8A8);
  FORMAT_GROUP_8BIT_FORMAT(B8G8R8A8);

#define FORMAT_GROUP_16BIT_FORMAT(FMT) constexpr FormatGroup FMT(makeFormatGroupFromIncludedRange(vk::Format::e##FMT##Unorm, vk::Format::e##FMT##Sfloat))
  
  FORMAT_GROUP_16BIT_FORMAT(R16);
  FORMAT_GROUP_16BIT_FORMAT(R16G16);
  FORMAT_GROUP_16BIT_FORMAT(R16G16B16);
  FORMAT_GROUP_16BIT_FORMAT(R16G16B16A16);

#define FORMAT_GROUP_32BIT_FORMAT(FMT) constexpr FormatGroup FMT(makeFormatGroupFromIncludedRange(vk::Format::e##FMT##Uint, vk::Format::e##FMT##Sfloat))
  
  FORMAT_GROUP_32BIT_FORMAT(R32);
  FORMAT_GROUP_32BIT_FORMAT(R32G32);
  FORMAT_GROUP_32BIT_FORMAT(R32G32B32);
  FORMAT_GROUP_32BIT_FORMAT(R32G32B32A32);

#define FORMAT_GROUP_64BIT_FORMAT(FMT) constexpr FormatGroup FMT(makeFormatGroupFromIncludedRange(vk::Format::e##FMT##Uint, vk::Format::e##FMT##Sfloat))
  
  FORMAT_GROUP_64BIT_FORMAT(R64);
  FORMAT_GROUP_64BIT_FORMAT(R64G64);
  FORMAT_GROUP_64BIT_FORMAT(R64G64B64);
  FORMAT_GROUP_64BIT_FORMAT(R64G64B64A64);
}
