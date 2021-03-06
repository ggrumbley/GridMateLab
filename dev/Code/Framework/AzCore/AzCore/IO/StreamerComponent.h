/*
* All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
* its licensors.
*
* For complete copyright and license terms please see the LICENSE at the root of this
* distribution (the "License"). All use of this software is governed by the License,
* or, if provided, by the license below or the license accompanying this file. Do not
* remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*
*/
#ifndef AZCORE_STREAMER_COMPONENT_H
#define AZCORE_STREAMER_COMPONENT_H

#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/Math/Crc.h>

namespace AZ
{
    namespace Debug
    {
        class StreamerDriller;
    }

    namespace IO
    {
        class Streamer;
    }

    class StreamerComponentUserSettings;

    class StreamerComponentRequests
        : public AZ::EBusTraits
    {
    public:

        static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;

        using Bus = AZ::EBus<StreamerComponentRequests>;

        virtual AZ::IO::Streamer* GetStreamer() = 0;
    };

    /**
     *
     */
    class StreamerComponent
        : public Component
        , public AZ::SystemTickBus::Handler
        , public StreamerComponentRequests::Bus::Handler
    {
    public:
        AZ_COMPONENT(StreamerComponent, "{DA47D715-2710-49e2-BC94-EF81C311D89C}")

        StreamerComponent();

    private:
        //////////////////////////////////////////////////////////////////////////
        // Component base
        void Activate() override;
        void Deactivate() override;
        //////////////////////////////////////////////////////////////////////////

        //////////////////////////////////////////////////////////////////////////
        // StreamerComponentRequests::Bus
        AZ::IO::Streamer* GetStreamer() override;
        //////////////////////////////////////////////////////////////////////////

        //////////////////////////////////////////////////////////////////////////
        // AZ::SystemTickBus
        void OnSystemTick() override;
        //////////////////////////////////////////////////////////////////////////

        /// \ref ComponentDescriptor::GetProvidedServices
        static void GetProvidedServices(ComponentDescriptor::DependencyArrayType& provided);
        /// \ref ComponentDescriptor::GetIncompatibleServices
        static void GetIncompatibleServices(ComponentDescriptor::DependencyArrayType& incompatible);
        /// \ref ComponentDescriptor::GetDependentServices
        static void GetDependentServices(ComponentDescriptor::DependencyArrayType& dependent);
        /// \red ComponentDescriptor::Reflect
        static void Reflect(ReflectContext* reflection);

        unsigned int            m_maxNumOpenLimitedStream;      ///< \ref IO::Streamer::Descriptor
        unsigned int            m_cacheBlockSize;               ///< \ref IO::Streamer::Descriptor
        unsigned int            m_numCacheBlocks;               ///< \ref IO::Streamer::Descriptor
        int                     m_deviceThreadCpuId;            ///< CPU Id to use for all device threads
        int                     m_deviceThreadPriority;         ///< Priority of the device thread
        int                     m_deviceThreadSleepTimeMS;      ///< Time to sleep between device operations in milliseconds. Use with caution. -1 to ignore
        Debug::StreamerDriller* m_driller;
    };
}

#endif // AZCORE_STREAMER_COMPONENT_H
#pragma once
