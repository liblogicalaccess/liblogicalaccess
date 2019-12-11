#include <logicalaccess/readerproviders/dummyreaderunit.hpp>

namespace logicalaccess {

DummyReaderUnit::DummyReaderUnit(std::string name)
    : ReaderUnit(name) {}
bool DummyReaderUnit::waitInsertion(unsigned int maxwait)
{
    throw std::runtime_error("Not implemented");
}
bool DummyReaderUnit::waitRemoval(unsigned int maxwait)
{
    throw std::runtime_error("Not implemented");
}
bool DummyReaderUnit::isConnected()
{
    throw std::runtime_error("Not implemented");
}
void DummyReaderUnit::setCardType(std::string cardType)
{
    throw std::runtime_error("Not implemented");
}
std::shared_ptr<Chip> DummyReaderUnit::getSingleChip()
{
    throw std::runtime_error("Not implemented");
}
bool DummyReaderUnit::connect()
{
    throw std::runtime_error("Not implemented");
}
std::vector<std::shared_ptr<Chip>> DummyReaderUnit::getChipList()
{
    throw std::runtime_error("Not implemented");
}
void DummyReaderUnit::disconnect()
{
    throw std::runtime_error("Not implemented");
}
bool DummyReaderUnit::connectToReader()
{
    throw std::runtime_error("Not implemented");
}
void DummyReaderUnit::disconnectFromReader()
{
    throw std::runtime_error("Not implemented");
}
std::string DummyReaderUnit::getName() const
{
    throw std::runtime_error("Not implemented");
}
std::string DummyReaderUnit::getReaderSerialNumber()
{
    throw std::runtime_error("Not implemented");
}
std::shared_ptr<CardProbe> DummyReaderUnit::createCardProbe()
{
    return ReaderUnit::createCardProbe();
}
std::shared_ptr<ResultChecker> DummyReaderUnit::createDefaultResultChecker() const
{
    return ReaderUnit::createDefaultResultChecker();
}
}
