// Copyright (c) 2021, A Effective Infrastructure Committee.
// All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "message.h"

#include <cstdint>
#include <iostream>

#include <xlib/io/buffer/byte_buffer.hpp>

bool CheckMessage(xlib::byte_buffer byte_buffer) {
  auto body_crc = byte_buffer.get<int32_t>();
  auto queue_id = byte_buffer.get<int32_t>();
  auto flag = byte_buffer.get<int32_t>();
  auto queue_offset = byte_buffer.get<int64_t>();
  auto physical_offset = byte_buffer.get<int64_t>();
  auto system_flag = byte_buffer.get<int32_t>();
  auto born_timestamp = byte_buffer.get<int64_t>();
  byte_buffer.skip((system_flag & kBornhostAddressV6Flag) == 0 ? 4 + 4 : 16 + 4);
  auto store_timestamp = byte_buffer.get<int64_t>();
  byte_buffer.skip((system_flag & kStorehostAddressV6Flag) == 0 ? 4 + 4 : 16 + 4);
  auto reconsume_times = byte_buffer.get<int32_t>();
  auto prepared_transaction_offset = byte_buffer.get<int64_t>();
  auto body_length = byte_buffer.get<int32_t>();
  std::string body = [&byte_buffer, body_length]() {
    if (body_length > 0) {
      auto body_buffer = byte_buffer.slice(body_length);
      return std::string{reinterpret_cast<char*>(body_buffer.raw()), body_buffer.limit()};
    }
    return std::string{};
  }();
  byte_buffer.skip(body_length);
  auto topic_length = byte_buffer.get<int8_t>();
  if (topic_length <= 0) {
    std::cout << "ERROR!!! NO TOPIC" << std::endl;
    return false;
  }
  auto topic_buffer = byte_buffer.slice(topic_length);
  std::string topic{reinterpret_cast<char*>(topic_buffer.raw()), topic_buffer.limit()};
  byte_buffer.skip(topic_length);
  auto properties_length = byte_buffer.get<int16_t>();
  byte_buffer.skip(properties_length);
  if (byte_buffer.has_remaining()) {
    std::cout << "ERROR!!! HAS REMAINING!" << std::endl;
    return false;
  }

  // std::cout << topic << "@[" << queue_id << ":" << queue_offset << "] is: " << body << std::endl;
  return true;
}
