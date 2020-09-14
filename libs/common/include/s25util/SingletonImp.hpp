// Copyright (c) 2005 - 2015 Settlers Freaks (sf-team at siedler25.org)
//
// This file is part of Return To The Roots.
//
// Return To The Roots is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Return To The Roots is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Return To The Roots. If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include <cassert>

// IWYU pragma: private, include "Singleton.h"

template<typename T, template<class> class L>
T* Singleton<T, L>::me;

template<typename T, template<class> class L>
bool& Singleton<T, L>::getIsDestroyed()
{
    static bool is_destroyed = false;
    return is_destroyed;
}

template<typename T, template<class> class L>
Singleton<T, L>::Singleton()
{
    // Init invalid access detection
    getIsDestroyed() = false;
}

template<typename T, template<class> class L>
Singleton<T, L>::~Singleton()
{
    getIsDestroyed() = true;
}

template<typename T, template<class> class LifetimePolicy>
void Singleton<T, LifetimePolicy>::MakeInstance()
{
    if(me)
        return;
    if(getIsDestroyed())
    {
        LifetimePolicy<T>::OnDeadReference();
        getIsDestroyed() = false;
    }
    me = new T;
    LifetimePolicy<T>::ScheduleDestruction(me, &DestroySingleton);
}

template<typename T, template<class> class L>
void Singleton<T, L>::DestroySingleton()
{
    assert(!getIsDestroyed());
    delete me;
    me = nullptr;
    getIsDestroyed() = true;
}

template<typename T, template<class> class L>
T& Singleton<T, L>::inst()
{
    if(!me)
        MakeInstance();
    return *me; // NOLINT
}
