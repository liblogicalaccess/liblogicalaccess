/**
* \file felicacommands.cpp
* \author Maxime C. <maxime-dev@islog.com>
* \brief FeliCa commands.
*/

#include <logicalaccess/logs.hpp>
#include <logicalaccess/myexception.hpp>
#include "felicacommands.hpp"

namespace logicalaccess
{
	unsigned short FeliCaCommands::requestService(unsigned short code)
	{
		std::vector<unsigned short> codes;
		codes.push_back(code);

		std::vector<unsigned short> versions = requestServices(codes);
		EXCEPTION_ASSERT_WITH_LOG(versions.size() > 0, LibLogicalAccessException, "Wrong versions result.");

		return versions.at(0);
	}

	ByteVector FeliCaCommands::read(unsigned short code, unsigned short block)
	{
		std::vector<unsigned short> blocks;
		blocks.push_back(block);

		return read(code, blocks);
	}
	
	ByteVector FeliCaCommands::read(unsigned short code, const std::vector<unsigned short>& blocks)
	{
		std::vector<unsigned short> codes;
		codes.push_back(code);

		return read(codes, blocks);
	}

	void FeliCaCommands::write(unsigned short code, unsigned short block, const ByteVector& data)
	{
		std::vector<unsigned short> blocks;
		blocks.push_back(block);

		write(code, blocks, data);
	}

	void FeliCaCommands::write(unsigned short code, const std::vector<unsigned short>& blocks, const ByteVector& data)
	{
		std::vector<unsigned short> codes;
		codes.push_back(code);

		write(codes, blocks, data);
	}
}