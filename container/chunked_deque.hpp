#pragma once

#include <assert.h>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <new>
#include <type_traits>
#include <utility>

// A double-ended queue over fixed-size blocks, with erasure and insertion anywhere in the sequence.
//
// Order is block order in the map, then ascending slot index within a block. Each block carries a bitmask of
// occupied slots, so an element is erased by clearing its bit: nothing is relocated and no block is scanned.
//
// The trade against std::deque, which relocates on a middle erase: an erased slot is never refilled by a push,
// so a block holds its allocation until every element in it is gone. That is what keeps sequence order exact
// while leaving elements where they are - unlike plf::colony and std::hive, which reuse erased slots on
// insertion and are therefore unordered.
//
// Invariants:
//   _size == 0 iff no blocks are allocated: an operation that empties the container re-anchors it.
//   push_back writes past the back block's last live slot, so an append cannot land before a live element.
//   Interior blocks may be empty; only the end operations release blocks, and only at the end they touch.
//
// Element requirements: destructible and move-constructible. insert() additionally requires a non-throwing
// move: it relocates a block's tail, and a throw part-way would leave the sequence broken.
//
// Iterators are invalidated by any insertion, and by an erasure only if it empties the container. References
// to elements are stable across erasure and across pushes.
template <typename T, size_t BlockSize = 64>
class chunked_deque
{
	static_assert(BlockSize >= 1 && BlockSize <= 64, "The occupancy mask is one uint64_t per block");

public:
	using value_type = T;
	using size_type = size_t;
	using difference_type = ptrdiff_t;
	using reference = T&;
	using const_reference = const T&;
	using pointer = T*;
	using const_pointer = const T*;

private:
	struct Block
	{
		uint64_t mask = 0; // Bit i set: slot i holds a live element
		alignas(T) std::byte storage[BlockSize * sizeof(T)];

		[[nodiscard]] void* address(size_t slot) noexcept { return storage + slot * sizeof(T); }
		[[nodiscard]] T& at(size_t slot) noexcept { return *std::launder(reinterpret_cast<T*>(address(slot))); }
		[[nodiscard]] const T& at(size_t slot) const noexcept { return const_cast<Block&>(*this).at(slot); }
	};

	static constexpr uint64_t bitOf(size_t slot) noexcept { return uint64_t{ 1 } << slot; }
	// Bits strictly below / above `slot`. The shift in maskAbove is undefined at slot == 63, hence the branch.
	static constexpr uint64_t maskBelow(size_t slot) noexcept { return bitOf(slot) - 1; }
	static constexpr uint64_t maskAbove(size_t slot) noexcept { return slot >= 63 ? 0 : ~maskBelow(slot + 1); }
	// maskAbove runs to bit 63; every use that can reach past the last slot must be bounded by this.
	static constexpr uint64_t slotsMask = BlockSize == 64 ? ~uint64_t{ 0 } : maskBelow(BlockSize);

	[[nodiscard]] static size_t lowestSlot(uint64_t mask) noexcept
	{
		assert(mask != 0);
		return static_cast<size_t>(std::countr_zero(mask));
	}

	[[nodiscard]] static size_t highestSlot(uint64_t mask) noexcept
	{
		assert(mask != 0);
		return 63 - static_cast<size_t>(std::countl_zero(mask));
	}

public:
	template <bool IsConst>
	class basic_iterator
	{
		friend class chunked_deque;
		using container_type = std::conditional_t<IsConst, const chunked_deque, chunked_deque>;

	public:
		using iterator_category = std::bidirectional_iterator_tag;
		using iterator_concept = std::bidirectional_iterator_tag;
		using value_type = T;
		using difference_type = ptrdiff_t;
		using reference = std::conditional_t<IsConst, const T&, T&>;
		using pointer = std::conditional_t<IsConst, const T*, T*>;

		basic_iterator() noexcept = default;

		template <bool OtherConst> requires (IsConst && !OtherConst)
		basic_iterator(const basic_iterator<OtherConst>& other) noexcept :
			_container{ other._container }, _blockOrdinal{ other._blockOrdinal }, _slot{ other._slot }
		{}

