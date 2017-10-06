/**
 * \file readercardadapter.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Reader/Card adapter base class.
 */

#include "logicalaccess/cards/readercardadapter.hpp"
#include "logicalaccess/bufferhelper.hpp"
#include "logicalaccess/myexception.hpp"
#include "logicalaccess/logs.hpp"
#include "logicalaccess/settings.hpp"

namespace logicalaccess
{
    std::vector<unsigned char> ReaderCardAdapter::adaptCommand(const std::vector<unsigned char>& command)
    {
        return command;
    }

    std::vector<unsigned char> ReaderCardAdapter::adaptAnswer(const std::vector<unsigned char>& answer)
    {
        return answer;
    }

    std::vector<unsigned char> ReaderCardAdapter::sendCommand(const std::vector<unsigned char>& command, long timeout)
    {
        std::vector<unsigned char> res;

        if (timeout == -1)
            timeout = Settings::getInstance()->DataTransportTimeout;

        if (d_dataTransport)
        {
            res = adaptAnswer(d_dataTransport->sendCommand(adaptCommand(command), timeout));

			if (res.size() > 0 && getResultChecker())
			{
				LOG(LogLevel::DEBUGS) << "Call ResultChecker..." << BufferHelper::getHex(res);
				getResultChecker()->CheckResult(&res[0], res.size());
			}
			else if (getResultChecker() && !getResultChecker()->AllowEmptyResult())
			{
				THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "ResultChecker is set but no data has been received !!!")
			}
        }
        else
        {
            LOG(LogLevel::ERRORS) << "Cannot transmit the command, data transport is not set!";
        }

        return res;
    }

ReaderCardAdapter::ReaderCardAdapter()
{

}

}