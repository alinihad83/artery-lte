//
// Copyright (C) 2014 Raphael Riebl <raphael.riebl@thi.de>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#ifndef ITSG5LTEMIDDLEWARE_H_
#define ITSG5LTEMIDDLEWARE_H_

#include "artery/application/Facilities.h"
#include "artery/application/VehicleDataProvider.h"
#include "artery/application/ItsG5Middleware.h"
#include "veins/base/modules/BaseApplLayer.h"
#include <omnetpp.h>
#include "UDPControlInfo_m.h"
#include <vanetza/access/data_request.hpp>
#include <vanetza/access/interface.hpp>
#include <vanetza/btp/data_interface.hpp>
#include <vanetza/btp/port_dispatcher.hpp>
#include <vanetza/common/clock.hpp>
#include <vanetza/dcc/access_control.hpp>
#include <vanetza/dcc/scheduler.hpp>
#include <vanetza/dcc/state_machine.hpp>
#include <vanetza/geonet/packet.hpp>
#include <vanetza/geonet/router.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <map>
#include <memory>


#include "UDPSocket.h"
#include "IPvXAddressResolver.h"

// forward declarations
namespace Veins { class TraCIMobility; }
class ItsG5BaseService;

/**
 * Middleware providing a runtime context for services.
 * It can be plugged in wherever a IBaseApplLayer implementation is required.
 */
class ItsG5LTEMiddleware : public ItsG5Middleware
{
	public:

		ItsG5LTEMiddleware();
		void request(const vanetza::access::DataRequest&, std::unique_ptr<vanetza::geonet::DownPacket>, bool sendWithLte);
		void request(const vanetza::access::DataRequest&, std::unique_ptr<vanetza::geonet::DownPacket>) override;

	protected:
		void initialize(int stage) override;
		void finish() override;
		void handleMessage(cMessage *msg) override;

	private:
		void initializeMiddleware();


		Veins::TraCIScenarioManager* manager;
		UDPSocket socket;
		int fromLte;
		int toLte;
		int ltePort;
};

#endif
