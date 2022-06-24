#pragma once

#include <algorithm>
#include <initializer_list>
#include <iostream>
#include "AVLTree.h"
#include <list>

// DECLARATION

template <class TKey, class TValue>
class Iterator;

template <class TKey, class TValue>
class ConstIterator;

template <class TKey, class TValue>
class Map {
private:
	AVL<TKey, TValue> container;
public:
	Map() = default;
	Map(const std::initializer_list<std::pair<TKey, TValue>>& list) {
		for (std::pair<TKey, TValue> e : list) {
			container.insert(e.first, e.second);
		}
	}

	void insert(const TKey& key, const TValue& value) {
		container.insert(key, value);
	}

	void insert(const std::pair<TKey, TValue>& pair) {
		container.insert(pair.first, pair.second);
	}

	bool search(const TKey& key, TValue& value) const {
		TValue* tmp = container.search(key);
		if (tmp == nullptr) {
			return false;
		}
		else {
			value = *tmp;
			return true;
		}
	}

	bool at(const TKey& key) const {
		return container.search(key) != nullptr;
	}

	TValue& operator[](const TKey& key) {
		return *container.search(key);
	}

	const TValue& operator[](const TKey& key) const {
		return *container.search(key);
	}

	void remove(const TKey& key) {
		container.deleteKey(key);
	}

	unsigned size() const {
		return container.length();
	}

	void show() { container.inorderPrint(); }

	
	using MyIterator = Iterator<TKey, TValue>;
	using MyConstIterator = ConstIterator<TKey, TValue>;

	MyIterator begin() {
		return MyIterator(container.getRoot());
	}
	MyIterator end() {
		return MyIterator(nullptr);
	}

	MyConstIterator begin() const {
		return MyConstIterator(container.getRoot());
	}
	MyConstIterator end() const {
		return MyConstIterator(nullptr);
	}
};

template <class TKey, class TValue>
class ConstIterator {
	using MyIterator = ConstIterator<TKey, TValue>;
	using NodePtr = TNode<TKey, TValue>*;
private:
	NodePtr ptr;
	std::stack<NodePtr, std::list<NodePtr>> parents;

	NodePtr min(NodePtr node) {
		if (node == nullptr) {
			return nullptr;
		}
		else if (node->leftChild == nullptr) {
			return node;
		}
		else {
			parents.push(node);
			return min(node->leftChild);
		}
	}

	NodePtr next(NodePtr node) {
		if (node->rightChild != nullptr) {
			parents.push(node);
			return min(node->rightChild);
		}

		NodePtr parent = parents.top();
		parents.pop();
		while (parent != nullptr && node == parent->rightChild) {
			node = parent;
			parent = parents.top();
			parents.pop();
		}
		return parent;
	}
public:
	ConstIterator(NodePtr root = nullptr) {
		parents.push(nullptr);
		ptr = min(root);
	}

	ConstIterator(const Iterator<TKey, TValue>& it) : ptr(it.ptr), parents(it.parents) {}

	bool operator==(const MyIterator& other) const {
		return ptr == other.ptr;
	}
	bool operator!=(const MyIterator& other) const {
		return ptr != other.ptr;
	}

	const std::pair<TKey, TValue>& operator*() const {
		return ptr->pair;
	}

	MyIterator& operator++() {
		ptr = next(ptr);
		return *this;
	}

	MyIterator operator++(int) {
		MyIterator copy(*this);
		++* this;
		return copy;
	}

};


template <class TKey, class TValue>
class Iterator {
	using MyIterator = Iterator<TKey, TValue>;
	using NodePtr = TNode<TKey, TValue>*;
private:
	NodePtr ptr;
	std::stack<NodePtr, std::list<NodePtr>> parents;
	friend ConstIterator<TKey, TValue>;

	NodePtr min(NodePtr node) {
		if (node == nullptr) {
			return nullptr;
		}
		else if (node->leftChild == nullptr) {
			return node;
		}
		else {
			parents.push(node);
			return min(node->leftChild);
		}
	}

	NodePtr next(NodePtr node) {
		if (node->rightChild != nullptr) {
			parents.push(node);
			return min(node->rightChild);
		}

		NodePtr parent = parents.top();
		parents.pop();
		while (parent != nullptr && node == parent->rightChild) {
			node = parent;
			parent = parents.top();
			parents.pop();
		}
		return parent;
	}
public:
	Iterator(NodePtr root = nullptr) {
		parents.push(nullptr);
		ptr = min(root);
	}

	bool operator==(const MyIterator& other) const {
		return ptr == other.ptr;
	}
	bool operator!=(const MyIterator& other) const {
		return ptr != other.ptr;
	}

	const std::pair<TKey, TValue>& operator*() const {
		return ptr->pair;
	}

	std::pair<const TKey, TValue>& operator*() {
		return reinterpret_cast<std::pair<const TKey, TValue>&>(ptr->pair);
	}

	MyIterator& operator++() {
		ptr = next(ptr);
		return *this;
	}

	MyIterator operator++(int) {
		MyIterator copy(*this);
		++* this;
		return copy;
	}
};


