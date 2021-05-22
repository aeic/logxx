// Copyright (c) 2021, A Effective Infrastructure Committee.
// All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include <cstdint>

#include <iostream>

#include <logxx/variable_logstream.h>
#include <xlib/filesystem.hpp>

#include "message.h"

int main() {
  xlib::fs::path commitlog_path{"/Users/james/store/commitlog"};
  int32_t commitlog_size = 1024 * 1024 * 1024;  // 1GB
  logxx::VariableLogStream commitlog{commitlog_path, commitlog_size};
  commitlog.Load();
  int64_t physical_offset = commitlog.Recover(CheckMessage);
  std::cout << "physical offset: " << physical_offset << std::endl;
  return 0;
}
