#pragma once

#include "logicalaccess/readerproviders/datatransport.hpp"

namespace logicalaccess
{

/**
 * A dummy DataTransport whose whole point is to allow extension
 * from C#.
 *
 * See DummyReaderUnit.
 */
class LLA_CORE_API DummyDataTransport : public DataTransport
{
  public:
    std::string getTransportType() const override;
    bool connect() override;
    void disconnect() override;
    bool isConnected() override;
    std::string getName() const override;
    void serialize(boost::property_tree::ptree &parentNode) override;
    void unSerialize(boost::property_tree::ptree &node) override;
    std::string getDefaultXmlNodeName() const override;

  protected:
    void send(const ByteVector &data) override;
    ByteVector receive(long int timeout) override;
};

}
