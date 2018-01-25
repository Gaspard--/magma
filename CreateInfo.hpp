#pragma once

namespace magma
{
  template<class T>
  struct SingleValListRef
  {
    T const &val;

    constexpr auto data() const noexcept
    {
      return &val;
    }

    constexpr auto size() const noexcept
    {
      return 1;
    }
  };

  struct EmptyList{
    constexpr auto data() const noexcept
    {
      return nullptr;
    }

    constexpr auto size() const noexcept
    {
      return 0;
    }
  };

  template<class CreateInfo, bool EmptyFlags = false>
  struct StructBuilder
  {
  private:
    static std::true_type isList(magma::EmptyList);

    template<class T>
    static std::true_type isList(std::vector<T>);

    template<class T>
    static std::true_type isList(magma::SingleValListRef<T>);

    template<class T, std::size_t size>
    static std::true_type isList(std::array<T, size>);

    template<class T>
    static std::false_type isList(T);

    template<class T>
    static constexpr auto convertToTuple(T const &arg)
    {
      if constexpr (decltype(isList(arg))::value)
		     return std::make_tuple(static_cast<uint32_t>(arg.size()), arg.data());
      else
	return std::make_tuple(arg);
    }

    template<class... Args, size_t... indices>
    static constexpr auto makeImpl(std::tuple<Args...> args, std::index_sequence<indices...>)
    {
      if constexpr (!EmptyFlags)
		     return CreateInfo(std::get<indices>(args)...);
      else
	return CreateInfo({}, std::get<indices>(args)...);
    }

  public:
    template<class... T>
    static constexpr auto make(T const &... args)
    {
      auto tuple(std::tuple_cat(convertToTuple(args)...));

      return makeImpl(tuple, std::make_index_sequence<std::tuple_size<decltype(tuple)>::value>());
    }    
  };

  template<class T>
  constexpr auto asListRef(T const &val) noexcept
  {
    return SingleValListRef<T>{val};
  }

  template<class T, class Deleter>
  constexpr auto asListRef(claws::Handle<T, Deleter> const &val) noexcept
  {
    return SingleValListRef<T>{val};
  }
};
