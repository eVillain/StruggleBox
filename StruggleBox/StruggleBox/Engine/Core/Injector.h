#pragma once

#include "ArenaOperators.h"
#include "Allocator.h"

#include <unordered_map>
#include <memory>
#include <functional>
#include <cassert>
#include <mutex>
#include <typeinfo>

//
// IoC Injector
//
// Idea and code based upon:
// http://www.codeproject.com/Articles/567981/AnplusIOCplusContainerplususingplusVariadicplusTem
// Naming loosely based on good old RobotLegs :)
//

class Injector
{
public:
	Injector(Allocator& allocator) : m_allocator(allocator) {}
	~Injector()
	{
		//Log::Debug("Injector destructor called at %p", this);
		if (_typesToInstances.size())
		{
			Log::Warn("Injector still contained %lu type-to-instance mappings!", _typesToInstances.size());
		}
		if (_typesToFactories.size())
		{
			Log::Warn("Injector still contained %lu type-to-factory mappings!", _typesToFactories.size());
		}
		if (_interfacesToInstanceGetters.size())
		{
			Log::Warn("Injector still contained %lu interface-to-instance mappings!", _interfacesToInstanceGetters.size());
		}
	}

	/// Does this injector have a mapping for the given type?
	template <typename T>
	bool hasMapping()
	{
		return hasTypeToInstanceMapping<T>() | hasTypeToFactoryMapping<T>() | hasInterfaceToInstanceMapping<T>();
	}

	/// Maps a class in the injector. A unique instance of the mapped class
	/// will be provided for each injection request.
	/// Mapping a previously mapped class will replace the old mapping.
	template <typename T, typename... Dependencies>
	Injector& map()
	{
		std::lock_guard<std::recursive_mutex> lockGuard{ _mutex };

		auto creator = [this]() -> T*
		{
			return CUSTOM_NEW(T, m_allocator)(getInstance<Dependencies>()...);
		};

		_typesToFactories.insert(std::pair<size_t, std::function<void*()>>{typeid(T).hash_code(), creator});

		return *this;
	}

	/// Maps a specific instance of an object in the injector. When an
	/// object of this class is requested the provided instance will fulfill
	/// the injection request.
	/// Note: Since you have manually created the instance it will not 
	/// get any dependencies injected into it.
	/// Warning: The instance will NOT be destroyed upon unmapping as it is not owned by the injector.
	template <typename T>
	Injector& mapInstance(T& instance)
	{
		std::lock_guard<std::recursive_mutex> lockGuard{ _mutex };

		//Log::Info("Injector::mapInstance %s", typeid(T).name());
		IHolder* holder = CUSTOM_NEW(Holder<T>, m_allocator)(instance, false);

		_typesToInstances.insert(std::pair<size_t, IHolder*>{typeid(T).hash_code(), holder});

		return *this;
	}

	/// Maps a class as a singleton in the injector. A single instance
	/// will be used to fulfill every injection request for this class.
	template <typename T, typename... Dependencies>
	Injector& mapSingleton()
	{
		std::lock_guard<std::recursive_mutex> lockGuard{ _mutex };

		//Log::Info("Injector::mapSingleton %s", typeid(T).name());
		T* instance = CUSTOM_NEW(T, m_allocator)(getInstance<Dependencies>()...);

		IHolder* holder = CUSTOM_NEW(Holder<T>, m_allocator)(*instance, true);

		_typesToInstances.insert(std::pair<size_t, IHolder*>{typeid(T).hash_code(), holder});

		return *this;
		//return mapInstance<T>(*instance);
	}

	/// Maps a class as a singleton of an interface class. When an object
	/// of the interface class is requested a single instance of the
	/// concrete class will fulfill the injection request.
	template <typename Interface, typename ConcreteClass, typename... Dependencies>
	Injector& mapSingletonOf()
	{
		std::lock_guard<std::recursive_mutex> lockGuard{ _mutex };

		// Ensure we have the concrete class registered as a singleton first
		if (!hasTypeToInstanceMapping<ConcreteClass>())
		{
			mapSingleton<ConcreteClass, Dependencies...>();
		}

		_interfacesToInstanceGetters.insert(std::pair<size_t, size_t>{typeid(Interface).hash_code(), typeid(ConcreteClass).hash_code()});

		return *this;
	}

	/// Maps a class as an interface of a previously mapped singleton. When
	/// an object of the interface class is requested a single instance of the
	/// concrete class will fulfill the injection request.
	template <typename Interface, typename RegisteredConcreteClass>
	Injector& mapInterfaceToType()
	{
		std::lock_guard<std::recursive_mutex> lockGuard{ _mutex };

		// Ensure we have the concrete class registered as a singleton first
		if (!hasTypeToInstanceMapping<RegisteredConcreteClass>() &&
			!hasTypeToFactoryMapping<RegisteredConcreteClass>())
		{
			assert(false && "One of your injected dependencies isn't mapped, please check your mappings.");
		}

		_interfacesToInstanceGetters.insert(std::pair<size_t, size_t>{typeid(Interface).hash_code(), typeid(RegisteredConcreteClass).hash_code()});

		return *this;
	}

