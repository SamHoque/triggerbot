/**
 * @file com_resource.h
 * @brief RAII wrapper for COM resources
 * @author Sam Hoque
 *
 * License: Do whatever the fuck you want
 */

#pragma once

/**
 * @class ComResource
 * @brief RAII wrapper for COM objects with move semantics
 *
 * Custom COM resource management class for proper RAII handling of COM objects.
 * Compatible with MinGW and MSVC.
 *
 * @tparam T COM interface type to be managed
 */
template <typename T>
class ComResource {
public:
    /**
     * @brief Default constructor initializes with null pointer
     */
    ComResource() : ptr(nullptr) {}

    /**
     * @brief Destructor automatically releases the COM resource
     */
    ~ComResource() {
        release();
    }

    /**
     * @brief Copy operations are deleted to prevent double-release
     */
    ComResource(const ComResource&) = delete;
    ComResource& operator=(const ComResource&) = delete;

    /**
     * @brief Move constructor transfers ownership
     */
    ComResource(ComResource&& other) noexcept : ptr(other.ptr) {
        other.ptr = nullptr;
    }

    /**
     * @brief Move assignment transfers ownership
     */
    ComResource& operator=(ComResource&& other) noexcept {
        if (this != &other) {
            release();
            ptr = other.ptr;
            other.ptr = nullptr;
        }
        return *this;
    }

    /**
     * @brief Release the COM resource if it exists
     */
    void release() {
        if (ptr) {
            ptr->Release();
            ptr = nullptr;
        }
    }

    /**
     * @brief Get the raw pointer
     * @return The underlying COM interface pointer
     */
    T* get() const { return ptr; }

    /**
     * @brief Get the address of the pointer for initialization
     * @return Address of the internal pointer
     */
    T** getAddressOf() { return &ptr; }

    /**
     * @brief Release current resources and return address for initialization
     * @return Address of the internal pointer after releasing any previous resource
     */
    T** releaseAndGetAddressOf() {
        release();
        return &ptr;
    }

    /**
     * @brief Arrow operator for direct member access
     * @return The COM interface pointer
     */
    T* operator->() const { return ptr; }

    /**
     * @brief Boolean conversion operator
     * @return true if the resource is valid, false otherwise
     */
    explicit operator bool() const { return ptr != nullptr; }

private:
    T* ptr; ///< The COM interface pointer
};