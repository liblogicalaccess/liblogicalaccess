#include <logicalaccess/asn1.hpp>
#include <logicalaccess/plugins/llacommon/logs.hpp>
#include <logicalaccess/myexception.hpp>
#include <sstream>

namespace logicalaccess
{
	ByteVector ASN1::encode_oid(const std::string &oid)
	{
        std::vector<unsigned int> values;
        std::stringstream stream(oid);
        unsigned int n;
        while (stream >> n)
        {
            values.push_back(n);
        }
        return encode_oid(values);
	}

	ByteVector ASN1::encode_oid(const std::vector<unsigned int> &oid)
	{
		// TODO
        return ByteVector();
	}
}
