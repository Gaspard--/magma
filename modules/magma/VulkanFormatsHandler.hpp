#pragma once

#include <type_traits>

#include <claws/container/array_ops.hpp>

#include "vulkan/vulkan.hpp"

namespace magma
{
  struct FormatGroup
  {
    static constexpr uint32_t format_count{static_cast<uint32_t>(vk::Format::eAstc12x12SrgbBlock) + 1u}; // 184 + 1

    std::array<uint64_t, (format_count >> 6u) + 1u> bits;

#define FORMAT_GROUP_OP(OP)                                                                                                                                    \
  constexpr FormatGroup &operator OP##=(FormatGroup const &other) noexcept                                                                                     \
  {                                                                                                                                                            \
    using namespace claws::array_ops;                                                                                                                          \
    bits OP## = other.bits;                                                                                                                                    \
    return *this;                                                                                                                                              \
  };                                                                                                                                                           \
                                                                                                                                                               \
  constexpr FormatGroup operator OP(FormatGroup const &other) const noexcept                                                                                   \
  {                                                                                                                                                            \
    FormatGroup result{*this};                                                                                                                                 \
                                                                                                                                                               \
    result OP## = other;                                                                                                                                       \
    return result;                                                                                                                                             \
  }

    FORMAT_GROUP_OP(|);
    FORMAT_GROUP_OP(&);
    FORMAT_GROUP_OP (^);

    constexpr FormatGroup() noexcept
      : bits{}
    {}

    constexpr FormatGroup(FormatGroup const &other) noexcept
      : bits(other.bits)
    {}

    constexpr FormatGroup operator=(FormatGroup const &other) noexcept
    {
      this->bits = other.bits;
      return *this;
    }

    template<class CONTAINER>
    constexpr FormatGroup(CONTAINER const &formats) noexcept
      : FormatGroup{}
    {
      for (auto format : formats)
        (*this)[format] = true;
    }

    constexpr FormatGroup operator~() const noexcept
    {
      using namespace claws::array_ops;
      FormatGroup result{*this};

      result.bits = ~result.bits;
      return result;
    }

    constexpr bool operator[](uint32_t format) const noexcept
    {
      return (bits[format >> 6] >> (format & 63)) & 1u;
    }

    constexpr bool operator[](vk::Format format) const noexcept
    {
      return (*this)[static_cast<uint32_t>(format)];
    }

    constexpr auto operator[](uint32_t format) noexcept
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
          formatGroup.bits[format >> 6] ^= (val * ~static_cast<uint64_t>(0) ^ formatGroup.bits[format >> 6]) & (1ul << (format & 63));
          return *this;
        }

        constexpr operator bool()
        {
          return (formatGroup.bits[format >> 6] >> (format & 63)) & 1u;
        }
      };

      return BoolProxy{*this, format};
    }

    constexpr auto operator[](vk::Format format) noexcept
    {
      return (*this)[static_cast<uint32_t>(format)];
    }

    operator bool() const noexcept
    {
      return bits != (decltype(bits){});
    }
  };

  namespace vulkanFormatGroups
  {
    constexpr FormatGroup makeFormatGroupFromIncludedRange(vk::Format min, vk::Format max) noexcept
    {
      FormatGroup result{};

      for (auto i(static_cast<uint32_t>(min)); i != static_cast<uint32_t>(max) + 1u; ++i)
        result[i] = true;
      return result;
    }

    constexpr FormatGroup makeFormatGroupFromIncludedRange(vk::Format min, vk::Format max, uint32_t skip) noexcept
    {
      FormatGroup result{};

      for (auto i(static_cast<uint32_t>(min)); i <= static_cast<uint32_t>(max); i += skip)
        result[i] = true;
      return result;
    }

    constexpr FormatGroup Srgb(makeFormatGroupFromIncludedRange(vk::Format::eR8Srgb, vk::Format::eB8G8R8A8Srgb, 7));

#define FORMAT_GROUP_8_AND_16_BIT_NUMERIC_FORMAT(NAME)                                                                                                         \
  constexpr FormatGroup NAME(makeFormatGroupFromIncludedRange(vk::Format::eR8##NAME, vk::Format::eB8G8R8A8##NAME, 7)                                           \
                             | makeFormatGroupFromIncludedRange(vk::Format::eR16##NAME, vk::Format::eR16G16B16A16##NAME, 7))

    FORMAT_GROUP_8_AND_16_BIT_NUMERIC_FORMAT(Unorm);
    FORMAT_GROUP_8_AND_16_BIT_NUMERIC_FORMAT(Snorm);
    FORMAT_GROUP_8_AND_16_BIT_NUMERIC_FORMAT(Uscaled);
    FORMAT_GROUP_8_AND_16_BIT_NUMERIC_FORMAT(Sscaled);

#define FORMAT_GROUP_8_16_32_AND_64_BIT_NUMERIC_FORMAT(NAME)                                                                                                   \
  constexpr FormatGroup NAME(makeFormatGroupFromIncludedRange(vk::Format::eR8##NAME, vk::Format::eB8G8R8A8##NAME, 7)                                           \
                             | makeFormatGroupFromIncludedRange(vk::Format::eR16##NAME, vk::Format::eR16G16B16A16##NAME, 7)                                    \
                             | makeFormatGroupFromIncludedRange(vk::Format::eR32##NAME, vk::Format::eR32G32B32A32##NAME, 3)                                    \
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
}
