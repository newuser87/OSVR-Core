/** @file
    @brief Implementation

    @date 2015

    @author
    Sensics, Inc.
    <http://sensics.com/osvr>
*/

// Copyright 2015 Sensics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// 	http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Internal Includes
#include "EyeTrackerRemoteFactory.h"
#include "VRPNConnectionCollection.h"
#include <osvr/Common/ClientInterface.h>
#include <osvr/Common/PathTreeFull.h>
#include <osvr/Util/ChannelCountC.h>
#include <osvr/Util/UniquePtr.h>
#include <osvr/Common/OriginalSource.h>
#include "InterfaceTree.h"
#include <osvr/Util/Verbosity.h>
#include <osvr/Common/CreateDevice.h>

#include <osvr/Common/EyeTrackerComponent.h>
#include <osvr/Common/Location2DComponent.h>
#include <osvr/Common/DirectionComponent.h>

// Library/third-party includes
#include <boost/lexical_cast.hpp>
#include <boost/any.hpp>
#include <boost/variant/get.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>
#include <json/value.h>
#include <json/reader.h>

// Standard includes
#include <iostream>
#include <string>

namespace osvr {
namespace client {

    class NetworkEyeTrackerRemoteHandler : public RemoteHandler {
      public:

		  struct Options {
			  Options()
				  : reportDirection(false), reportBasePoint(false),
				  reportLocation2D(false), reportBlink(false),
				  dirIface() {}
			  bool reportDirection;
			  bool reportBasePoint;
			  bool reportLocation2D;
			  bool reportBlink;
			  osvr::common::ClientInterfacePtr dirIface;
			  osvr::common::ClientInterfacePtr locationIface;
			  osvr::common::ClientInterfacePtr trackerIface;
			  osvr::common::ClientInterfacePtr buttonIface;
		  };


        NetworkEyeTrackerRemoteHandler(vrpn_ConnectionPtr const &conn,
                                    std::string const &deviceName,
                                    boost::optional<OSVR_ChannelCount> sensor,
                                    common::InterfaceList &ifaces)
            : m_dev(common::createClientDevice(deviceName, conn)),
              m_interfaces(ifaces), m_all(!sensor.is_initialized()),
              m_sensor(sensor) {
            auto eyetracker = common::EyeTrackerComponent::create();
			m_dev->addComponent(eyetracker);
			eyetracker->registerEyeHandler(
				[&](common::OSVR_EyeNotification const &data,
                    util::time::TimeValue const &timestamp) {
                    m_handleEyeTracking(data, timestamp);
                });
			OSVR_DEV_VERBOSE("Constructed an Eye Handler for "
                             << deviceName);

        }

        /// @brief Deleted assignment operator.
        NetworkEyeTrackerRemoteHandler &
        operator=(NetworkEyeTrackerRemoteHandler const &) = delete;

        virtual ~NetworkEyeTrackerRemoteHandler() {
            /// @todo do we need to unregister?
        }

        virtual void update() { m_dev->update(); }

      private:

