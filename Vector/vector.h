#pragma once
#include <cassert>
#include <cstdlib>
#include <new>
#include <utility>
#include <memory>
#include <algorithm>

template <typename T>
class RawMemory {
public:
    RawMemory() = default;

    explicit RawMemory(size_t capacity)
        : buffer_(Allocate(capacity))
        , capacity_(capacity) {
    }
    
    RawMemory(const RawMemory&) = delete;
    RawMemory& operator=(const RawMemory& rhs) = delete;
    RawMemory(RawMemory&& other) noexcept {
        delete[] buffer_;
        buffer_ = other.buffer_;
        other.buffer_ = nullptr;
        capacity_ = other.capacity_;
        other.capacity_ = 0;
        
    }
    RawMemory& operator=(RawMemory&& rhs) noexcept {
        delete[] buffer_;
        buffer_ = rhs.buffer_;
        rhs.buffer_ = nullptr;
        capacity_ = rhs.capacity_;
        rhs.capacity_ = 0;
        return *this;
    }

    ~RawMemory() {
        Deallocate(buffer_);
    }

    T* operator+(size_t offset) noexcept {
        // Разрешается получать адрес ячейки памяти, следующей за последним элементом массива
        assert(offset <= capacity_);
        return buffer_ + offset;
    }

    const T* operator+(size_t offset) const noexcept {
        return const_cast<RawMemory&>(*this) + offset;
    }

    const T& operator[](size_t index) const noexcept {
        return const_cast<RawMemory&>(*this)[index];
    }

    T& operator[](size_t index) noexcept {
        assert(index < capacity_);
        return buffer_[index];
    }

    void Swap(RawMemory& other) noexcept {
        std::swap(buffer_, other.buffer_);
        std::swap(capacity_, other.capacity_);
    }

    const T* GetAddress() const noexcept {
        return buffer_;
    }

    T* GetAddress() noexcept {
        return buffer_;
    }

    size_t Capacity() const {
        return capacity_;
    }

private:
    // Выделяет сырую память под n элементов и возвращает указатель на неё
    static T* Allocate(size_t n) {
        return n != 0 ? static_cast<T*>(operator new(n * sizeof(T))) : nullptr;
    }

    // Освобождает сырую память, выделенную ранее по адресу buf при помощи Allocate
    static void Deallocate(T* buf) noexcept {
        operator delete(buf);
    }

    T* buffer_ = nullptr;
    size_t capacity_ = 0;
};

template <typename T>
class Vector {
public:
    using iterator = T*;
    using const_iterator = const T*;
    
    Vector() = default;

    explicit Vector(size_t size) : data_(size), size_(size) {
        std::uninitialized_value_construct_n(data_.GetAddress(), size);
    }
    
    Vector(const Vector& other) : data_(other.size_), size_(other.size_) {
        std::uninitialized_copy_n(other.data_.GetAddress(), size_, data_.GetAddress());
    }
    
    Vector(Vector&& other) : data_(std::move(other.data_)), size_(other.size_) {
        other.size_ = 0;
    }
    
    Vector& operator=(const Vector& rhs) {
        if (this != &rhs) {
            if (rhs.size_ > data_.Capacity()) {
                Vector rhs_copy(rhs);
                Swap(rhs_copy);
            } else {
                auto new_end = std::copy_n(rhs.data_.GetAddress(), 
                                           (rhs.size_ > size_) ? size_ : rhs.size_, data_.GetAddress());
                if (rhs.size_ > size_) {
                    std::uninitialized_copy_n(rhs.data_.GetAddress() + size_, rhs.size_ - size_, new_end);
                } else {
                    DestroyN(new_end, size_ - rhs.size_);
                }
                size_ = rhs.size_;
            }
        }
        return *this;
    }
    
    Vector& operator=(Vector&& rhs) noexcept {
        Swap(rhs);
        return *this;
    }
    
    ~Vector() {
        DestroyN(data_.GetAddress(), size_);
    }
    
    size_t Size() const noexcept {
        return size_;
    }

    size_t Capacity() const noexcept {
        return data_.Capacity();
    }
    
    void Reserve(size_t new_capacity) {
        if (new_capacity <= data_.Capacity()) {
            return;
        }
        RawMemory<T> new_data(new_capacity);
        CopyOrMove(data_.GetAddress(), size_, new_data.GetAddress());
        data_.Swap(new_data);
    }
    
    void Resize(size_t new_size) {
        if (new_size < size_) {
            DestroyN(data_.GetAddress() + new_size, size_ - new_size);
        }
        else {
            Reserve(new_size);
            std::uninitialized_value_construct_n(data_.GetAddress() + size_, new_size - size_);
        }
        size_ = new_size;
    }
    
