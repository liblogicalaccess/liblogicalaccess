/**
 * \file NfcData.cpp
 * \author Remi W.
 * \brief NFC data.
 */

#include <logicalaccess/services/nfctag/nfcdata.hpp>
#include <logicalaccess/services/nfctag/ndefmessage.hpp>
#include <logicalaccess/services/nfctag/lockcontroltlv.hpp>
#include <logicalaccess/services/nfctag/memorycontroltlv.hpp>
#include <iostream>

namespace logicalaccess
{

   NfcData::NfcData() : m_type(0)
   {

   }

   NfcData::NfcData(char type) : m_type(type)
   {

   }

   NfcData::~NfcData()
   {

   }

   char NfcData::getType() const
   {
     return m_type;
   }

   void NfcData::setType(char type)
   {
     m_type = type;
   }

    std::vector<std::shared_ptr<NfcData> > NfcData::tlvToData(ByteVector tlv)
   {
      std::vector<std::shared_ptr<NfcData> > messageList;

      unsigned short i = 0;

      while (i + 1u < tlv.size())
      {
          switch (tlv[i++])
          {
          case 0x00: // Null
              break;
          case 0x01: // Lock
            if (tlv.size() >= i + 1u + tlv[i])
            {
              ByteVector msgdata(tlv.begin() + i - 1, tlv.begin() + i + 1 + tlv[i]);
              std::shared_ptr<LockControlTlv> lock = std::make_shared<LockControlTlv>(msgdata);
              messageList.push_back(lock);
              i += tlv[i] + 1;
            }
            break;
          case 0x02: // Memory control
          {
            if (tlv.size() >= i + 1u + tlv[i])
            {
              ByteVector msgdata(tlv.begin() + i - 1, tlv.begin() + i + 1 + tlv[i]);
              std::shared_ptr<MemoryControlTlv> mem = std::make_shared<MemoryControlTlv>(msgdata);
              messageList.push_back(mem);
              i += tlv[i] + 1;
            }
            break;
          }
          case 0xFD: // Proprietary
              i += tlv[i];
              break;
          case 0x03: // Ndef message
              if (tlv.size() >= i + 1u + tlv[i])
              {
                  ByteVector msgdata(tlv.begin() + i + 1, tlv.begin() + i + 1 + tlv[i]);
                  std::shared_ptr<NdefMessage> ndef = std::make_shared<NdefMessage>(msgdata);
                  messageList.push_back(ndef);
                  i += tlv[i] + 1;
              }
              break;
          case 0xFE: // Terminator
              i = static_cast<unsigned short>(tlv.size());
              break;
          default:;
          }
      }
     return messageList;
   }

   ByteVector NfcData::dataToTLV(std::shared_ptr<NfcData> data)
   {
     ByteVector ret = {};

     if (data->getType() == 0x01)
     {
       std::shared_ptr<LockControlTlv> lockControl = std::dynamic_pointer_cast<LockControlTlv>(data);
       ret = LockControlTlv::lockControlToTLV(lockControl);
     }
     else if (data->getType() == 0x02)
     {
       std::shared_ptr<MemoryControlTlv> memoryControl  = std::dynamic_pointer_cast<MemoryControlTlv>(data);
       ret = MemoryControlTlv::memoryControlToTLV(memoryControl);
     }
     else if (data->getType() == 0x03)
     {
       std::shared_ptr<NdefMessage> ndefMess = std::dynamic_pointer_cast<NdefMessage>(data);
       ret = NdefMessage::NdefMessageToTLV(ndefMess);
     }
     return ret;
   }

   std::string NfcData::getDefaultXmlNodeName() const
   {
       return "NfcData";
   }

   void NfcData::serialize(boost::property_tree::ptree &parentNode)
   {
     /* TO DO */
   }

   void NfcData::unSerialize(boost::property_tree::ptree &node)
   {
     /* TO DO */
   }
}
