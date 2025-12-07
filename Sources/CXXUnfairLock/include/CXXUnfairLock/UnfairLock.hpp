//
// Copyright © 2020-2025 Stephen F. Booth
// Part of https://github.com/sbooth/CXXUnfairLock
// MIT license
//

#pragma once

#import <os/availability.h>
#import <os/lock.h>

namespace CXXUnfairLock {

/// A wrapper around os_unfair_lock satisfying the Lockable requirements.
///
/// This class may be used with std::lock_guard for a scope-based lock.
class __attribute__((capability("mutex"))) UnfairLock final {
public:
	// MARK: Creation and Destruction

	/// Creates a new unfair lock.
	UnfairLock() noexcept = default;

	UnfairLock(const UnfairLock&) = delete;
	UnfairLock& operator=(const UnfairLock&) = delete;

//	UnfairLock(const UnfairLock&&) = delete;
//	UnfairLock& operator=(const UnfairLock&&) = delete;

	/// Destroys the unfair lock.
	~UnfairLock() noexcept = default;

	// MARK: Lockable

	/// Locks the lock.
	void lock() noexcept __attribute__((acquire_capability()))
	{
		os_unfair_lock_lock(&lock_);
	}

	/// Locks the lock.
	/// @param flags Flags to alter the behavior of the lock.
	API_AVAILABLE(macos(15.0), ios(18.0), tvos(18.0), watchos(11.0), visionos(2.0))
	void lock(os_unfair_lock_flags_t flags) noexcept __attribute__((acquire_capability()))
	{
		os_unfair_lock_lock_with_flags(&lock_, flags);
	}

	/// Unlocks the lock.
	void unlock() noexcept __attribute__((release_capability()))
	{
		os_unfair_lock_unlock(&lock_);
	}

	/// Attempts to lock the lock.
	/// @return true if the lock was successfully locked, false if the lock was already locked.
	bool try_lock() noexcept __attribute__((try_acquire_capability(true)))
	{
		return os_unfair_lock_trylock(&lock_);
	}

	// MARK: Ownership

	/// Asserts that the calling thread is the current owner of the lock.
	///
	/// If the lock is currently owned by the calling thread, this function returns.
	///
	/// If the lock is unlocked or owned by a different thread, this function
	/// asserts and terminates the process.
	void assert_owner() noexcept
	{
		os_unfair_lock_assert_owner(&lock_);
	}

	///	Asserts that the calling thread is not the current owner of the lock.
	///
	///	If the lock is unlocked or owned by a different thread, this function returns.
	///
	///	If the lock is currently owned by the current thread, this function asserts
	///	and terminates the process.
	void assert_not_owner() noexcept
	{
		os_unfair_lock_assert_not_owner(&lock_);
	}

private:
	/// The primitive lock.
	os_unfair_lock lock_{OS_UNFAIR_LOCK_INIT};
};

} /* namespace CXXUnfairLock */
