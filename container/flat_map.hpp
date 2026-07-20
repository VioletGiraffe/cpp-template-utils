#pragma once

#include <algorithm>
#include <assert.h>
#include <concepts>
#include <cstddef>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

namespace FlatContainerInternal {

	template <typename Left, typename Right>
	concept EqualityComparable = requires(const Left& left, const Right& right) {
		{ left == right } -> std::convertible_to<bool>;
	};

	template <typename Left, typename Right, typename Compare>
	[[nodiscard]] bool sorted_keys_equal(const Left& left, const Right& right, [[maybe_unused]] const Compare& compare)
	{
		if constexpr (EqualityComparable<Left, Right>)
			return left == right;
		else
			return !compare(left, right); // Sortedness already established !(right < left)
	}

	template <typename Left, typename Right, typename Compare>
	[[nodiscard]] bool lower_bound_matches(const Left& stored_key, const Right& query, [[maybe_unused]] const Compare& compare)
	{
		if constexpr (EqualityComparable<Left, Right>)
			return stored_key == query;
		else
			return !compare(query, stored_key); // lower_bound already established !(stored_key < query)
	}

	template <typename Key, typename Mapped, typename MappedReference>
	struct flat_map_reference
	{
		const Key& first;
		MappedReference second;

		[[nodiscard]] const Key& key() const noexcept { return first; }
		[[nodiscard]] decltype(auto) value() noexcept { return (second); }
		[[nodiscard]] decltype(auto) value() const noexcept { return (second); }

		operator std::pair<Key, Mapped>() const { return { first, second }; }

		template <typename OtherMappedReference>
		[[nodiscard]] friend bool operator==(const flat_map_reference& left, const flat_map_reference<Key, Mapped, OtherMappedReference>& right)
		{
			return left.first == right.first && left.second == right.second;
		}

		template <typename OtherKey, typename OtherMapped>
		[[nodiscard]] friend bool operator==(const flat_map_reference& left, const std::pair<OtherKey, OtherMapped>& right)
		{
			return left.first == right.first && left.second == right.second;
		}

		template <typename OtherKey, typename OtherMapped>
		[[nodiscard]] friend bool operator==(const std::pair<OtherKey, OtherMapped>& left, const flat_map_reference& right)
		{
			return right == left;
		}
	};

	template <typename Reference>
	class flat_map_arrow_proxy
	{
	public:
		explicit flat_map_arrow_proxy(Reference reference): _reference(std::move(reference)) {}

		[[nodiscard]] Reference* operator->() noexcept { return &_reference; }
		[[nodiscard]] const Reference* operator->() const noexcept { return &_reference; }

	private:
		Reference _reference;
	};

} // namespace FlatContainerInternal

template <typename Key, typename Mapped, typename Compare = std::less<>>
class flat_map
{
public:
	using key_type = Key;
	using mapped_type = Mapped;
	using value_type = std::pair<Key, Mapped>;
	using key_compare = Compare;
	using size_type = typename std::vector<Key>::size_type;
	using difference_type = typename std::vector<Key>::difference_type;
	using mapped_reference = typename std::vector<Mapped>::reference;
	using const_mapped_reference = typename std::vector<Mapped>::const_reference;

private:
	template <bool IsConst>
	class basic_iterator
	{
	private:
		using container_type = std::conditional_t<IsConst, const flat_map, flat_map>;
		using mapped_reference = std::conditional_t<IsConst, typename std::vector<Mapped>::const_reference, typename std::vector<Mapped>::reference>;

	public:
		using iterator_category = std::random_access_iterator_tag;
		using iterator_concept = std::random_access_iterator_tag;
		using value_type = typename flat_map::value_type;
		using difference_type = typename flat_map::difference_type;
		using reference = FlatContainerInternal::flat_map_reference<Key, Mapped, mapped_reference>;
		using pointer = FlatContainerInternal::flat_map_arrow_proxy<reference>;

		basic_iterator() noexcept = default;

		template <bool OtherConst> requires (IsConst && !OtherConst)
		basic_iterator(const basic_iterator<OtherConst>& other) noexcept: _container(other._container), _index(other._index) {}

		[[nodiscard]] reference operator*() const
		{
			assert_dereferenceable();
			return { _container->_keys[_index], _container->_values[_index] };
		}

		[[nodiscard]] pointer operator->() const { return pointer(**this); }
		[[nodiscard]] reference operator[](difference_type offset) const { return *(*this + offset); }

