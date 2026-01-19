// swift-tools-version: 5.9
//
// SPDX-FileCopyrightText: 2025 Stephen F. Booth <contact@sbooth.dev>
// SPDX-License-Identifier: MIT
//
// Part of https://github.com/sbooth/CXXUnfairLock
//

import PackageDescription

let package = Package(
    name: "CXXUnfairLock",
    products: [
        .library(
            name: "CXXUnfairLock",
            targets: [
                "CXXUnfairLock",
            ]
        ),
    ],
    targets: [
        .target(
            name: "CXXUnfairLock",
            cSettings: [
                .headerSearchPath("include/cxx_lock"),
            ]
        ),
        .testTarget(
            name: "CXXUnfairLockTests",
            dependencies: [
                "CXXUnfairLock",
            ],
            swiftSettings: [
                .interoperabilityMode(.Cxx),
            ]
        ),
    ],
    cxxLanguageStandard: .cxx17
)
