/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <common/exceptions/LogicError.hpp>
#include <common/SensorMap.hpp>

#include <memory>

#if defined(TARGET_PLATFORM_ANDROID)
#include <cstdint>
#endif

namespace royale
{
    namespace factory
    {
        template<class BridgeIfT>
        class IBridgeFactoryImpl;

        /**
        * Virtual base for bridge factories.
        *
        * Bridge factory implementations shouldn't inherit from this directly,
        * they should multiply-inherit the IBridgeFactory<...> interfaces they
        * actually implement.
        *
        */

        class IBridgeFactory
        {
        public:
            virtual ~IBridgeFactory() = default;

            /**
            * Initialize the bridge factory.
            * Must be called once before any of the getBridge* methods are called.
            *
            */
            virtual void initialize () = 0;

            /**
            * Test whether the factory can create the bridge interface given.
            */
            template<class BridgeIfT>
            bool supports()
            {
                auto factory = dynamic_cast<IBridgeFactoryImpl<BridgeIfT>*> (this);
                return factory != nullptr;
            }

            /**
            * Create a bridge interface of appropriate type.
            */
            template<class BridgeIfT>
            std::shared_ptr<BridgeIfT> create()
            {
                auto factory = dynamic_cast<IBridgeFactoryImpl<BridgeIfT>*> (this);
                if (!factory)
                {
                    throw royale::common::LogicError ("Wrong bridge type");
                }

                std::shared_ptr<BridgeIfT> retval;
                factory->createImpl (retval);
                return retval;
            }
        };

        /**
        * Template interface
        */
        template<class BridgeIfT>
        class IBridgeFactoryImpl : virtual public IBridgeFactory
        {
        protected:
            virtual ~IBridgeFactoryImpl() = default;

            virtual void createImpl (std::shared_ptr<BridgeIfT> &) = 0;
            friend class IBridgeFactory;
        };

    }
}
