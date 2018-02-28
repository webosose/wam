// Copyright (c) 2016-2018 LG Electronics, Inc.
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

#ifndef ObserverList_H
#define ObserverList_H

#include <vector>

template <class ObserverType>
class ObserverList {
public:
    void addObserver(ObserverType* observer);
    void removeObserver(ObserverType* observer);
    bool hasObserver(ObserverType* observer);
    void clear();
    void compact();
    size_t size() { return m_observers.size(); }

    typename std::vector<ObserverType*>::iterator begin();
    typename std::vector<ObserverType*>::iterator end();

private:
    std::vector<ObserverType*> m_observers;
};

template <class ObserverType>
void ObserverList<ObserverType>::addObserver(ObserverType* observer)
{
    if (!observer)
        return;

    if (hasObserver(observer))
        return;

    m_observers.push_back(observer);
}

template <class ObserverType>
void ObserverList<ObserverType>::removeObserver(ObserverType* observer)
{
    if (!observer || !hasObserver(observer))
        return;

    m_observers.erase(std::find(m_observers.begin(), m_observers.end(), observer));
}

template <class ObserverType>
bool ObserverList<ObserverType>::hasObserver(ObserverType* observer)
{
    if (!observer)
        return false;

    if (std::find(m_observers.begin(), m_observers.end(), observer) != m_observers.end())
        return true;
    return false;
}

template <class ObserverType>
void ObserverList<ObserverType>::clear()
{
    m_observers.clear();
}

template <class ObserverType>
void ObserverList<ObserverType>::compact()
{
    m_observers.erase(std::remove(m_observers.begin(), m_observers.end(), static_cast<ObserverType*>(NULL)), m_observers.end());
}

template <class ObserverType>
typename std::vector<ObserverType*>::iterator ObserverList<ObserverType>::begin()
{
    return m_observers.begin();
}

template <class ObserverType>
typename std::vector<ObserverType*>::iterator ObserverList<ObserverType>::end()
{
    return m_observers.end();
}

#define FOR_EACH_OBSERVER(ObserverType, observer_list, func)                                                   \
    if (observer_list.size()) {                                                                                \
        for (std::vector<ObserverType*>::iterator it = observer_list.begin(); it != observer_list.end(); ++it) \
            (*it)->func;                                                                                       \
    }

#endif /* ObserverList_H */
