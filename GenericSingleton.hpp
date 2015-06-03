// -*- C++ -*-
/* 
 * File:   GenericSingleton.hpp
 * Author: baodn
 *
 * Created on June 27, 2014, 10:34 AM
 */

#ifndef GENERIC_SINGLETON_H
#define	GENERIC_SINGLETON_H

template <class T>
class GenericSingleton
{
public:
    static T& Instance()
    {
        static T _instance;
        return _instance;
    }
};

#endif //GENERIC_SINGLETON_H