		[[nodiscard]] const Key& key() const
		{
			assert_dereferenceable();
			return _container->_keys[_index];
		}

		[[nodiscard]] mapped_reference value() const
		{
			assert_dereferenceable();
			return _container->_values[_index];
		}

		basic_iterator& operator++() noexcept { ++_index; return *this; }
		basic_iterator operator++(int) noexcept { const auto old = *this; ++*this; return old; }
		basic_iterator& operator--() noexcept { --_index; return *this; }
		basic_iterator operator--(int) noexcept { const auto old = *this; --*this; return old; }
		basic_iterator& operator+=(difference_type offset) noexcept { _index = static_cast<size_type>(static_cast<difference_type>(_index) + offset); return *this; }
		basic_iterator& operator-=(difference_type offset) noexcept { return *this += -offset; }

		[[nodiscard]] friend basic_iterator operator+(basic_iterator iterator, difference_type offset) noexcept { iterator += offset; return iterator; }
		[[nodiscard]] friend basic_iterator operator+(difference_type offset, basic_iterator iterator) noexcept { iterator += offset; return iterator; }
		[[nodiscard]] friend basic_iterator operator-(basic_iterator iterator, difference_type offset) noexcept { iterator -= offset; return iterator; }

		template <bool OtherConst>
		[[nodiscard]] difference_type operator-(const basic_iterator<OtherConst>& other) const noexcept
		{
			assert(_container == other._container);
			return static_cast<difference_type>(_index) - static_cast<difference_type>(other._index);
		}

		template <bool OtherConst>
		[[nodiscard]] bool operator==(const basic_iterator<OtherConst>& other) const noexcept
		{
			return _container == other._container && _index == other._index;
		}

		template <bool OtherConst>
		[[nodiscard]] bool operator!=(const basic_iterator<OtherConst>& other) const noexcept { return !(*this == other); }

		template <bool OtherConst>
		[[nodiscard]] bool operator<(const basic_iterator<OtherConst>& other) const noexcept
		{
			assert(_container == other._container);
			return _index < other._index;
		}

		template <bool OtherConst>
		[[nodiscard]] bool operator>(const basic_iterator<OtherConst>& other) const noexcept { return other < *this; }

		template <bool OtherConst>
		[[nodiscard]] bool operator<=(const basic_iterator<OtherConst>& other) const noexcept { return !(other < *this); }

		template <bool OtherConst>
		[[nodiscard]] bool operator>=(const basic_iterator<OtherConst>& other) const noexcept { return !(*this < other); }

	private:
		friend class flat_map;
		template <bool> friend class basic_iterator;

		basic_iterator(container_type* container, size_type index) noexcept: _container(container), _index(index) {}

		void assert_dereferenceable() const
		{
			assert(_container != nullptr);
			assert(!_container->batch_open());
			assert(_index < _container->size());
		}

		container_type* _container = nullptr;
		size_type _index = 0;
	};

public:
	using iterator = basic_iterator<false>;
	using const_iterator = basic_iterator<true>;
	using reference = typename iterator::reference;
	using const_reference = typename const_iterator::reference;

	flat_map() = default;
	explicit flat_map(Compare compare): _compare(std::move(compare)) {}

	flat_map(std::initializer_list<value_type> values, Compare compare = {}): _compare(std::move(compare))
	{
		begin_batch();
		for (const auto& [key, value] : values)
			append_unsorted(key, value);
		end_batch();
	}

	[[nodiscard]] bool empty() const noexcept { return _keys.empty(); }
	[[nodiscard]] size_type size() const noexcept { return _keys.size(); }
	[[nodiscard]] bool batch_open() const noexcept { return _batch_start != no_batch; }
	[[nodiscard]] const Compare& key_comp() const noexcept { return _compare; }

	[[nodiscard]] friend bool operator==(const flat_map& left, const flat_map& right)
	{
		left.assert_not_batching();
		right.assert_not_batching();
		return left._keys == right._keys && left._values == right._values;
	}

	[[nodiscard]] iterator begin() noexcept { assert_not_batching(); return iterator(this, 0); }
	[[nodiscard]] const_iterator begin() const noexcept { assert_not_batching(); return const_iterator(this, 0); }
	[[nodiscard]] const_iterator cbegin() const noexcept { return begin(); }
	[[nodiscard]] iterator end() noexcept { assert_not_batching(); return iterator(this, size()); }
	[[nodiscard]] const_iterator end() const noexcept { assert_not_batching(); return const_iterator(this, size()); }
	[[nodiscard]] const_iterator cend() const noexcept { return end(); }

