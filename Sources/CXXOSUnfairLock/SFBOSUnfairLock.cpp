//
// Copyright © 2020-2025 Stephen F. Booth
// Part of https://github.com/sbooth/CXXOSUnfairLock
// MIT license
//

#import "SFBOSUnfairLock.hpp"

// These methods could also be included in the header
// but SPM doesn't support header-only libraries.

void SFB::OSUnfairLock::assert_owner() noexcept
{
	os_unfair_lock_assert_owner(&lock_);
}

void SFB::OSUnfairLock::assert_not_owner() noexcept
{
	os_unfair_lock_assert_not_owner(&lock_);
}
