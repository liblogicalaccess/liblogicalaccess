#pragma once

#include <logicalaccess/readerproviders/readerunit.hpp>

namespace logicalaccess
{

/**
 * A dummy reader unit whose whole point is to allow extension
 * from C#.
 *
 * Extending from C# is possible only against non abstract class (C# code
 * need to instantiate some C++ backing object) so this class simply provide
 * implementations for all pure virtual method declared by ReaderUnit.
 */
class LLA_CORE_API DummyReaderUnit : public ReaderUnit
{
  public:
    explicit DummyReaderUnit(std::string name) : ReaderUnit(name) {}

    bool waitInsertion(unsigned int maxwait) override
    {
        throw std::runtime_error("Not implemented");
    }

    bool waitRemoval(unsigned int maxwait) override
    {
        throw std::runtime_error("Not implemented");
    }

    bool isConnected() override
    {
        throw std::runtime_error("Not implemented");
    }

    void setCardType(std::string cardType) override
    {
        throw std::runtime_error("Not implemented");
    }

    std::shared_ptr<Chip> getSingleChip() override
    {
        throw std::runtime_error("Not implemented");
    }

    std::vector<std::shared_ptr<Chip>> getChipList() override
    {
        throw std::runtime_error("Not implemented");
    }

    bool connect() override
    {
        throw std::runtime_error("Not implemented");
    }

    void disconnect() override
    {
        throw std::runtime_error("Not implemented");
    }

    bool connectToReader() override
    {
        throw std::runtime_error("Not implemented");
    }

    void disconnectFromReader() override
    {
        throw std::runtime_error("Not implemented");
    }

    std::string getName() const override
    {
        throw std::runtime_error("Not implemented");
    }

    std::string getReaderSerialNumber() override
    {
        throw std::runtime_error("Not implemented");
    }

  protected:
    /**
     * Despite being non-pure virtual methods, createCardProbe() and
     * createDefaultResultChecker() have been overloaded to work around
     * an issue / misuse of SWIG.
     *
     * Indeed, SWIG code gen didn't work properly for us in the presence
     * of Director and non-public virtual method.
     * Similarly, adding final to prevent overloading does not work properly
     * with directors. Either generated C# or generated C++ failed to compile.
     *
     * However, a simple overload is enough for generated code to build.
     */

    std::shared_ptr<CardProbe> createCardProbe() override
    {
       return ReaderUnit::createCardProbe();
    }
    std::shared_ptr<ResultChecker> createDefaultResultChecker() const override
    {
        return ReaderUnit::createDefaultResultChecker();
    }
};
}