	void clear() noexcept
	{
		_keys.clear();
		_values.clear();
		_batch_start = no_batch;
	}

	void reserve(size_type count)
	{
		_keys.reserve(count);
		_values.reserve(count);
	}

	void shrink_to_fit()
	{
		assert_not_batching();
		_keys.shrink_to_fit();
		_values.shrink_to_fit();
	}

	template <typename Query>
	[[nodiscard]] iterator find(const Query& key)
	{
		const auto index = find_index(key);
		return iterator(this, index);
	}

	template <typename Query>
	[[nodiscard]] const_iterator find(const Query& key) const
	{
		const auto index = find_index(key);
		return const_iterator(this, index);
	}

	template <typename Query>
	[[nodiscard]] bool contains(const Query& key) const { return find_index(key) != size(); }

	template <typename Query>
	[[nodiscard]] size_type count(const Query& key) const { return contains(key) ? 1 : 0; }

	template <typename Query>
	[[nodiscard]] iterator lower_bound(const Query& key) { return iterator(this, lower_bound_index(key)); }

	template <typename Query>
	[[nodiscard]] const_iterator lower_bound(const Query& key) const { return const_iterator(this, lower_bound_index(key)); }

	template <typename Query>
	[[nodiscard]] iterator upper_bound(const Query& key) { return iterator(this, upper_bound_index(key)); }

	template <typename Query>
	[[nodiscard]] const_iterator upper_bound(const Query& key) const { return const_iterator(this, upper_bound_index(key)); }

	template <typename Query>
	[[nodiscard]] mapped_reference at(const Query& key)
	{
		const auto index = find_index(key);
		if (index == size())
			throw std::out_of_range("flat_map::at");
		return _values[index];
	}

	template <typename Query>
	[[nodiscard]] const_mapped_reference at(const Query& key) const
	{
		const auto index = find_index(key);
		if (index == size())
			throw std::out_of_range("flat_map::at");
		return _values[index];
	}

	mapped_reference operator[](const Key& key) { return try_emplace(key).first.value(); }
	mapped_reference operator[](Key&& key) { return try_emplace(std::move(key)).first.value(); }

	std::pair<iterator, bool> insert(const value_type& value) { return try_emplace(value.first, value.second); }
	std::pair<iterator, bool> insert(value_type&& value) { return try_emplace(std::move(value.first), std::move(value.second)); }

	template <typename KeyArgument, typename... MappedArguments>
	std::pair<iterator, bool> try_emplace(KeyArgument&& key, MappedArguments&&... mapped_arguments)
	{
		assert_not_batching();
		const auto [existing_index, insertion_index] = find_or_insertion_index(key);
		if (existing_index != size())
			return { iterator(this, existing_index), false };

		_keys.insert(_keys.begin() + static_cast<difference_type>(insertion_index), std::forward<KeyArgument>(key));
		_values.emplace(_values.begin() + static_cast<difference_type>(insertion_index), std::forward<MappedArguments>(mapped_arguments)...);
		return { iterator(this, insertion_index), true };
	}

	template <typename KeyArgument, typename MappedArgument>
	std::pair<iterator, bool> insert_or_assign(KeyArgument&& key, MappedArgument&& value)
	{
		auto [position, inserted] = try_emplace(std::forward<KeyArgument>(key), std::forward<MappedArgument>(value));
		if (!inserted)
			position.value() = std::forward<MappedArgument>(value);
		return { position, inserted };
	}

	iterator erase(iterator position) { return erase(const_iterator(position)); }

	iterator erase(const_iterator position)
	{
		assert_not_batching();
		assert(position._container == this && position._index < size());
		const auto index = position._index;
		_keys.erase(_keys.begin() + static_cast<difference_type>(index));
		_values.erase(_values.begin() + static_cast<difference_type>(index));
		return iterator(this, index);
	}

	iterator erase(const_iterator first, const_iterator last)
	{
		assert_not_batching();
		assert(first._container == this && last._container == this && first._index <= last._index && last._index <= size());
		const auto first_index = first._index;
		_keys.erase(_keys.begin() + static_cast<difference_type>(first._index), _keys.begin() + static_cast<difference_type>(last._index));
		_values.erase(_values.begin() + static_cast<difference_type>(first._index), _values.begin() + static_cast<difference_type>(last._index));
		return iterator(this, first_index);
	}