		  /*
		  void m_handleEyeTracking3d(common::OSVR_EyeNotification const &data,
			  util::time::TimeValue const &timestamp) {

			  OSVR_EyeTracker3DReport report;
			  report.sensor = data.sensor;
			  report.directionValid = false;
			  report.basePointValid = false;
			  if (m_opts.reportDirection) {
				//  report.directionValid = m_opts.dirIface->getState<OSVR_DirectionState>(timestamp, report.direction);
			  }
			  if (m_opts.reportBasePoint) {
				//  report.basePointValid = m_opts.trackerIface->getState<OSVR_PositionReport>(timestamp, report.basePoint);
			  }
			  if (!(report.basePointValid || report.directionValid)) {
				  return; // don't send an empty report.
			  }
			  for (auto &iface : m_interfaces) {
				  iface->triggerCallbacks(timestamp, report);
			  }
		  }
		  
		  void m_handleEyeTracking2d(common::OSVR_EyeNotification const &data,
			  util::time::TimeValue const &timestamp) {

			  OSVR_EyeTracker2DReport report;
			  report.sensor = data.sensor;
			  report.locationValid = false;
			  OSVR_OrientationReport rep;
			  if (m_opts.reportLocation2D) {
				 // report.locationValid = m_opts.locationIface->getState<OSVR_OrientationReport>(timestamp, rep.rotation);
			  }
			  if (!(report.locationValid)) {
				  return; // don't send an empty report.
			  }
			  for (auto &iface : m_interfaces) {
				  iface->triggerCallbacks(timestamp, report);
			  }
		  }
		  */
		  void m_handleEyeTracking(common::OSVR_EyeNotification const &data,
                           util::time::TimeValue const &timestamp) {
			if (!m_all && *m_sensor != data.sensor) {
                /// doesn't match our filter.
                return;
            }

			/*
            OSVR_EyeTrackerReport report;
			report.sensor = 99;

            common::ClientInterfacePtr anInterface;
            for (auto &iface : m_interfaces) {
                anInterface = iface;
                iface->triggerCallbacks(timestamp, report);
            }
			*/

/*
			m_handleEyeTracking3d(data, timestamp);
			m_handleEyeTracking2d(data, timestamp);
*/
        }

        common::BaseDevicePtr m_dev;
        common::InterfaceList &m_interfaces;
        bool m_all;
		Options m_opts;
        boost::optional<OSVR_ChannelCount> m_sensor;
    };

    EyeTrackerRemoteFactory::EyeTrackerRemoteFactory(
        VRPNConnectionCollection const &conns)
        : m_conns(conns) {}

    shared_ptr<RemoteHandler> EyeTrackerRemoteFactory::
    operator()(common::OriginalSource const &source,
	common::InterfaceList &ifaces,
	common::ClientContext &ctx) {

        shared_ptr<RemoteHandler> ret;

		NetworkEyeTrackerRemoteHandler::Options opts;

		auto myDescriptor = source.getDeviceElement().getDescriptor();		
	
		if (myDescriptor["interfaces"]["eyetracker"].isMember("direction")) {
			opts.reportDirection = true;
			const std::string iface = source.getDeviceElement().getDeviceName() +
				"/direction" ;
			// + boost::lexical_cast<std::string>(source.getSensorNumberAsChannelCount())
			opts.dirIface = ctx.getInterface(iface.c_str());
		}
		if (myDescriptor["interfaces"]["eyetracker"].isMember("tracker")) {
			opts.reportBasePoint = true;
			const std::string iface = (source.getDeviceElement().getDeviceName() + "/tracker/");
			// + boost::lexical_cast<std::string>(source.getSensorNumberAsChannelCount())
			opts.trackerIface = ctx.getInterface(iface.c_str());
		}
		
		if (myDescriptor["interfaces"]["eyetracker"].isMember("location2D")) {
			opts.reportLocation2D = true;
			const std::string iface = (source.getDeviceElement().getDeviceName() + "/location2D/");
			// + boost::lexical_cast<std::string>(source.getSensorNumberAsChannelCount())
			opts.locationIface = ctx.getInterface(iface.c_str());
		}
		if (myDescriptor["interfaces"]["eyetracker"].isMember("button")) {
			opts.reportBlink = true;
			const std::string iface = (source.getDeviceElement().getDeviceName() + "/button/");
			// + boost::lexical_cast<std::string>(source.getSensorNumberAsChannelCount())
			opts.buttonIface = ctx.getInterface(iface.c_str());
		}

        if (source.hasTransform()) {
            OSVR_DEV_VERBOSE(
                "Ignoring transform found on route for Eye Tracker data!");
		}

        /// @todo This is where we'd take a different path for IPC imaging data.
        auto const &devElt = source.getDeviceElement();

        /// @todo find out why make_shared causes a crash here
        ret.reset(new NetworkEyeTrackerRemoteHandler(
            m_conns.getConnection(devElt), devElt.getFullDeviceName(),
            source.getSensorNumberAsChannelCount(), ifaces));
        return ret;
    }

} // namespace client
} // namespace osvr