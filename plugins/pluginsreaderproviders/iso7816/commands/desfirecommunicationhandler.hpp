
#ifndef LOGICALACCESS_DESFIRECOMMUNICATIONHANDLER_HPP
#define LOGICALACCESS_DESFIRECOMMUNICATIONHANDLER_HPP

#include <vector>

#include "desfire/desfirelocation.hpp"

namespace logicalaccess
{
	class LIBLOGICALACCESS_API DESFireCommunicationHandler
	{
	public:
		virtual ~DESFireCommunicationHandler() {}

		virtual std::vector<unsigned char> handleReadData(unsigned char err, const std::vector<unsigned char>& firstMsg, unsigned int length, EncryptionMode mode) = 0;

		virtual void handleWriteData(unsigned char cmd, unsigned char* parameters, unsigned int paramLength, const std::vector<unsigned char>& data, EncryptionMode mode) = 0;

	};
}
#endif //LOGICALACCESS_DESFIRECOMMUNICATIONHANDLER_HPP