	template <typename Query>
	size_type erase(const Query& key)
	{
		const auto position = find(key);
		if (position == end())
			return 0;
		erase(position);
		return 1;
	}

	template <typename InputIterator>
	void insert_sorted(InputIterator first, InputIterator last)
	{
		assert_not_batching();
		std::vector<Key> incoming_keys;
		std::vector<Mapped> incoming_values;
		if constexpr (std::forward_iterator<InputIterator>) {
			const auto count = static_cast<size_type>(std::distance(first, last));
			incoming_keys.reserve(count);
			incoming_values.reserve(count);
		}
		for (; first != last; ++first) {
			incoming_keys.emplace_back((*first).first);
			incoming_values.emplace_back((*first).second);
		}
		assert(std::is_sorted(incoming_keys.begin(), incoming_keys.end(), _compare));
		merge_sorted(std::move(incoming_keys), std::move(incoming_values));
	}

	void begin_batch()
	{
		assert_not_batching();
		_batch_start = size();
	}

	template <typename KeyArgument, typename MappedArgument>
	void append_unsorted(KeyArgument&& key, MappedArgument&& value)
	{
		assert(batch_open());
		_keys.emplace_back(std::forward<KeyArgument>(key));
		_values.emplace_back(std::forward<MappedArgument>(value));
	}

	void append_unsorted(const value_type& value) { append_unsorted(value.first, value.second); }
	void append_unsorted(value_type&& value) { append_unsorted(std::move(value.first), std::move(value.second)); }

	void end_batch()
	{
		assert(batch_open());
		const auto batch_start = _batch_start;
		_batch_start = no_batch;
		if (batch_start == 0) {
			sort_and_deduplicate();
			return;
		}

		std::vector<size_type> order(size() - batch_start);
		for (size_type index = 0; index < order.size(); ++index)
			order[index] = batch_start + index;
		sort_indices_by_key(order);

		std::vector<Key> incoming_keys;
		std::vector<Mapped> incoming_values;
		incoming_keys.reserve(order.size());
		incoming_values.reserve(order.size());
		for (const auto index : order) {
			incoming_keys.emplace_back(std::move(_keys[index]));
			incoming_values.emplace_back(std::move(_values[index]));
		}

		_keys.erase(_keys.begin() + static_cast<difference_type>(batch_start), _keys.end());
		_values.erase(_values.begin() + static_cast<difference_type>(batch_start), _values.end());
		merge_sorted(std::move(incoming_keys), std::move(incoming_values));
	}

private:
	static constexpr size_type no_batch = std::numeric_limits<size_type>::max();

	void assert_not_batching() const { assert(!batch_open()); }

	template <typename Query>
	[[nodiscard]] size_type lower_bound_index(const Query& key) const
	{
		assert_not_batching();
		return static_cast<size_type>(std::lower_bound(_keys.begin(), _keys.end(), key, _compare) - _keys.begin());
	}

	template <typename Query>
	[[nodiscard]] size_type upper_bound_index(const Query& key) const
	{
		assert_not_batching();
		return static_cast<size_type>(std::upper_bound(_keys.begin(), _keys.end(), key,
			[this](const auto& query, const Key& stored_key) { return _compare(query, stored_key); }) - _keys.begin());
	}

	template <typename Query>
	[[nodiscard]] size_type find_index(const Query& key) const
	{
		const auto index = lower_bound_index(key);
		if (index < size() && FlatContainerInternal::lower_bound_matches(_keys[index], key, _compare))
			return index;
		return size();
	}

	template <typename Query>
	[[nodiscard]] std::pair<size_type, size_type> find_or_insertion_index(const Query& key) const
	{
		const auto index = lower_bound_index(key);
		if (index < size() && FlatContainerInternal::lower_bound_matches(_keys[index], key, _compare))
			return { index, index };
		return { size(), index };
	}

