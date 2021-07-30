// Copyright (c) 2016-2021 LG Electronics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef UTIL_OBSERVER_LIST_H_
#define UTIL_OBSERVER_LIST_H_

#include <algorithm>
#include <vector>

template <class ObserverType>
class ObserverList {
 public:
  void AddObserver(ObserverType* observer);
  void RemoveObserver(ObserverType* observer);
  bool HasObserver(ObserverType* observer);
  void Clear();
  void Compact();
  size_t Size() { return observers_.size(); }

  typename std::vector<ObserverType*>::iterator Begin();
  typename std::vector<ObserverType*>::iterator End();

 private:
  std::vector<ObserverType*> observers_;
};

template <class ObserverType>
void ObserverList<ObserverType>::AddObserver(ObserverType* observer) {
  if (!observer)
    return;

  if (HasObserver(observer))
    return;

  observers_.push_back(observer);
}

template <class ObserverType>
void ObserverList<ObserverType>::RemoveObserver(ObserverType* observer) {
  if (!observer || !HasObserver(observer))
    return;

  observers_.erase(std::find(observers_.begin(), observers_.end(), observer));
}

template <class ObserverType>
bool ObserverList<ObserverType>::HasObserver(ObserverType* observer) {
  if (!observer)
    return false;

  if (std::find(observers_.begin(), observers_.end(), observer) !=
      observers_.end())
    return true;
  return false;
}

template <class ObserverType>
void ObserverList<ObserverType>::Clear() {
  observers_.clear();
}

template <class ObserverType>
void ObserverList<ObserverType>::Compact() {
  observers_.erase(std::remove(observers_.begin(), observers_.end(),
                               static_cast<ObserverType*>(NULL)),
                   observers_.end());
}

template <class ObserverType>
typename std::vector<ObserverType*>::iterator
ObserverList<ObserverType>::Begin() {
  return observers_.begin();
}

template <class ObserverType>
typename std::vector<ObserverType*>::iterator
ObserverList<ObserverType>::End() {
  return observers_.end();
}

#define FOR_EACH_OBSERVER(ObserverType, observer_list, func)              \
  if (observer_list.Size()) {                                             \
    for (std::vector<ObserverType*>::iterator it = observer_list.Begin(); \
         it != observer_list.End(); ++it)                                 \
      (*it)->func;                                                        \
  }

#endif  // UTIL_OBSERVER_LIST_H_
