// Copyright (c) 2021, A Effective Infrastructure Committee.
// All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include <cstdint>

#include <iostream>
#include <utility>

#include <logxx/fixed_logstream.h>
#include <logxx/variable_logstream.h>
#include <xlib/filesystem.hpp>

#include "message.h"

bool CheckMessageIndex(xlib::byte_buffer byte_buffer, logxx::VariableLogStream& commitlog) {
  auto physical_offset = byte_buffer.get<int64_t>();
  auto frame_size = byte_buffer.get<int32_t>();
  if (physical_offset < 0 || frame_size <= 0) {
    return false;
  }
  auto tag_code = byte_buffer.get<int64_t>();
  auto* ledger = commitlog.FindLedgerByOffset(physical_offset);
  if (ledger != nullptr) {
    auto byte_buffer = ledger->SelectEntry(physical_offset, frame_size);
    return CheckMessage(std::move(byte_buffer));
  }
  return true;
}

int main() {
  xlib::fs::path commitlog_path{"/Users/james/store/commitlog"};
  int32_t commitlog_size = 1024 * 1024 * 1024;  // 1GB
  logxx::VariableLogStream commitlog{commitlog_path, commitlog_size};
  commitlog.Load();

  xlib::fs::path consume_queue_path{"/Users/james/store/consumequeue/test_yinweihe/0"};
  int32_t consume_queue_size = 300000 * 20;  // 5.72MB
  logxx::FixedLogStream<20> consume_queue{consume_queue_path, consume_queue_size};
  consume_queue.Load();
  int64_t physical_offset = consume_queue.Recover([&commitlog](xlib::byte_buffer byte_buffer) -> bool {
    return CheckMessageIndex(std::move(byte_buffer), commitlog);
  });

  std::cout << "physical offset: " << physical_offset << std::endl;
  return 0;
}