	void merge_sorted(std::vector<Key>&& incoming_keys, std::vector<Mapped>&& incoming_values)
	{
		assert(incoming_keys.size() == incoming_values.size());
		if (incoming_keys.empty())
			return;
		if (empty()) {
			const auto unique_size = deduplicate_sorted(incoming_keys, incoming_values);
			incoming_keys.resize(unique_size);
			incoming_values.resize(unique_size);
			_keys = std::move(incoming_keys);
			_values = std::move(incoming_values);
			return;
		}

		std::vector<Key> merged_keys;
		std::vector<Mapped> merged_values;
		merged_keys.reserve(size() + incoming_keys.size());
		merged_values.reserve(size() + incoming_values.size());

		auto append_existing = [&](size_type index) {
			merged_keys.emplace_back(std::move(_keys[index]));
			merged_values.emplace_back(std::move(_values[index]));
		};
		auto append_incoming = [&](size_type index) {
			if (merged_keys.empty() || !FlatContainerInternal::sorted_keys_equal(merged_keys.back(), incoming_keys[index], _compare)) {
				merged_keys.emplace_back(std::move(incoming_keys[index]));
				merged_values.emplace_back(std::move(incoming_values[index]));
			}
		};

		auto existing_index = size_type{ 0 };
		auto incoming_index = size_type{ 0 };
		while (existing_index < size() && incoming_index < incoming_keys.size()) {
			if constexpr (FlatContainerInternal::EqualityComparable<Key, Key>) {
				if (_keys[existing_index] == incoming_keys[incoming_index]) {
					append_existing(existing_index++);
					++incoming_index;
				} else if (_compare(_keys[existing_index], incoming_keys[incoming_index])) {
					append_existing(existing_index++);
				} else {
					assert(_compare(incoming_keys[incoming_index], _keys[existing_index]));
					append_incoming(incoming_index++);
				}
			} else {
				if (_compare(_keys[existing_index], incoming_keys[incoming_index])) {
					append_existing(existing_index++);
				} else if (_compare(incoming_keys[incoming_index], _keys[existing_index])) {
					append_incoming(incoming_index++);
				} else {
					append_existing(existing_index++);
					++incoming_index;
				}
			}
		}
		while (existing_index < size())
			append_existing(existing_index++);
		while (incoming_index < incoming_keys.size())
			append_incoming(incoming_index++);

		_keys = std::move(merged_keys);
		_values = std::move(merged_values);
	}

	[[nodiscard]] size_type deduplicate_sorted(std::vector<Key>& keys, std::vector<Mapped>& values) const
	{
		assert(keys.size() == values.size());
		auto unique_size = size_type{ 0 };
		for (size_type index = 0; index < keys.size(); ++index) {
			if (unique_size != 0 && FlatContainerInternal::sorted_keys_equal(keys[unique_size - 1], keys[index], _compare))
				continue;
			if (unique_size != index) {
				keys[unique_size] = std::move(keys[index]);
				values[unique_size] = std::move(values[index]);
			}
			++unique_size;
		}
		return unique_size;
	}

	void sort_indices_by_key(std::vector<size_type>& indices) const
	{
		std::sort(indices.begin(), indices.end(), [this](size_type left, size_type right) {
			if constexpr (FlatContainerInternal::EqualityComparable<Key, Key>) {
				return _keys[left] == _keys[right] ? left < right : _compare(_keys[left], _keys[right]);
			} else {
				if (_compare(_keys[left], _keys[right]))
					return true;
				if (_compare(_keys[right], _keys[left]))
					return false;
				return left < right;
			}
		});
	}

	void sort_and_deduplicate()
	{
		std::vector<size_type> order(size());
		for (size_type index = 0; index < order.size(); ++index)
			order[index] = index;
		sort_indices_by_key(order);

		for (size_type start = 0; start < order.size(); ++start) {
			if (order[start] == start)
				continue;

			Key key = std::move(_keys[start]);
			Mapped value = std::move(_values[start]);
			auto destination = start;
			for (;;) {
				const auto source = order[destination];
				order[destination] = destination;
				if (source == start) {
					_keys[destination] = std::move(key);
					_values[destination] = std::move(value);
					break;
				}
				_keys[destination] = std::move(_keys[source]);
				_values[destination] = std::move(_values[source]);
				destination = source;
			}
		}

		const auto unique_size = deduplicate_sorted(_keys, _values);
		_keys.resize(unique_size);
		_values.resize(unique_size);
	}

	std::vector<Key> _keys;
	std::vector<Mapped> _values;
	[[no_unique_address]] Compare _compare{};
	// While a batch is open, only the prefix before _batch_start is ordered and searchable.
	size_type _batch_start = no_batch;
};