		[[nodiscard]] reference operator*() const noexcept { return _container->blockAt(_blockOrdinal).at(_slot); }
		[[nodiscard]] pointer operator->() const noexcept { return &operator*(); }

		basic_iterator& operator++() noexcept
		{
			assert(_blockOrdinal < _container->_blockCount);
			const uint64_t rest = _container->blockAt(_blockOrdinal).mask & maskAbove(_slot);
			if (rest != 0)
			{
				_slot = lowestSlot(rest);
				return *this;
			}

			_slot = 0;
			while (++_blockOrdinal < _container->_blockCount)
			{
				const uint64_t mask = _container->blockAt(_blockOrdinal).mask;
				if (mask != 0)
				{
					_slot = lowestSlot(mask);
					break;
				}
			}

			return *this;
		}

		basic_iterator& operator--() noexcept
		{
			if (_blockOrdinal < _container->_blockCount)
			{
				const uint64_t rest = _container->blockAt(_blockOrdinal).mask & maskBelow(_slot);
				if (rest != 0)
				{
					_slot = highestSlot(rest);
					return *this;
				}
			}

			while (_blockOrdinal > 0)
			{
				const uint64_t mask = _container->blockAt(--_blockOrdinal).mask;
				if (mask != 0)
				{
					_slot = highestSlot(mask);
					return *this;
				}
			}

			assert(false && "Decrementing begin()");
			return *this;
		}

		basic_iterator operator++(int) noexcept { auto copy = *this; ++*this; return copy; }
		basic_iterator operator--(int) noexcept { auto copy = *this; --*this; return copy; }

		template <bool OtherConst>
		[[nodiscard]] bool operator==(const basic_iterator<OtherConst>& other) const noexcept
		{
			return _blockOrdinal == other._blockOrdinal && _slot == other._slot;
		}

	private:
		basic_iterator(container_type* container, size_t blockOrdinal, size_t slot) noexcept :
			_container{ container }, _blockOrdinal{ blockOrdinal }, _slot{ slot }
		{}

		friend class basic_iterator<!IsConst>;

		container_type* _container = nullptr;
		size_t _blockOrdinal = 0;
		size_t _slot = 0;
	};

	using iterator = basic_iterator<false>;
	using const_iterator = basic_iterator<true>;

	chunked_deque() noexcept = default;

	chunked_deque(const chunked_deque& other) requires std::is_copy_constructible_v<T>
	{
		for (const T& item : other)
			push_back(item);
	}

	chunked_deque(chunked_deque&& other) noexcept
	{
		swap(other);
	}

	chunked_deque& operator=(const chunked_deque& other) requires std::is_copy_constructible_v<T>
	{
		if (this != &other)
		{
			chunked_deque copy{ other };
			swap(copy);
		}

		return *this;
	}

	chunked_deque& operator=(chunked_deque&& other) noexcept
	{
		if (this != &other)
		{
			clear();
			swap(other);
		}

		return *this;
	}

	~chunked_deque() noexcept
	{
		clear();
		delete _spareBlock;
		delete[] _map;
	}

	void swap(chunked_deque& other) noexcept
	{
		std::swap(_map, other._map);
		std::swap(_mapCapacity, other._mapCapacity);
		std::swap(_mapHead, other._mapHead);
		std::swap(_blockCount, other._blockCount);
		std::swap(_spareBlock, other._spareBlock);
		std::swap(_size, other._size);
	}

	[[nodiscard]] size_t size() const noexcept { return _size; }
	[[nodiscard]] bool empty() const noexcept { return _size == 0; }

	[[nodiscard]] iterator begin() noexcept { const auto [block, slot] = frontPosition(); return { this, block, slot }; }
	[[nodiscard]] const_iterator begin() const noexcept { const auto [block, slot] = frontPosition(); return { this, block, slot }; }
	[[nodiscard]] const_iterator cbegin() const noexcept { return begin(); }

	[[nodiscard]] iterator end() noexcept { return { this, _blockCount, 0 }; }
	[[nodiscard]] const_iterator end() const noexcept { return { this, _blockCount, 0 }; }
	[[nodiscard]] const_iterator cend() const noexcept { return end(); }

