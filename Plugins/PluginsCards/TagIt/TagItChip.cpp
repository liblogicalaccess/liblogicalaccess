/**
 * \file TagItChip.cpp
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief Tag-It chip.
 */

#include "TagItChip.h"

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
	TagItChip::TagItChip()
		: ISO15693Chip("TagIt")
	{

	}

	TagItChip::~TagItChip()
	{

	}	

	boost::shared_ptr<LocationNode> TagItChip::getRootLocationNode()
	{
		boost::shared_ptr<LocationNode> rootNode = ISO15693Chip::getRootLocationNode();
		rootNode->setName("Tag-It");

		return rootNode;
	}
}