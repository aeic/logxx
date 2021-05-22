// Copyright (c) 2021, A Effective Infrastructure Committee.
// All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include <cstdint>

#include <xlib/io/buffer/byte_buffer.hpp>

constexpr int32_t kBornhostAddressV6Flag = 0x1 << 4;
constexpr int32_t kStorehostAddressV6Flag = 0x1 << 5;

bool CheckMessage(xlib::byte_buffer byte_buffer);
