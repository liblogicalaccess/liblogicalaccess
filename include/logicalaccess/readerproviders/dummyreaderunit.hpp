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
    explicit DummyReaderUnit(std::string name);

    bool waitInsertion(unsigned int maxwait) override;

    bool waitRemoval(unsigned int maxwait) override;

    bool isConnected() override;

    void setCardType(std::string cardType) override;

    std::shared_ptr<Chip> getSingleChip() override;

    std::vector<std::shared_ptr<Chip>> getChipList() override;

    bool connect() override;

    void disconnect() override;
    
    bool reconnect(int action) override;

    bool connectToReader() override;

    void disconnectFromReader() override;

    std::string getName() const override;

    std::string getReaderSerialNumber() override;

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

    std::shared_ptr<CardProbe> createCardProbe() override;
    std::shared_ptr<ResultChecker> createDefaultResultChecker() const override;
};
}