	[[nodiscard]] T& front() noexcept { const auto [block, slot] = frontPosition(); return blockAt(block).at(slot); }
	[[nodiscard]] const T& front() const noexcept { const auto [block, slot] = frontPosition(); return blockAt(block).at(slot); }

	[[nodiscard]] T& back() noexcept { const auto [block, slot] = backPosition(); return blockAt(block).at(slot); }
	[[nodiscard]] const T& back() const noexcept { const auto [block, slot] = backPosition(); return blockAt(block).at(slot); }

	template <typename... Args>
	T& emplace_back(Args&&... args)
	{
		if (_blockCount != 0)
		{
			Block& block = blockAt(_blockCount - 1);
			// One past the last live slot, so an append can never land before a live element. Slots above it
			// are dead - an erasure left them - and appending there reclaims them.
			const size_t slot = block.mask == 0 ? 0 : highestSlot(block.mask) + 1;
			if (slot < BlockSize)
				return construct(block, slot, std::forward<Args>(args)...);
		}

		appendBlock();
		return construct(blockAt(_blockCount - 1), 0, std::forward<Args>(args)...);
	}

	template <typename... Args>
	T& emplace_front(Args&&... args)
	{
		trimFront();
		if (_blockCount == 0)
		{
			// Slot 0, not the top of the block: a container only ever pushed to the back must not pay for the
			// room a push_front would want below.
			appendBlock();
			return construct(blockAt(0), 0, std::forward<Args>(args)...);
		}

		const size_t lowest = lowestSlot(blockAt(0).mask);
		if (lowest > 0)
			return construct(blockAt(0), lowest - 1, std::forward<Args>(args)...);

		prependBlock();
		return construct(blockAt(0), BlockSize - 1, std::forward<Args>(args)...);
	}

	void push_back(const T& item) { emplace_back(item); }
	void push_back(T&& item) { emplace_back(std::move(item)); }
	void push_front(const T& item) { emplace_front(item); }
	void push_front(T&& item) { emplace_front(std::move(item)); }

	void pop_front() noexcept
	{
		assert(_size != 0);
		trimFront();
		Block& block = blockAt(0);
		destroy(block, lowestSlot(block.mask));
		onErased();
	}

	void pop_back() noexcept
	{
		assert(_size != 0);
		trimBack();
		Block& block = blockAt(_blockCount - 1);
		destroy(block, highestSlot(block.mask));
		onErased();
	}

	// Returns an iterator to the element after the erased one.
	iterator erase(const_iterator pos) noexcept
	{
		assert(pos != cend());
		iterator next{ this, pos._blockOrdinal, pos._slot };
		++next;

		destroy(blockAt(pos._blockOrdinal), pos._slot);
		if (onErased())
			return end();

		assertOccupancyMatchesSize();
		return next;
	}

	// Removes every element matching the predicate; returns how many were removed.
	template <typename Pred>
	size_t remove_if(Pred pred)
	{
		size_t removedCount = 0;
		for (size_t ordinal = 0; ordinal < _blockCount; ++ordinal)
		{
			Block& block = blockAt(ordinal);
			for (uint64_t rest = block.mask; rest != 0; rest &= rest - 1)
			{
				const size_t slot = lowestSlot(rest);
				if (pred(std::as_const(block.at(slot))))
				{
					destroy(block, slot);
					--_size;
					++removedCount;
				}
			}
		}

		if (_size == 0)
			resetToEmpty();

		assertOccupancyMatchesSize();
		return removedCount;
	}