template <typename Key, typename Compare = std::less<>>
class flat_set
{
public:
	using key_type = Key;
	using value_type = Key;
	using key_compare = Compare;
	using value_compare = Compare;
	using size_type = typename std::vector<Key>::size_type;
	using difference_type = typename std::vector<Key>::difference_type;
	using reference = const Key&;
	using const_reference = const Key&;
	using const_iterator = typename std::vector<Key>::const_iterator;
	using iterator = const_iterator;

	flat_set() = default;
	explicit flat_set(Compare compare): _compare(std::move(compare)) {}

	flat_set(std::initializer_list<Key> values, Compare compare = {}): _compare(std::move(compare))
	{
		begin_batch();
		for (const auto& value : values)
			append_unsorted(value);
		end_batch();
	}

	[[nodiscard]] bool empty() const noexcept { return _keys.empty(); }
	[[nodiscard]] size_type size() const noexcept { return _keys.size(); }
	[[nodiscard]] bool batch_open() const noexcept { return _batch_start != no_batch; }
	[[nodiscard]] const Compare& key_comp() const noexcept { return _compare; }
	[[nodiscard]] const Compare& value_comp() const noexcept { return _compare; }

	[[nodiscard]] friend bool operator==(const flat_set& left, const flat_set& right)
	{
		left.assert_not_batching();
		right.assert_not_batching();
		return left._keys == right._keys;
	}

	[[nodiscard]] const_iterator begin() const noexcept { assert_not_batching(); return _keys.begin(); }
	[[nodiscard]] const_iterator cbegin() const noexcept { return begin(); }
	[[nodiscard]] const_iterator end() const noexcept { assert_not_batching(); return _keys.end(); }
	[[nodiscard]] const_iterator cend() const noexcept { return end(); }

	void clear() noexcept { _keys.clear(); _batch_start = no_batch; }
	void reserve(size_type count) { _keys.reserve(count); }
	void shrink_to_fit() { assert_not_batching(); _keys.shrink_to_fit(); }

	template <typename Query>
	[[nodiscard]] const_iterator find(const Query& key) const
	{
		const auto index = find_index(key);
		return _keys.begin() + static_cast<difference_type>(index);
	}

	template <typename Query>
	[[nodiscard]] bool contains(const Query& key) const { return find_index(key) != size(); }

	template <typename Query>
	[[nodiscard]] size_type count(const Query& key) const { return contains(key) ? 1 : 0; }

	template <typename Query>
	[[nodiscard]] const_iterator lower_bound(const Query& key) const { return _keys.begin() + static_cast<difference_type>(lower_bound_index(key)); }

	template <typename Query>
	[[nodiscard]] const_iterator upper_bound(const Query& key) const { return _keys.begin() + static_cast<difference_type>(upper_bound_index(key)); }

	template <typename KeyArgument>
	std::pair<const_iterator, bool> insert(KeyArgument&& key)
	{
		assert_not_batching();
		const auto [existing_index, insertion_index] = find_or_insertion_index(key);
		if (existing_index != size())
			return { _keys.begin() + static_cast<difference_type>(existing_index), false };

		const auto iterator = _keys.insert(_keys.begin() + static_cast<difference_type>(insertion_index), std::forward<KeyArgument>(key));
		return { iterator, true };
	}

	const_iterator erase(const_iterator position)
	{
		assert_not_batching();
		return _keys.erase(position);
	}

	const_iterator erase(const_iterator first, const_iterator last)
	{
		assert_not_batching();
		return _keys.erase(first, last);
	}

	template <typename Query>
	size_type erase(const Query& key)
	{
		const auto position = find(key);
		if (position == end())
			return 0;
		erase(position);
		return 1;
	}

	template <typename InputIterator>
	void insert_sorted(InputIterator first, InputIterator last)
	{
		assert_not_batching();
		std::vector<Key> incoming_keys(first, last);
		assert(std::is_sorted(incoming_keys.begin(), incoming_keys.end(), _compare));
		merge_sorted(std::move(incoming_keys));
	}

	void begin_batch()
	{
		assert_not_batching();
		_batch_start = size();
	}

	template <typename KeyArgument>
	void append_unsorted(KeyArgument&& key)
	{
		assert(batch_open());
		_keys.emplace_back(std::forward<KeyArgument>(key));
	}

