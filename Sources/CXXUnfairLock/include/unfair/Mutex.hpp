//
// SPDX-FileCopyrightText: 2020 Stephen F. Booth <contact@sbooth.dev>
// SPDX-License-Identifier: MIT
//
// Part of https://github.com/sbooth/CXXUnfairLock
//

#pragma once

#include <os/availability.h>
#include <os/lock.h>

#include <functional>
#include <mutex>
#include <optional>
#include <type_traits>
#include <utility>

namespace unfair {

/// A wrapper around os_unfair_lock satisfying the Lockable requirements.
///
/// This class may be used with std::lock_guard for a scope-based lock.
class __attribute__((capability("mutex"))) Mutex final {
  public:
    // MARK: Construction and Destruction

    /// Creates a new mutex.
    Mutex() noexcept = default;

    Mutex(const Mutex &) = delete;
    Mutex &operator=(const Mutex &) = delete;

    Mutex(Mutex &&) = delete;
    Mutex &operator=(Mutex &&) = delete;

    /// Destroys the mutex.
    ~Mutex() noexcept = default;

    // MARK: Lockable

    /// Locks the mutex.
    void lock() noexcept __attribute__((acquire_capability()));

    /// Locks the mutex.
    /// @param flags Flags to alter the behavior of the mutex.
    void lock(os_unfair_lock_flags_t flags) noexcept __attribute__((acquire_capability()))
    API_AVAILABLE(macos(15.0), ios(18.0), tvos(18.0), watchos(11.0), visionos(2.0));

    /// Unlocks the mutex.
    void unlock() noexcept __attribute__((release_capability()));

    /// Attempts to lock the mutex.
    /// @return true if the mutex was successfully locked, false if the mutex was already locked.
    [[nodiscard]] bool try_lock() noexcept __attribute__((try_acquire_capability(true)));

    // MARK: Scoped Locking

    /// Executes a callable within a locked scope.
    ///
    /// Acquires the mutex, blocking until it becomes available, then invokes the provided callable while holding the
    /// mutex. The mutex is released when the callable finishes, including if it exits by throwing an exception.
    /// @tparam Func The type of the callable object.
    /// @tparam Args The types of arguments to pass to the callable.
    /// @param func The lambda, function, or functor to execute.
    /// @param args Arguments to be perfectly forwarded to the callable.
    /// @return The result of the callable execution.
    /// @throw Any exception thrown by the callable.
    template <typename Func, typename... Args>
    auto withLock(Func &&func, Args &&...args) noexcept(std::is_nothrow_invocable_v<Func &&, Args &&...>)
            __attribute__((locks_excluded(this)));

    /// Attempts to execute a callable within a locked scope without blocking.
    ///
    /// Attempts to acquire the mutex without blocking. If acquisition succeeds, the callable is invoked while holding
    /// the mutex, which is released when the callable finishes, including if it exits by throwing an exception. If the
    /// mutex is already locked, the function returns immediately without executing the callable.
    /// @tparam Func The type of the callable object.
    /// @tparam Args The types of arguments to pass to the callable.
    /// @param func The callable to execute if the lock is acquired.
    /// @param args Arguments to be perfectly forwarded to the callable.
    /// @return For non-void functions, a std::optional containing the result of func if the lock was acquired, or
    /// std::nullopt otherwise; for void functions, a bool that is true if the lock was acquired and func executed,
    /// false otherwise.
    /// @throw Any exception thrown by the callable.
    template <typename Func, typename... Args>
    auto tryWithLock(Func &&func, Args &&...args) noexcept(std::is_nothrow_invocable_v<Func &&, Args &&...>)
            __attribute__((locks_excluded(this)));

    // MARK: Ownership

    /// Asserts that the calling thread is the current owner of the mutex.
    ///
    /// If the mutex is currently owned by the calling thread, this function returns.
    ///
    /// If the mutex is unlocked or owned by a different thread, this function asserts and terminates the process.
    void assertIsOwner() const noexcept __attribute__((assert_capability(this)));

    /// Asserts that the calling thread is not the current owner of the mutex.
    ///
    /// If the mutex is unlocked or owned by a different thread, this function returns.
    ///
    /// If the mutex is currently owned by the current thread, this function asserts and terminates the process.
    void assertIsNotOwner() const noexcept;

  private:
    /// The locking primitive.
    os_unfair_lock lock_{OS_UNFAIR_LOCK_INIT};
};

// MARK: - Implementation -

// MARK: Lockable

inline void Mutex::lock() noexcept { os_unfair_lock_lock(&lock_); }

inline void Mutex::lock(os_unfair_lock_flags_t flags) noexcept { os_unfair_lock_lock_with_flags(&lock_, flags); }

inline void Mutex::unlock() noexcept { os_unfair_lock_unlock(&lock_); }

inline bool Mutex::try_lock() noexcept { return os_unfair_lock_trylock(&lock_); }

// MARK: Scoped Locking

template <typename Func, typename... Args>
inline auto Mutex::withLock(Func &&func, Args &&...args) noexcept(std::is_nothrow_invocable_v<Func &&, Args &&...>) {
    std::lock_guard lock{*this};
    return std::invoke(std::forward<Func>(func), std::forward<Args>(args)...);
}

template <typename Func, typename... Args>
inline auto Mutex::tryWithLock(Func &&func, Args &&...args) noexcept(std::is_nothrow_invocable_v<Func &&, Args &&...>) {
    using ReturnType = std::invoke_result_t<Func &&, Args &&...>;
    using ResultType = std::conditional_t<std::is_void_v<ReturnType>, bool, std::optional<ReturnType>>;

    std::unique_lock lock{*this, std::try_to_lock};
    if (!lock) {
        return ResultType{};
    }

    if constexpr (std::is_void_v<ReturnType>) {
        std::invoke(std::forward<Func>(func), std::forward<Args>(args)...);
        return true;
    } else {
        return std::optional<ReturnType>{std::invoke(std::forward<Func>(func), std::forward<Args>(args)...)};
    }
}

// MARK: Ownership

inline void Mutex::assertIsOwner() const noexcept { os_unfair_lock_assert_owner(&lock_); }

inline void Mutex::assertIsNotOwner() const noexcept { os_unfair_lock_assert_not_owner(&lock_); }

} /* namespace unfair */