	// Inserts before pos. Fills a free slot adjacent to the insertion point when there is one, and otherwise
	// splits the block, which relocates that block's tail.
	template <typename... Args>
	iterator emplace(const_iterator pos, Args&&... args)
	{
		static_assert(std::is_nothrow_move_constructible_v<T>, "insert() relocates a block's tail");

		if (pos == cend())
		{
			emplace_back(std::forward<Args>(args)...);
			return backPositionIterator();
		}

		if (pos == cbegin())
		{
			emplace_front(std::forward<Args>(args)...);
			return begin();
		}

		const_iterator predecessor = pos;
		--predecessor;

		if (predecessor._blockOrdinal == pos._blockOrdinal)
		{
			// Between two live slots of one block every slot is free, so this fails only when they are adjacent.
			const uint64_t gap = ~blockAt(pos._blockOrdinal).mask & maskAbove(predecessor._slot) & maskBelow(pos._slot);
			if (gap != 0)
			{
				const size_t slot = lowestSlot(gap);
				construct(blockAt(pos._blockOrdinal), slot, std::forward<Args>(args)...);
				assertOccupancyMatchesSize();
				return { this, pos._blockOrdinal, slot };
			}
		}
		else
		{
			// pos is the first live element of its block and the predecessor the last of an earlier one, so the
			// slots below pos and above the predecessor are all free. Either side takes the element.
			const uint64_t freeBelow = ~blockAt(pos._blockOrdinal).mask & maskBelow(pos._slot);
			if (freeBelow != 0)
			{
				const size_t slot = highestSlot(freeBelow);
				construct(blockAt(pos._blockOrdinal), slot, std::forward<Args>(args)...);
				assertOccupancyMatchesSize();
				return { this, pos._blockOrdinal, slot };
			}

			const uint64_t freeAbove = ~blockAt(predecessor._blockOrdinal).mask & maskAbove(predecessor._slot) & slotsMask;
			if (freeAbove != 0)
			{
				const size_t slot = lowestSlot(freeAbove);
				construct(blockAt(predecessor._blockOrdinal), slot, std::forward<Args>(args)...);
				assertOccupancyMatchesSize();
				return { this, predecessor._blockOrdinal, slot };
			}
		}

		splitBlockAt(pos._blockOrdinal, pos._slot);
		construct(blockAt(pos._blockOrdinal), pos._slot, std::forward<Args>(args)...);
		assertOccupancyMatchesSize();
		return { this, pos._blockOrdinal, pos._slot };
	}

	iterator insert(const_iterator pos, const T& item) { return emplace(pos, item); }
	iterator insert(const_iterator pos, T&& item) { return emplace(pos, std::move(item)); }

	void clear() noexcept
	{
		for (size_t ordinal = 0; ordinal < _blockCount; ++ordinal)
		{
			Block& block = blockAt(ordinal);
			for (uint64_t rest = block.mask; rest != 0; rest &= rest - 1)
				block.at(lowestSlot(rest)).~T();

			block.mask = 0;
		}

		_size = 0;
		resetToEmpty();
	}

private:
	[[nodiscard]] Block& blockAt(size_t ordinal) noexcept
	{
		assert(ordinal < _blockCount);
		return *_map[(_mapHead + ordinal) & (_mapCapacity - 1)];
	}

	[[nodiscard]] const Block& blockAt(size_t ordinal) const noexcept
	{
		return const_cast<chunked_deque&>(*this).blockAt(ordinal);
	}

	// The end blocks may be empty when an erasure emptied them, so both ends scan rather than assume.
	[[nodiscard]] std::pair<size_t, size_t> frontPosition() const noexcept
	{
		for (size_t ordinal = 0; ordinal < _blockCount; ++ordinal)
		{
			if (const uint64_t mask = blockAt(ordinal).mask; mask != 0)
				return { ordinal, lowestSlot(mask) };
		}

		return { _blockCount, 0 };
	}

	[[nodiscard]] std::pair<size_t, size_t> backPosition() const noexcept
	{
		for (size_t ordinal = _blockCount; ordinal > 0; --ordinal)
		{
			if (const uint64_t mask = blockAt(ordinal - 1).mask; mask != 0)
				return { ordinal - 1, highestSlot(mask) };
		}

		return { _blockCount, 0 };
	}

	[[nodiscard]] iterator backPositionIterator() noexcept
	{
		const auto [block, slot] = backPosition();
		return { this, block, slot };
	}

	template <typename... Args>
	T& construct(Block& block, size_t slot, Args&&... args)
	{
		assert((block.mask & bitOf(slot)) == 0);
		T* item = ::new (block.address(slot)) T(std::forward<Args>(args)...);
		block.mask |= bitOf(slot);
		++_size;
		return *item;
	}

	void destroy(Block& block, size_t slot) noexcept
	{
		assert((block.mask & bitOf(slot)) != 0);
		block.at(slot).~T();
		block.mask &= ~bitOf(slot);
	}

