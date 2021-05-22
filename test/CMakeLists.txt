# Copyright (c) 2021, A Effective Infrastructure Committee.
# All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

add_library(rocketmq-kit STATIC)
target_sources(
  rocketmq-kit
  PUBLIC message.h
  PRIVATE message.cxx)
target_link_libraries(rocketmq-kit PUBLIC logxx)

add_executable(commitlog commitlog.cxx)
target_link_libraries(commitlog PUBLIC rocketmq-kit)

add_executable(consumequeue consumequeue.cxx)
target_link_libraries(consumequeue PUBLIC rocketmq-kit)
