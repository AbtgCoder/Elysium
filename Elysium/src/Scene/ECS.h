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
#include <limits>
#include <cassert>

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
		virtual const std::vector<Entity>& entities() const = 0;
	};

	template<typename T>
	class ComponentStorage : public IComponentStorage
	{
	public:
		ComponentStorage() = default;

		template<typename... Args>
		T& emplace(Entity e, Args&&... args)
		{
			size_t* slot = tryGetSparseSlot(e);
			if (slot && *slot != INVALID)
				return data[*slot];

			size_t index = dense.size();
			dense.push_back(e);
			data.emplace_back(std::forward<Args>(args)...);

			sparseSlot(e) = index;
			return data.back();
		}

		T& emplaceCopy(Entity e, const T& value)
		{
			size_t* slot = tryGetSparseSlot(e);
			if (slot && *slot != INVALID)
				return data[*slot];

			size_t index = dense.size();
			dense.push_back(e);
			data.push_back(value);

			sparseSlot(e) = index;
			return data.back();
		}

		void remove(Entity e) override
		{
			size_t* slot = tryGetSparseSlot(e);
			if (!slot || *slot == INVALID)
				return;

			size_t index = *slot;
			size_t last = dense.size() - 1;

			if (index != last)
			{
				dense[index] = dense[last];
				data[index] = std::move(data[last]);

				sparseSlot(dense[index]) = index;
			}

			dense.pop_back();
			data.pop_back();
			*slot = INVALID;
		}

		bool contains(Entity e) const override
		{
			const size_t* slot = tryGetSparseSlotConst(e);
			return slot && *slot != INVALID;
		}

		T& get(Entity e)
		{
			size_t* slot = tryGetSparseSlot(e);
			assert(slot && *slot != INVALID && "Component does not exist on entity");
			return data[*slot];
		}

		const std::vector<Entity>& entities() const override
		{
			return dense;
		}

		size_t size() const override
		{
			return dense.size();
		}
	private:
		size_t& sparseSlot(Entity e)
		{
			const size_t pageIndex = e.index >> PAGE_SHIFT;
			const size_t offset = e.index & PAGE_MASK;

			if (pageIndex >= sparsePages.size())
				sparsePages.resize(pageIndex + 1);

			if (!sparsePages[pageIndex])
			{
				auto page = std::make_unique<Page>();
				page->fill(INVALID);
				sparsePages[pageIndex] = std::move(page);
			}

			return (*sparsePages[pageIndex])[offset];
		}
		size_t* tryGetSparseSlot(Entity e)
		{
			const size_t pageIndex = e.index >> PAGE_SHIFT;
			if (pageIndex >= sparsePages.size())
				return nullptr;

			auto& page = sparsePages[pageIndex];
			if (!page)
				return nullptr;

			return &(*page)[e.index & PAGE_MASK];
		}
		const size_t* tryGetSparseSlotConst(Entity e) const
		{
			const size_t pageIndex = e.index >> PAGE_SHIFT;
			if (pageIndex >= sparsePages.size())
				return nullptr;

			const auto& page = sparsePages[pageIndex];
			if (!page)
				return nullptr;

			return &(*page)[e.index & PAGE_MASK];
		}
	private:
		static constexpr size_t INVALID = std::numeric_limits<size_t>::max();

		std::vector<Entity> dense;
		std::vector<T> data;

		static constexpr size_t PAGE_SHIFT = 8; // 256 entries per page...
		static constexpr size_t PAGE_SIZE = 1ull << PAGE_SHIFT;
		static constexpr size_t PAGE_MASK = PAGE_SIZE - 1;

		using Page = std::array<size_t, PAGE_SIZE>;

		std::vector<std::unique_ptr<Page>> sparsePages;
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
			assert(isValid(e) && "emplace on invalid entity");
			return storage<T>().emplace(e, std::forward<Args>(args)...);
		}

		template<typename T>
		T& emplaceCopy(Entity e, const T& value)
		{
			assert(isValid(e) && "emplaceCopy on invalid entity");
			return storage<T>().emplaceCopy(e, value);
		}

		template<typename T, typename... Args>
		T& emplaceOrReplace(Entity e, Args&&... args)
		{
			assert(isValid(e) && "emplaceOrReplace on invalid entity");
			if (has<T>(e))
			{
				get<T>(e) = T(std::forward<Args>(args)...);
				return get<T>(e);
			}
			return emplace<T>(e, std::forward<Args>(args)...);
		}

		template<typename T>
		bool has(Entity e) const
		{
			if (!(e.index < generations.size())) return false;
			auto it = storages.find(std::type_index(typeid(T)));
			return it != storages.end() && it->second->contains(e);
		}

		template<typename T>
		T& get(Entity e)
		{
			assert(isValid(e) && "get on invalid entity");
			return storage<T>().get(e);
		}
		
		template<typename T>
		void remove(Entity e)
		{
			assert(isValid(e) && "remove on invalid entity");
			storage<T>().remove(e);
		}

		void removeAllComponents(Entity e)
		{
			assert(isValid(e) && "tried removing all components on invalid entity");
			for (auto& [k, s] : storages)
				s->remove(e);
		}

		const std::vector<Entity>& entities() const
		{
			return m_AliveEntities;
		}

		bool isValid(Entity e) const
		{
			return e.index < generations.size() && generations[e.index] == e.generation;
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

	template<typename T>
	using Storage = ComponentStorage<std::remove_cv_t<T>>;

	template<typename... Components>
	class View
	{
		static_assert(sizeof...(Components) > 0, "View must have at least one component");

		static constexpr size_t N = sizeof...(Components);

	public:
		explicit View(Registry& registry)
			: registry(registry),
			storages(&registry.storage<std::remove_cv_t<Components>>()...)
		{
			// Find smallest storage
			smallestIndex = findSmallestIndex();

			// Build unchecked list
			uncheckedCount = 0;
			for (size_t i = 0; i < N; ++i)
			{
				if (i != smallestIndex)
					uncheckedIndices[uncheckedCount++] = i;
			}
		}

		struct Iterator
		{
			size_t index;
			View* view;

			Entity operator*() const
			{
				return view->entities()[index];
			}

			Iterator& operator++()
			{
				do
				{
					++index;
				} while (index < view->entities().size() &&
					!view->valid(view->entities()[index]));
				return *this;
			}

			bool operator!=(const Iterator& other) const
			{
				return index != other.index;
			}
		};

		Iterator begin()
		{
			Iterator it{ 0, this };
			if (it.index < entities().size() && !valid(entities()[it.index]))
				++it;
			return it;
		}

		Iterator end()
		{
			return { entities().size(), this };
		}

		template<typename T>
		T& get(Entity e)
		{
			return registry.get<T>(e);
		}

		template<typename T, typename U, typename... Rest>
		std::tuple<T&, U&, Rest&...> get(Entity e)
		{
			return { registry.get<T>(e), registry.get<U>(e), registry.get<Rest>(e)... };
		}

		const std::vector<Entity>& entities() const
		{
			return entitiesAt(smallestIndex);
		}

	private:
		bool valid(Entity e) const //TODO: could optimize this further, could try to make it branchless ig..
		{
			for (size_t i = 0; i < uncheckedCount; ++i)
			{
				if (!containsAt(uncheckedIndices[i], e))
					return false;
			}
			return true;
		}

		size_t findSmallestIndex() const
		{
			return findSmallestIndexImpl(std::make_index_sequence<N>{});
		}

		template<size_t... Is> 
		size_t findSmallestIndexImpl(std::index_sequence<Is...>) const
		{
			size_t smallest = 0;
			size_t smallestSize = std::get<0>(storages)->size();

			(([&] {
				const size_t currentSize = std::get<Is>(storages)->size();
				if (currentSize < smallestSize)
				{
					smallest = Is;
					smallestSize = currentSize;
				}
			}()), ...);

			return smallest;
		}

		template<size_t I>
		size_t sizeAt() const
		{
			return std::get<I>(storages)->size();
		}

		bool containsAt(size_t index, Entity e) const
		{
			return visitStorage(index, [&](auto* s) {
				return s->contains(e);
				});
		}

		const std::vector<Entity>& entitiesAt(size_t index) const
		{
			return visitStorage(index, [&](auto* s) -> const std::vector<Entity>&{
				return s->entities();
				});
		}

		template<typename Func>
		decltype(auto) visitStorage(size_t index, Func&& func) const
		{
			using Ret = decltype(func(std::get<0>(storages)));

			static constexpr auto table = makeDispatchTable<Func, Ret>(std::make_index_sequence<N>{});

			return table[index](*this, std::forward<Func>(func));
		}
		
		/*
		* we want to call a function on the i-th storage in our tuple for a given index i
		* but tuples don't support runtime indexing: std::get<I>(tuple) requires I to be known at compile time
		* solution: create a compile-time lookup table (using index sequences and function pointers)
		* it stores one function per tuple index and stores them in an array
		* and then we can just index the table at runtime...
		*/
		template<typename Func, typename Ret, size_t... Is>
		static constexpr auto makeDispatchTable(std::index_sequence<Is...>)
		{
			using Fn = Ret(*)(const View&, Func&&);

			return std::array<Fn, N>{+[](const View& view, Func&& func) -> Ret {return func(std::get<Is>(view.storages)); }...};
		}

	private:
		Registry& registry;

		std::tuple<Storage<Components>*...> storages;

		size_t smallestIndex = 0;

		std::array<size_t, N - 1> uncheckedIndices{};
		size_t uncheckedCount = 0;
	};



	template<typename ...Components>
	auto Registry::view()
	{
		return View<Components...>(*this);
	}
}

