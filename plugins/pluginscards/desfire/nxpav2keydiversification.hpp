#ifndef NXPAV2KEYDIVERSIFICATION_HPP__
#define NXPAV2KEYDIVERSIFICATION_HPP__

#include "logicalaccess/cards/keydiversification.hpp"
#include "logicalaccess/key.hpp"
#include "nxpkeydiversification.hpp"
#include <vector>
#include <string>

namespace logicalaccess
{
class LIBLOGICALACCESS_API NXPAV2KeyDiversification : public NXPKeyDiversification
{
  public:
    void initDiversification(ByteVector identifier, int AID, std::shared_ptr<Key> key,
                             unsigned char keyno, ByteVector &diversify) override;
    ByteVector getDiversifiedKey(std::shared_ptr<Key> key, ByteVector diversify) override;

    NXPAV2KeyDiversification()
        : d_revertAID(false)
        , d_forceK2Use(false)
    {
    }

    explicit NXPAV2KeyDiversification(const ByteVector &divInput)
        : d_revertAID(false)
        , d_divInput(divInput)
        , d_forceK2Use(false)
    {
    }

    virtual ~NXPAV2KeyDiversification()
    {
    }

    std::string getType() override
    {
        return "NXPAV2";
    }

    void serialize(boost::property_tree::ptree &parentNode) override;
    void unSerialize(boost::property_tree::ptree &node) override;
    std::string getDefaultXmlNodeName() const override
    {
        return "NXPAV2KeyDiversification";
    }

    bool getRevertAID() const
    {
        return d_revertAID;
    }

    void setRevertAID(bool revertAID)
    {
        d_revertAID = revertAID;
    }

    bool getForceK2Use() const
    {
        return d_forceK2Use;
    }

    void setForceK2Use(bool forceK2Use)
    {
        d_forceK2Use = forceK2Use;
    }

    const ByteVector &getDivInput() const
    {
        return d_divInput;
    }

    void setDivInput(ByteVector divInput)
    {
        d_divInput = divInput;
    }

    const ByteVector &getSystemIdentifier() const
    {
        return d_systemIdentifier;
    }

    void setSystemIdentifier(ByteVector systemIdentifier)
    {
        d_systemIdentifier = systemIdentifier;
    }

  private:
    bool d_revertAID;

    ByteVector d_divInput;

    ByteVector d_systemIdentifier;

    bool d_forceK2Use;
};
}

#endif