    template <typename... Args>
    T& EmplaceBack(Args&&... args) {
        if (size_ == Capacity()) {
            RawMemory<T> new_data(size_ == 0 ? 1 : size_ * 2);
            new (new_data + size_) T(std::forward<Args>(args)...);
            CopyOrMove(data_.GetAddress(), size_, new_data.GetAddress());
            data_.Swap(new_data);
        }
        else {
            new (data_ + size_) T(std::forward<Args>(args)...);
        }
        ++size_;
        return *(data_.GetAddress() + size_-1);
    }
    
    void PushBack(const T& value) {
        EmplaceBack(value);
    }
    
    void PushBack(T&& value) {
        EmplaceBack(std::move(value));
    }
    
    template <typename... Args>
    iterator Emplace(const_iterator pos, Args&&... args) {
        if (pos == end()) {
            return &EmplaceBack(std::forward<Args>(args)...);
        }
        
        size_t i = pos - begin();
        if (size_ == Capacity()) {
            RawMemory<T> new_data(size_ == 0 ? 1 : size_ * 2);
            new (new_data + i) T(std::forward<Args>(args)...);
            try {
                CopyOrMove(begin(), i, new_data.GetAddress());
            }
            catch(...) {
                (new_data.GetAddress()+i)->~T();
            }
            try {
                CopyOrMove(begin() + i, size_ - i, new_data.GetAddress()+i+1);
            }
            catch(...) {
                DestroyN(new_data.GetAddress(), i + 1);
            }
            data_.Swap(new_data);
        }
        else {
            T temp(std::forward<Args>(args)...);
            new (end()) T(std::move(data_[size_ - 1]));
            std::move_backward(begin() + i, end() - 1, end());
            data_[i] = std::move(temp);
        }
        ++size_;
        return begin() + i;
    }
    
    iterator Insert(const_iterator pos, const T& value) {
        return Emplace(pos, value);
    }
    iterator Insert(const_iterator pos, T&& value) {
        return Emplace(pos, std::move(value));
    }
    
    iterator Erase(const_iterator pos) noexcept(std::is_nothrow_move_assignable_v<T>) {
        size_t i = pos - begin();
        if (size_ != 0) {
            std::move(begin() + i + 1, end(), begin() + i);
            data_[size_ - 1].~T();
            --size_;
            return begin() + i;
        }
        return begin() + i;
    }
    
    void PopBack() noexcept {
        if (size_ != 0) {
            Destroy(data_.GetAddress() + size_-1);
            --size_;
        }
    }
    
    void Swap(Vector& other) noexcept {
        data_.Swap(other.data_);
        std::swap(size_, other.size_);
    }

    const T& operator[](size_t index) const noexcept {
        return const_cast<Vector&>(*this)[index];
    }

    T& operator[](size_t index) noexcept {
        assert(index < size_);
        return data_[index];
    }
    
    iterator begin() noexcept {
        return data_.GetAddress();
    }
    iterator end() noexcept {
        return data_.GetAddress() + size_;
    }
    const_iterator cbegin() const noexcept {
        return data_.GetAddress();
    }
    const_iterator cend() const noexcept {
        return data_.GetAddress() + size_;
    }
    const_iterator begin() const noexcept {
        return cbegin();
    }
    const_iterator end() const noexcept {
        return cend();
    }

private:
    RawMemory<T> data_;
    size_t size_ = 0;
    
    // Выделяет сырую память под n элементов и возвращает указатель на неё
    static T* Allocate(size_t n) {
        return n != 0 ? static_cast<T*>(operator new(n * sizeof(T))) : nullptr;
    }

    // Освобождает сырую память, выделенную ранее по адресу buf при помощи Allocate
    static void Deallocate(T* buf) noexcept {
        operator delete(buf);
    }
    
    // Вызывает деструкторы n объектов массива по адресу buf
    static void DestroyN(T* buf, size_t n) noexcept {
        for (size_t i = 0; i != n; ++i) {
            Destroy(buf + i);
        }
    }

    // Создаёт копию объекта elem в сырой памяти по адресу buf
    static void CopyConstruct(T* buf, const T& elem) {
        new (buf) T(elem);
    }

    // Вызывает деструктор объекта по адресу buf
    static void Destroy(T* buf) noexcept {
        buf->~T();
    }
    
    void CopyOrMove(iterator first1, size_t size, iterator first2) {
        if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>) {
            std::uninitialized_move_n(first1, size, first2);
        } else {
            std::uninitialized_copy_n(first1, size, first2);
        }
        DestroyN(first1, size);
    }
};