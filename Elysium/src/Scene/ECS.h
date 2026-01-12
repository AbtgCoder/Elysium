#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <type_traits>
#include <typeindex>

namespace ECS
{
	struct Entity
	{
		uint32_t index;
		uint32_t generation;

		bool operator == (const Entity& other) const
		{
			return index == other.index && generation == other.generation;
		}
	};

	constexpr uint32_t ENTITY_INDEX_MASK = 0x00FFFFFF;
	constexpr uint32_t ENTITY_GENERATION_SHIFT = 24;

	inline uint32_t ToEntityID(Entity e)
	{
		return (e.generation <<  ENTITY_GENERATION_SHIFT) | (e.index & ENTITY_INDEX_MASK);
	}

	inline Entity GetEntityFromEntityID(uint32_t id)
	{
		return {id & ENTITY_INDEX_MASK, id >> ENTITY_GENERATION_SHIFT};
	}

	static constexpr Entity NullEntity{ UINT32_MAX, UINT32_MAX };

	struct IComponentStorage
	{
		virtual ~IComponentStorage() = default;
		virtual void remove(Entity e) = 0;
		virtual bool contains(Entity e) const = 0;
		virtual size_t size() const = 0;
	};

	template<typename T>
	class ComponentStorage : public IComponentStorage
	{
	public:
		template<typename... Args>
		T& emplace(Entity e, Args&&... args)
		{
			if (contains(e))
				return data[sparse[e.index]];

			if (e.index >= sparse.size())
				sparse.resize(e.index + 1, INVALID);

			sparse[e.index] = dense.size();
			dense.push_back(e);
			data.emplace_back(std::forward<Args>(args)...);
			return data.back();
		}

		T& emplaceCopy(Entity e, const T& value)
		{
			if (contains(e))
				return data[sparse[e.index]];

			if (e.index >= sparse.size())
				sparse.resize(e.index + 1, INVALID);

			sparse[e.index] = dense.size();
			dense.push_back(e);
			data.push_back(value); // requires copyable, explicit
			return data.back();
		}

		void remove(Entity e) override
		{
			if (!contains(e))
				return;

			size_t index = sparse[e.index];
			size_t last = dense.size() - 1;

			dense[index] = dense[last];
			data[index] = std::move(data[last]);

			sparse[dense[index].index] = index;

			dense.pop_back();
			data.pop_back();
			sparse[e.index] = INVALID;
		}

		bool contains(Entity e) const override
		{
			return e.index < sparse.size() && sparse[e.index] != INVALID;
		}

		T& get(Entity e)
		{
			return data[sparse[e.index]];
		}

		const std::vector<Entity>& entities() const
		{
			return dense;
		}

		size_t size() const override
		{
			return dense.size();
		}

	private:
		static constexpr size_t INVALID = std::numeric_limits<size_t>::max();

		std::vector<Entity> dense;
		std::vector<T> data;
		std::vector<size_t> sparse; // = std::vector<size_t>(100000, INVALID);
	};

	class Registry
	{
	public:
		Entity create()
		{
			uint32_t index;
			if (!free.empty())
			{
				index = free.back();
				free.pop_back();
			}
			else
			{
				index = (uint32_t)generations.size();
				generations.push_back(0);
			}

			Entity e{ index, generations[index] };
			m_AliveEntities.push_back(e);
			return e;
		}

		void destroy(Entity e)
		{
			generations[e.index]++;
			free.push_back(e.index);

			for (auto& [_, storage] : storages)
			{
				storage->remove(e);
			}

			// remove from alive list (swap-remove)
			auto it = std::find(m_AliveEntities.begin(), m_AliveEntities.end(), e);
			if (it != m_AliveEntities.end())
			{
				*it = m_AliveEntities.back();
				m_AliveEntities.pop_back();
			}
		}

		template<typename T, typename... Args>
		T& emplace(Entity e, Args&&... args)
		{
			return storage<T>().emplace(e, std::forward<Args>(args)...);
		}

		template<typename T>
		T& emplaceCopy(Entity e, const T& value)
		{
			return storage<T>().emplaceCopy(e, value);
		}

		template<typename T>
		bool has(Entity e) const
		{
			auto it = storages.find(std::type_index(typeid(T)));
			return it != storages.end() && it->second->contains(e);
		}

		template<typename T>
		T& get(Entity e)
		{
			return storage<T>().get(e);
		}
		
		template<typename T>
		void remove(Entity e)
		{
			storage<T>().remove(e);
		}

		const std::vector<Entity>& entities() const
		{
			return m_AliveEntities;
		}

		template<typename... Components>
		auto view();

	private:
		template<typename T>
		ComponentStorage<T>& storage()
		{
			const std::type_index id(typeid(T));

			auto it = storages.find(id);
			if (it == storages.end())
			{
				auto storage = std::make_unique<ComponentStorage<T>>();
				auto* ptr = storage.get();
				storages.emplace(id, std::move(storage));
				return *ptr;
			}

			return *static_cast<ComponentStorage<T>*>(it->second.get());
		}
	private:
		std::unordered_map<std::type_index, std::unique_ptr<IComponentStorage>> storages;
		std::vector<uint32_t> generations;
		std::vector<uint32_t> free;
		std::vector<Entity> m_AliveEntities;

		template<typename...>
		friend class View;
	};

	template<typename... Components>
	class View
	{
	public:
		View(Registry& registry)
			: registry(registry), pools{ &registry.storage<Components>()... }
		{
			// pick smallest dense pool
			smallest = *std::min_element(pools.begin(), pools.end(),
				[](auto* a, auto* b)
				{
					return a->size() < b->size();
				}
				);
		}

		struct Iterator
		{
			size_t index = 0;
			View* view;

			Entity operator* () const
			{
				return static_cast<ComponentStorage<std::tuple_element_t<0, std::tuple<Components...>>>*>(view->smallest)->entities()[index];
			}

			Iterator& operator++ ()
			{
				do { ++index; }
				while (index < view->size() && !view->valid(entity()));
				return *this;
			}

			bool operator != (const Iterator& other) const
			{
				return index != other.index;
			}

		private:
			Entity entity() const
			{
				return static_cast<ComponentStorage<std::tuple_element_t<0, std::tuple<Components...>>>*>(view->smallest)->entities()[index];
			}
		};

		Iterator begin()
		{
			Iterator it{ 0, this };
			if (it.index < size() && !valid(*it))
				++it;
			return it;
		}

		Iterator end()
		{
			return { size(), this };
		}

		template<typename T>
		T& get(Entity e)
		{
			return registry.get<T>(e);
		}

		template<typename T, typename U, typename... Rest>
		std::tuple<T&, U&, Rest&...> get(Entity e)
		{
			return {registry.get<T>(e), registry.get<U>(e), registry.get<Rest>(e)...};
		}
	private:
		bool valid(Entity e)
		{
			return (registry.has<Components>(e) && ...);
		}

		size_t size() const
		{
			return smallest->size();
		}
	private:
		Registry& registry;
		std::array<IComponentStorage*, sizeof...(Components)> pools;
		IComponentStorage* smallest;
	};

	template<typename ...Components>
	auto Registry::view()
	{
		return View<Components...>(*this);
	}
}