	/// Returns an instance of the given type if a mapping exists.
	template <typename T>
	T& getInstance()
	{
		std::lock_guard<std::recursive_mutex> lockGuard{ _mutex };

		// Try getting registered singleton or instance.
		if (hasTypeToInstanceMapping<T>())
		{
			// get as reference to avoid refcount increment
			IHolder* iholder = _typesToInstances.at(typeid(T).hash_code());

			auto holder = dynamic_cast<Holder<T>*>(iholder);
			return holder->_instance;
		} // Otherwise attempt getting the creator and act as factory.
		else if (hasTypeToFactoryMapping<T>())
		{
			auto& creator = _typesToFactories.at(typeid(T).hash_code());

			return {*(T*)creator()};
		}
		else if (!hasInterfaceToInstanceMapping<T>())
		{
			// If you debug, in some debuggers (e.g Apple's lldb in Xcode) it will breakpoint in this assert
			// and by looking in the stack trace you'll be able to see which class you forgot to map.
			assert(false && "One of your injected dependencies isn't mapped, please check your mappings.");
		}

		const size_t concreteTypeHash = _interfacesToInstanceGetters.at(typeid(T).hash_code());
		IHolder* iholder = _typesToInstances[concreteTypeHash];
		Holder<T>* holder = static_cast<Holder<T>*>(iholder);

		return holder->_instance;
	}

	template <typename T, typename... Dependencies>
	T& instantiateUnmapped()
	{
		return *CUSTOM_NEW(T, m_allocator)(getInstance<Dependencies>()...);
	}

	template <typename T>
	void unmap()
	{
		std::lock_guard<std::recursive_mutex> lockGuard{_mutex};

		// Try getting registered singleton or instance.
		if (hasTypeToInstanceMapping<T>())
		{
			const size_t typeHash = typeid(T).hash_code();
			IHolder* iholder = _typesToInstances.at(typeHash);
			_typesToInstances.erase(typeHash);
			Holder<T>* holder = dynamic_cast<Holder<T>*>(iholder);
			T& instance = holder->_instance;
			if (holder->_ownedByInjector)
			{
				CUSTOM_DELETE(&instance, m_allocator);
			}
			CUSTOM_DELETE(iholder, m_allocator);
		} // Otherwise attempt getting the creator and act as factory.
		else if (hasTypeToFactoryMapping<T>())
		{
			_typesToFactories.erase(typeid(T).hash_code());
		}
		else if (hasInterfaceToInstanceMapping<T>())
		{
			_interfacesToInstanceGetters.erase(typeid(T).hash_code());
		}
		else
		{
			Log::Warn("Injector::unmap failed to unmap %s", typeid(T).name());
		}
	}

	void clear()
	{
		for (const auto& pair : _typesToInstances)
		{
			IHolder* holder = pair.second;
			CUSTOM_DELETE(holder, m_allocator);
		}
		_typesToInstances.clear();
		_typesToFactories.clear();
		_interfacesToInstanceGetters.clear();
	}

private:

	struct IHolder
	{
		virtual ~IHolder() = default;
	};

	template <typename T>
	struct Holder : public IHolder
	{
		Holder(T& instance, bool ownedByInjector) : _instance(instance), _ownedByInjector(ownedByInjector)
		{}

		T& _instance;
		bool _ownedByInjector;
	};

	Allocator& m_allocator;
	// Holds instances - keeps singletons and custom registered instances
	std::unordered_map<size_t, IHolder*> _typesToInstances;
	// Holds creators used to instantiate a type
	std::unordered_map<size_t, std::function<void*()>> _typesToFactories;
	// Holds interface mappings used to get concrete instances
	std::unordered_map<size_t, size_t> _interfacesToInstanceGetters;
	
	std::recursive_mutex _mutex;

	/// Check if we have a mapped singleton or instance.
	template <typename T>
	inline bool hasTypeToInstanceMapping() {
		return _typesToInstances.find(typeid(T).hash_code()) != _typesToInstances.end();
	}
	/// Check if we have a mapped factory.
	template <typename T>
	inline bool hasTypeToFactoryMapping() {
		return _typesToFactories.find(typeid(T).hash_code()) != _typesToFactories.end();
	}
	/// Check if we have an interface type mapped to an instance.
	template <typename T>
	inline bool hasInterfaceToInstanceMapping() {
		return _interfacesToInstanceGetters.find(typeid(T).hash_code()) != _interfacesToInstanceGetters.end();
	}
};
