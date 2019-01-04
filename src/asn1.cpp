#include <logicalaccess/asn1.hpp>
#include <logicalaccess/plugins/llacommon/logs.hpp>
#include <logicalaccess/myexception.hpp>
#include <sstream>
#include <cmath>

namespace logicalaccess
{
	ByteVector ASN1::encode_oid(const std::string &oid)
	{
        std::vector<unsigned int> values;
        std::stringstream stream(oid);
        std::string s;
        while (getline(stream, s, '.'))
        {
            values.push_back(std::stoi(s));
        }
        return encode_oid(values);
	}

	ByteVector ASN1::encode_oid(const std::vector<unsigned int> &oid)
	{
        ByteVector r;

        if (oid.size() < 2)
		{
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                      "OID must have at least two nodes.");
		} 

		// The first two nodes of the OID are encoded onto a single byte
        r.push_back(static_cast<unsigned char>((oid[0] * 40) + oid[1]));
        
		for (int i = 2; i < oid.size(); ++i)
		{
            // Node values less than or equal to 127 are encoded on one byte.
			if (oid[i] <= 127)
			{
				r.push_back(oid[i]);
			}
			// Node values greater than or equal to 128 are encoded on
			// multiple bytes.
			//   Bit 7 of the leftmost byte is set to one.
			//   Bits 0 through 6 of each byte contains the encoded value.
			else
			{
                int mb = static_cast<int>(std::ceil(oid[i] / (double)0xff));
                for (int j = mb - 1; j >= 0; --j)
				{
                    unsigned char b = static_cast<unsigned char>(
                        (oid[i] >> (7 * j)) & 0x7f);
                    if (j == mb - 1)
					{
                        b |= 0x80;
					}

                    r.push_back(b);
				}
			}
		}

        return r;
	}
}