	// Common tail of the single-element removals. Returns true if the container is now empty.
	bool onErased() noexcept
	{
		--_size;
		if (_size != 0)
			return false;

		resetToEmpty();
		return true;
	}

	void trimFront() noexcept
	{
		while (_blockCount > 0 && blockAt(0).mask == 0)
		{
			recycleBlock(_map[_mapHead]);
			_mapHead = (_mapHead + 1) & (_mapCapacity - 1);
			--_blockCount;
		}
	}

	void trimBack() noexcept
	{
		while (_blockCount > 0 && blockAt(_blockCount - 1).mask == 0)
		{
			recycleBlock(&blockAt(_blockCount - 1));
			--_blockCount;
		}
	}

	void resetToEmpty() noexcept
	{
		assert(_size == 0);
		for (size_t ordinal = 0; ordinal < _blockCount; ++ordinal)
			recycleBlock(&blockAt(ordinal));

		_blockCount = 0;
		_mapHead = 0;
	}

	[[nodiscard]] Block* acquireBlock()
	{
		if (_spareBlock == nullptr)
			return new Block;

		Block* block = std::exchange(_spareBlock, nullptr);
		block->mask = 0;
		return block;
	}

	void recycleBlock(Block* block) noexcept
	{
		assert(block->mask == 0);
		if (_spareBlock == nullptr)
			_spareBlock = block;
		else
			delete block;
	}

	// The map grows first: a block acquired before it would leak if the map's allocation threw.
	void appendBlock()
	{
		reserveMapSlot();
		Block* block = acquireBlock();
		_map[(_mapHead + _blockCount) & (_mapCapacity - 1)] = block;
		++_blockCount;
	}

	void prependBlock()
	{
		reserveMapSlot();
		Block* block = acquireBlock();
		_mapHead = (_mapHead + _mapCapacity - 1) & (_mapCapacity - 1);
		_map[_mapHead] = block;
		++_blockCount;
	}

	// Only the pointer map is ever copied wholesale - one word per block, against a block of elements that
	// never moves.
	void reserveMapSlot()
	{
		if (_blockCount < _mapCapacity)
			return;

		const size_t newCapacity = _mapCapacity == 0 ? 4 : _mapCapacity * 2;
		Block** newMap = new Block*[newCapacity];
		for (size_t ordinal = 0; ordinal < _blockCount; ++ordinal)
			newMap[ordinal] = _map[(_mapHead + ordinal) & (_mapCapacity - 1)];

		delete[] _map;
		_map = newMap;
		_mapCapacity = newCapacity;
		_mapHead = 0;
	}

	// Moves everything from `slot` up into a fresh block placed right after `ordinal`, leaving that slot and
	// everything above it free.
	void splitBlockAt(size_t ordinal, size_t slot)
	{
		reserveMapSlot();
		Block* tail = acquireBlock();

		Block& block = blockAt(ordinal);
		size_t tailSlot = 0;
		for (uint64_t rest = block.mask & ~maskBelow(slot); rest != 0; rest &= rest - 1)
		{
			T& item = block.at(lowestSlot(rest));
			::new (tail->address(tailSlot)) T(std::move(item));
			tail->mask |= bitOf(tailSlot);
			item.~T();
			++tailSlot;
		}

		block.mask &= maskBelow(slot);

		for (size_t i = _blockCount; i > ordinal + 1; --i)
			_map[(_mapHead + i) & (_mapCapacity - 1)] = _map[(_mapHead + i - 1) & (_mapCapacity - 1)];

		_map[(_mapHead + ordinal + 1) & (_mapCapacity - 1)] = tail;
		++_blockCount;
	}

	void assertOccupancyMatchesSize() const noexcept
	{
#ifndef NDEBUG
		size_t liveCount = 0;
		for (size_t ordinal = 0; ordinal < _blockCount; ++ordinal)
			liveCount += static_cast<size_t>(std::popcount(blockAt(ordinal).mask));

		assert(liveCount == _size);
#endif
	}

	Block** _map = nullptr; // Ring of block pointers; capacity is a power of two
	size_t _mapCapacity = 0;
	size_t _mapHead = 0;
	size_t _blockCount = 0;
	Block* _spareBlock = nullptr; // Absorbs a queue oscillating across a block boundary
	size_t _size = 0;
};
