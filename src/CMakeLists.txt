# Copyright (c) 2021, A Effective Infrastructure Committee.
# All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
add_library(logxx STATIC)
target_sources(
  logxx
  PUBLIC ${LOGXX_INCLUDE_DIR}/logxx/append_entry_status.h
         ${LOGXX_INCLUDE_DIR}/logxx/append_entry_result.h
         ${LOGXX_INCLUDE_DIR}/logxx/basic_ledger.hpp
         ${LOGXX_INCLUDE_DIR}/logxx/basic_logstream.hpp
         ${LOGXX_INCLUDE_DIR}/logxx/variable_ledger.h
         ${LOGXX_INCLUDE_DIR}/logxx/variable_logstream.h
         ${LOGXX_INCLUDE_DIR}/logxx/fixed_ledger.h
         ${LOGXX_INCLUDE_DIR}/logxx/fixed_logstream.h
  PRIVATE ${LOGXX_SOURCE_DIR}/variable_ledger.cxx
          ${LOGXX_SOURCE_DIR}/variable_logstream.cxx
          ${LOGXX_SOURCE_DIR}/fixed_ledger.cxx
          ${LOGXX_SOURCE_DIR}/fixed_logstream.cxx)
target_include_directories(logxx PUBLIC ${LOGXX_INCLUDE_DIR})
target_link_libraries(logxx PUBLIC xlib::xlib absl::str_format)
