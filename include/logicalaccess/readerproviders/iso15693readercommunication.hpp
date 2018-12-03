/**
 * \file iso15693readercommunication.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief ISO15693 Reader communication.
 */

#ifndef LOGICALACCESS_ISO15693BREADERCOMMUNICATION_HPP
#define LOGICALACCESS_ISO15693BREADERCOMMUNICATION_HPP

#include <logicalaccess/readerproviders/readercommunication.hpp>

#include <string>
#include <vector>

namespace logicalaccess
{
/**
 * \brief A ISO15693 reader communication base class.
 */
class LLA_CORE_API ISO15693ReaderCommunication : public ReaderCommunication
{
};
}

#endif /* LOGICALACCESS_ISO15693BREADERCOMMUNICATION_HPP */