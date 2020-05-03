#ifndef DEV_SPIRALGERBIL_BF_COMPILER_POLY_LIST_H_
#define DEV_SPIRALGERBIL_BF_COMPILER_POLY_LIST_H_

#include <iterator>
#include <memory>
#include <type_traits>
#include <vector>

#include "boost/iterator/indirect_iterator.hpp"

namespace dev::spiralgerbil::bf {

template <typename T>
class PolyList final {
 private:
  using InnerData = std::vector<std::unique_ptr<T>>;

 public:
  using value_type = T;
  using size_type	= std::size_t;
  using difference_type = std::ptrdiff_t;
  using reference =	T&;
  using const_reference = const T&;
  using pointer = T*;
  using const_pointer = const T*;
  class iterator;
  class const_iterator;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  PolyList() = default;

  template <typename... Args>
  PolyList(Args... values) { (push_back(values), ...); }

  // Move only.
  PolyList(PolyList&& other) = default;
  PolyList& operator=(PolyList&& other) = default;

  T& at(size_t pos) { return *data_.at(pos); }
  const T& at(size_t pos) const { return *data_.at(pos); }
  T& operator[](size_t pos) { return *data_[pos]; }
  const T& operator[](size_t pos) const { return *data_[pos]; }
  T& front() { return *data_.front(); }
  const T& front() const { return *data_.front(); }
  T& back() { return *data_.back(); }
  const T& back() const { return *data_.back(); }

  iterator begin() { return iterator(data_.begin()); }
  const_iterator cbegin() const { return const_iterator(data_.cbegin()); }
  const_iterator begin() const { return cbegin(); }
  iterator end() { return iterator(data_.end()); }
  const_iterator cend() const { return const_iterator(data_.cend()); }
  const_iterator end() const { return cend(); }
  reverse_iterator rbegin() { return std::reverse_iterator(iterator(data_.begin())); }
  const_reverse_iterator crbegin() const { return std::reverse_iterator(const_iterator(data_.cbegin())); }
  const_iterator rbegin() const { return crbegin(); }
  reverse_iterator rend() { return std::reverse_iterator(iterator(data_.end())); }
  const_reverse_iterator crend() const { return std::reverse_iterator(const_iterator(data_.cend())); }
  const_reverse_iterator rend() const { return crend(); }

  [[nodiscard]] bool empty() const { return data_.empty(); }
  size_t size() const { return data_.size(); }
  size_t max_size() const { return data_.max_size(); }
  void reserve(size_t new_cap) { data_.reserve(new_cap); }
  size_t capacity() const { return data_.capacity(); }

  void clear() { data_.clear(); }

  iterator insert(iterator before, std::unique_ptr<T> value) {
    return iterator(data_.insert(before.base(), std::move(value)));
  }

  iterator insert(iterator before, iterator from, iterator to) {
    return iterator(data_.insert(before.base(),
                                 std::make_move_iterator(from.base()),
                                 std::make_move_iterator(to.base())));
  }

  template <typename V, typename... Args>
  iterator emplace(iterator before, Args&&... args) {
    static_assert(std::is_base_of_v<T, V>);
    static_assert(std::has_virtual_destructor_v<T>);
    return iterator(data_.emplace(before.base(), std::forward<Args>(args)...));
  }

  iterator erase(iterator pos) {
    return iterator(data_.erase(pos.base()));
  }

  iterator erase(iterator from, iterator to) {
    return iterator(data_.erase(from.base(), to.base()));
  }

  template <typename V>
  V& push_back(std::unique_ptr<V> value) {
    static_assert(std::is_base_of_v<T, V>);
    return static_cast<V&>(*data_.emplace_back(std::move(value)));
  }

  template <typename V, typename = std::enable_if_t<std::is_base_of_v<T, V>>>
  V& push_back(V&& value) {
    static_assert(std::has_virtual_destructor_v<T>);
    return static_cast<V&>(*data_.emplace_back(std::make_unique<V>(std::forward<V>(value))));
  }

  template <typename V, typename... Args>
  V& emplace_back(Args&&... args) {
    static_assert(std::is_base_of_v<T, V>);
    static_assert(std::has_virtual_destructor_v<T>);
    return static_cast<V&>(*data_.emplace_back(
        std::make_unique<V>(std::forward<Args>(args)...)));
  }

  void pop_back() { data_.pop_back(); }
  void swap(PolyList& other) { data_.swap(other.data_); }

  template <typename V>
  [[nodiscard]] std::unique_ptr<V> remove(V* value) {
    static_assert(std::is_base_of_v<T, V>);
    for (auto iter = data_.begin(); iter != data_.end(); iter++) {
      if (iter->get() == value) {
        std::unique_ptr<T> temp_ptr = std::move(*iter);
        data_.erase(iter);
        return std::unique_ptr<V>(static_cast<V*>(temp_ptr.release()), std::move(temp_ptr.get_deleter()));
      }
    }
    return nullptr;
  }

  template <typename V>
  [[nodiscard]] std::unique_ptr<V> remove(const V& value) {
    static_assert(std::is_base_of_v<T, V>);
    return remove(&value);
  }

  bool operator==(const PolyList& other) const { return this == &other; }
  bool operator!=(const PolyList& other) const { return this != &other; }

 private:
  InnerData data_;
};

template <typename T>
class PolyList<T>::iterator : public boost::indirect_iterator<typename InnerData::iterator, T> {
 public:
  using boost::indirect_iterator<typename InnerData::iterator, T>::indirect_iterator;

  iterator(const boost::indirect_iterator<typename InnerData::iterator, T>& other)
      : iterator(other.base()) {}

  template <typename V>
  void replace(std::unique_ptr<V> new_value) {
    static_assert(std::is_base_of_v<T, V>);
    *this->base() = std::move(new_value);
  }

  template <typename V, typename... Args>
  void replace(Args&&... args) {
    static_assert(std::is_base_of_v<T, V>);
    *this->base() = std::make_unique<V>(std::forward<Args>(args)...);
  }
};

template <typename T>
class PolyList<T>::const_iterator : public boost::indirect_iterator<typename InnerData::const_iterator, const T> {
 public:
  using boost::indirect_iterator<typename InnerData::const_iterator, const T>::indirect_iterator;

  const_iterator(const boost::indirect_iterator<typename InnerData::const_iterator, const T>& other)
      : const_iterator(other.base()) {}
  
  const_iterator(const iterator& other) : boost::indirect_iterator<typename InnerData::const_iterator, const T>::indirect_iterator(other.base()) {}
};

}  // namespace dev::spiralgerbil::bf

#endif  // DEV_SPIRALGERBIL_BF_COMPILER_POLY_LIST_H_
