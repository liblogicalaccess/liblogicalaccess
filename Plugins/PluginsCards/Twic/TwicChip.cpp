/**
 * \file TwicChip.cpp
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief Twic chip.
 */

#include "TwicChip.h"
#include "TwicProfile.h"

#include <iostream>
#include <iomanip>
#include <sstream>

using std::endl;
using std::dec;
using std::hex;
using std::setfill;
using std::setw;
using std::ostringstream;
using std::istringstream;

#include "logicalaccess/Services/AccessControl/Formats/BitHelper.h"

namespace LOGICALACCESS
{
	TwicChip::TwicChip()
		: ISO7816Chip("Twic")
	{
		d_profile.reset(new TwicProfile());
	}

	TwicChip::~TwicChip()
	{

	}

	boost::shared_ptr<LocationNode> TwicChip::getRootLocationNode()
	{
		boost::shared_ptr<LocationNode> rootNode;
		rootNode.reset(new LocationNode());
		rootNode->setName("Twic");

		boost::shared_ptr<LocationNode> applNode;
		applNode.reset(new LocationNode());
		applNode->setName("Twic Application");
		applNode->setParent(rootNode);
		rootNode->getChildrens().push_back(applNode);

		boost::shared_ptr<LocationNode> ucuiNode;
		ucuiNode.reset(new LocationNode());
		ucuiNode->setName("Unsigned Cardholder Unique Identifier");
		boost::shared_ptr<TwicLocation> ucuiLocation;
		ucuiLocation.reset(new TwicLocation());
		ucuiLocation->dataObject = 0x5FC104;
		ucuiNode->setLocation(ucuiLocation);
		ucuiNode->setNeedAuthentication(true);
		ucuiNode->setLength(getTwicCardProvider()->getDataObjectLength(ucuiLocation->dataObject, true));
		ucuiNode->setParent(applNode);		
		applNode->getChildrens().push_back(ucuiNode);

		boost::shared_ptr<LocationNode> tpkNode;
		tpkNode.reset(new LocationNode());
		tpkNode->setName("TWIC Privacy Key");
		boost::shared_ptr<TwicLocation> tpkLocation;
		tpkLocation.reset(new TwicLocation());
		tpkLocation->dataObject = 0xDFC101;
		tpkNode->setLocation(tpkLocation);
		tpkNode->setNeedAuthentication(true);
		tpkNode->setLength(getTwicCardProvider()->getDataObjectLength(tpkLocation->dataObject, true));
		tpkNode->setParent(applNode);
		applNode->getChildrens().push_back(tpkNode);

		boost::shared_ptr<LocationNode> cuiNode;
		cuiNode.reset(new LocationNode());
		cuiNode->setName("Cardholder Unique Identifier");
		boost::shared_ptr<TwicLocation> cuiLocation;
		cuiLocation.reset(new TwicLocation());
		cuiLocation->dataObject = 0x5FC102;
		cuiNode->setLocation(cuiLocation);
		cuiNode->setNeedAuthentication(true);
		cuiNode->setLength(getTwicCardProvider()->getDataObjectLength(cuiLocation->dataObject, true));
		cuiNode->setParent(applNode);
		applNode->getChildrens().push_back(cuiNode);

		boost::shared_ptr<LocationNode> cfNode;
		cfNode.reset(new LocationNode());
		cfNode->setName("Cardholder Fingerprints");
		boost::shared_ptr<TwicLocation> cfLocation;
		cfLocation.reset(new TwicLocation());
		cfLocation->dataObject = 0xDFC103;
		cfNode->setLocation(cfLocation);
		cfNode->setNeedAuthentication(true);
		cfNode->setLength(getTwicCardProvider()->getDataObjectLength(cfLocation->dataObject, true));
		cfNode->setParent(applNode);
		applNode->getChildrens().push_back(cfNode);

		boost::shared_ptr<LocationNode> soNode;
		soNode.reset(new LocationNode());
		soNode->setName("Security Object");
		boost::shared_ptr<TwicLocation> soLocation;
		soLocation.reset(new TwicLocation());
		soLocation->dataObject = 0xDFC10F;
		soNode->setLocation(soLocation);
		soNode->setNeedAuthentication(true);
		soNode->setLength(getTwicCardProvider()->getDataObjectLength(soLocation->dataObject, true));
		soNode->setParent(applNode);
		applNode->getChildrens().push_back(soNode);

		return rootNode;
	}
}