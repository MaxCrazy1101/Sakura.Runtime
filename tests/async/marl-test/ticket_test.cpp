// Copyright 2019 The Marl Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "marl_test.h"

#include "marl/ticket.h"

template<uint32_t N_THREADS>
void WithBoundScheduler<N_THREADS>::TestTicket()
{
  marl::Ticket::Queue queue;

  constexpr int count = 1000;
  std::atomic<int> next = {0};
  int result[count] = {};

  for (int i = 0; i < count; i++) {
    auto ticket = queue.take();
    marl::schedule([ticket, i, &result, &next] {
      ticket.wait();
      result[next++] = i;
      ticket.done();
    });
  }

  queue.take().wait();

  for (int i = 0; i < count; i++) {
    ASSERT_EQ(result[i], i);
  }
}

TEST_CASE_METHOD(WithBoundScheduler<0>, "TestTicket-0") { TestTicket(); };
TEST_CASE_METHOD(WithBoundScheduler<1>, "TestTicket-1") { TestTicket(); };
TEST_CASE_METHOD(WithBoundScheduler<2>, "TestTicket-2") { TestTicket(); };
TEST_CASE_METHOD(WithBoundScheduler<8>, "TestTicket-8") { TestTicket(); };
TEST_CASE_METHOD(WithBoundScheduler<32>, "TestTicket-32") { TestTicket(); };
