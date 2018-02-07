#pragma once


namespace magma
{
  /// Singleton for object -> device association
  template<class T>
  auto &getDeviceMapSingleton()
  {
    static std::unordered_map<T, Device<claws::NoDelete>> map;

    return map;
  }

  template<class T>
  void registerDeleter(Device<claws::NoDelete> device, T const &t)
  {
    getDeviceMapSingleton<T>()[t] = device;
  }

  template<class T>
  auto &getAssociatedDevice(T const &t)
  {
    return getDeviceMapSingleton<T>().at(t);
  }

  template<class Func>
  struct SingletonDeviceDeleter
  {
    template<class T>
    void operator()(T const &t) const
    {
      auto &map(getDeviceMapSingleton<T>);

      Func{map.at(t)}(t);
      map.erase(t);
    }
  };

  template<class T, class Deleter>
  auto toSingletonDeleter(claws::Handle<T, Deleter> &&t)
  {
    claws::Handle<T, SingletonDeviceDeleter<Deleter>> handle(std::move(t));

    registerDeleter(t.getDeleter().device, t);
    return handle;
  }
};
