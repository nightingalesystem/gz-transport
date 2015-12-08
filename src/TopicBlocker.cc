/*
 * Copyright (C) 2015 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

#include <mutex>
#include "ignition/transport/TopicBlocker.hh"
#include "ignition/transport/TopicBlockerPrivate.hh"
#include "ignition/transport/TopicUtils.hh"

using namespace ignition;
using namespace transport;

//////////////////////////////////////////////////
TopicBlocker::TopicBlocker()
  : dataPtr(new TopicBlockerPrivate())
{
}

//////////////////////////////////////////////////
TopicBlocker::~TopicBlocker()
{
}

//////////////////////////////////////////////////
bool TopicBlocker::Wait(const unsigned int _timeout)
{
  auto now = std::chrono::system_clock::now();
  std::unique_lock<std::mutex> lk(this->dataPtr->mutex);
  this->dataPtr->blocked = true;

  return this->dataPtr->condition.wait_until(lk,
    now + std::chrono::milliseconds(_timeout),
    [this]
    {
      return !this->dataPtr->blocked;
    });
}

//////////////////////////////////////////////////
void TopicBlocker::Release()
{
  {
    std::lock_guard<std::mutex> lk(this->dataPtr->mutex);
    this->dataPtr->blocked = false;
  }
  this->dataPtr->condition.notify_one();
}

//////////////////////////////////////////////////
bool TopicBlocker::Blocked() const
{
  std::lock_guard<std::mutex> lk(this->dataPtr->mutex);
  return this->dataPtr->blocked;
}
