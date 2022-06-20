#pragma once

#include <initializer_list>
#include <iterator>
#include <vector>
#include <forward_list>

template<class KeyType, class ValueType, class Hash = std::hash<KeyType>>
class UnorderedMap 
{
public:
	typedef std::pair<const KeyType, ValueType> Element;
	typedef typename std::forward_list<Element>::iterator iterator;
	typedef typename std::forward_list<Element>::const_iterator const_iterator;

private:
	Hash hash_;
	std::forward_list<Element> elements_;
	std::vector<std::forward_list<iterator>> table_;
	size_t size_;

	constexpr static float maxLoadFactor_ = 1.0;

	typename std::forward_list<iterator>::iterator Find_Previous_In_Bucket(size_t bucketIndex, const KeyType& key)
	{
		auto it = table_[bucketIndex].before_begin();
		while (next(it) != table_[bucketIndex].end() && !(next(*next(it))->first == key))
			++it;
		return it;
	}

public:
	UnorderedMap& operator=(const UnorderedMap& other)
	{
		if (this == &other)
			return *this;
		clear();
		for (auto it = other.begin(); it != other.end(); ++it)
			insert(*it);

		return *this;
	}

	explicit UnorderedMap(Hash h = Hash{}) : size_(0), hash_(h) { table_.resize(1); }

	template<typename Iter>
	UnorderedMap(Iter first, Iter last, Hash h = Hash{}) : UnorderedMap(h)
	{
		for (auto it = first; it != last; ++it)
			insert(*it);
	}

	UnorderedMap(std::initializer_list<Element> init_list, Hash h = Hash{}) : UnorderedMap(h)
	{
		for (const auto& it : init_list)
			insert(it);
	}

	void ReHash()
	{
		size_t prevSize = table_.size();
		table_.resize(2 * prevSize);

		for (auto it = begin(); it != end(); ++it)
		{
			size_t h = hash_(it->first);
			size_t prevBucketIndex = h % prevSize;
			size_t newBucketIndex = h % table_.size();

			if (prevBucketIndex != newBucketIndex)
			{
				auto jt = Find_Previous_In_Bucket(prevBucketIndex, it->first);
				table_[newBucketIndex].push_front(*next(jt));
				table_[prevBucketIndex].erase_after(jt);
			}
		}
	}

	ValueType& operator[](const KeyType& key)
	{
		size_t bucketIndex = hash_(key) % table_.size();
		auto it = ++Find_Previous_In_Bucket(bucketIndex, key);

		if (it != table_[bucketIndex].end())
			return next(*it)->second;

		insert({ key, ValueType() });
		return elements_.begin()->second;
	}

	const ValueType& at(const KeyType& key) const
	{
		size_t bucketIndex = hash_(key) % table_.size();
		auto it = ++Find_Previous_In_Bucket(bucketIndex, key);

		if (it == table_[bucketIndex].end())
			throw std::out_of_range("");

		return next(*it)->second;
	}

	void clear()
	{
		for (auto it = elements_.begin(); it != elements_.end(); ++it)
		{
			size_t bucketIndex = hash_(it->first) % table_.size();
			table_[bucketIndex].clear();
		}
		elements_.clear();
		size_ = 0;
	}

	Hash hashFunction() const { return hash_; }

	size_t size() const { return size_; }

	bool empty() const { return elements_.empty(); }

	iterator begin() { return elements_.begin(); }
	const_iterator begin() const { return elements_.begin(); }

	iterator end() { return elements_.end(); }
	const_iterator end() const { return elements_.end(); }

	iterator find(const KeyType& key)
	{
		size_t bucketIndex = hash_(key) % table_.size();
		auto it = ++Find_Previous_In_Bucket(bucketIndex, key);

		if (it == table_[bucketIndex].end())
			return end();

		return next(*it);
	}

	const_iterator find(const KeyType& key) const
	{
		size_t bucketIndex = hash_(key) % table_.size();
		auto it = ++Find_Previous_In_Bucket(bucketIndex, key);

		if (it == table_[bucketIndex].end())
			return end();

		return next(*it);
	}

	void insert(const Element& p)
	{
		if (find(p.first) != end())
			return;

		if (size_ > 0)
		{
			size_t bucketIndex = hash_(begin()->first) % table_.size();
			auto it = ++Find_Previous_In_Bucket(bucketIndex, begin()->first);
			elements_.push_front(p);
			*it = begin();
		}
		else
		{
			elements_.push_front(p);
		}
		++size_;

		size_t bucketIndex = hash_(p.first) % table_.size();
		table_[bucketIndex].push_front(elements_.before_begin());

		if (static_cast<double>(size_) / table_.size() > maxLoadFactor_)
			ReHash();
	}

	void erase(const KeyType& key)
	{
		size_t bucketIndex = hash_(key) % table_.size();
		auto it = Find_Previous_In_Bucket(bucketIndex, key);

		if (next(it) == table_[bucketIndex].end())
			return;
		--size_;

		auto jt = next(*next(it));
		if (next(jt) != end())
		{
			size_t tmpBucketIndex = hash_(next(jt)->first) % table_.size();
			auto kt = ++Find_Previous_In_Bucket(tmpBucketIndex, next(jt)->first);
			*kt = *next(it);
		}

		elements_.erase_after(*next(it));
		table_[bucketIndex].erase_after(it);
	}
};