	void end_batch()
	{
		assert(batch_open());
		const auto batch_start = _batch_start;
		_batch_start = no_batch;
		std::sort(_keys.begin() + static_cast<difference_type>(batch_start), _keys.end(), _compare);
		if (batch_start == 0) {
			_keys.erase(std::unique(_keys.begin(), _keys.end(),
				[this](const Key& left, const Key& right) { return FlatContainerInternal::sorted_keys_equal(left, right, _compare); }), _keys.end());
			return;
		}

		std::vector<Key> incoming_keys(std::make_move_iterator(_keys.begin() + static_cast<difference_type>(batch_start)), std::make_move_iterator(_keys.end()));
		_keys.erase(_keys.begin() + static_cast<difference_type>(batch_start), _keys.end());
		merge_sorted(std::move(incoming_keys));
	}

private:
	static constexpr size_type no_batch = std::numeric_limits<size_type>::max();

	void assert_not_batching() const { assert(!batch_open()); }

	template <typename Query>
	[[nodiscard]] size_type lower_bound_index(const Query& key) const
	{
		assert_not_batching();
		return static_cast<size_type>(std::lower_bound(_keys.begin(), _keys.end(), key, _compare) - _keys.begin());
	}

	template <typename Query>
	[[nodiscard]] size_type upper_bound_index(const Query& key) const
	{
		assert_not_batching();
		return static_cast<size_type>(std::upper_bound(_keys.begin(), _keys.end(), key,
			[this](const auto& query, const Key& stored_key) { return _compare(query, stored_key); }) - _keys.begin());
	}

	template <typename Query>
	[[nodiscard]] size_type find_index(const Query& key) const
	{
		const auto index = lower_bound_index(key);
		if (index < size() && FlatContainerInternal::lower_bound_matches(_keys[index], key, _compare))
			return index;
		return size();
	}

	template <typename Query>
	[[nodiscard]] std::pair<size_type, size_type> find_or_insertion_index(const Query& key) const
	{
		const auto index = lower_bound_index(key);
		if (index < size() && FlatContainerInternal::lower_bound_matches(_keys[index], key, _compare))
			return { index, index };
		return { size(), index };
	}

	void merge_sorted(std::vector<Key>&& incoming_keys)
	{
		if (incoming_keys.empty())
			return;
		if (empty()) {
			incoming_keys.erase(std::unique(incoming_keys.begin(), incoming_keys.end(),
				[this](const Key& left, const Key& right) { return FlatContainerInternal::sorted_keys_equal(left, right, _compare); }), incoming_keys.end());
			_keys = std::move(incoming_keys);
			return;
		}

		std::vector<Key> merged_keys;
		merged_keys.reserve(size() + incoming_keys.size());

		auto append_incoming = [&](size_type index) {
			if (merged_keys.empty() || !FlatContainerInternal::sorted_keys_equal(merged_keys.back(), incoming_keys[index], _compare))
				merged_keys.emplace_back(std::move(incoming_keys[index]));
		};

		auto existing_index = size_type{ 0 };
		auto incoming_index = size_type{ 0 };
		while (existing_index < size() && incoming_index < incoming_keys.size()) {
			if constexpr (FlatContainerInternal::EqualityComparable<Key, Key>) {
				if (_keys[existing_index] == incoming_keys[incoming_index]) {
					merged_keys.emplace_back(std::move(_keys[existing_index++]));
					++incoming_index;
				} else if (_compare(_keys[existing_index], incoming_keys[incoming_index])) {
					merged_keys.emplace_back(std::move(_keys[existing_index++]));
				} else {
					assert(_compare(incoming_keys[incoming_index], _keys[existing_index]));
					append_incoming(incoming_index++);
				}
			} else {
				if (_compare(_keys[existing_index], incoming_keys[incoming_index])) {
					merged_keys.emplace_back(std::move(_keys[existing_index++]));
				} else if (_compare(incoming_keys[incoming_index], _keys[existing_index])) {
					append_incoming(incoming_index++);
				} else {
					merged_keys.emplace_back(std::move(_keys[existing_index++]));
					++incoming_index;
				}
			}
		}
		while (existing_index < size())
			merged_keys.emplace_back(std::move(_keys[existing_index++]));
		while (incoming_index < incoming_keys.size())
			append_incoming(incoming_index++);

		_keys = std::move(merged_keys);
	}

	std::vector<Key> _keys;
	[[no_unique_address]] Compare _compare{};
	// While a batch is open, only the prefix before _batch_start is ordered and searchable.
	size_type _batch_start = no_batch;
};
