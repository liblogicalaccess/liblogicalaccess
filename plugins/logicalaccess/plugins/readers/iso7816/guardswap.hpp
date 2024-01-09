#pragma once

#include "logicalaccess/cards/readercardadapter.hpp"
#include <memory>

namespace logicalaccess
{

#ifndef SWIG
/**
 * General purpose RAII object to swap object attribute.
 */
template <typename Source, typename T, T (Source::*Getter)() const,
          void (Source::*Setter)(T)>
struct GuardSwap
{
    GuardSwap(Source &src, T new_item)
        : src_(src)
    {
        old_ = (src.*Getter)();
        (src.*Setter)(new_item);
    }

    ~GuardSwap()
    {
        (src_.*Setter)(old_);
    }

  private:
    T old_;
    Source &src_;
};

class ResultChecker;
/**
 * Object to swap a ResultChecker associated with a ReaderCardAdapter.
 *
 * The new ResultChecker becomes active during the construction of the
 * ResultCheckerSwapper object. The previous ResultChecker is restore
 * when ResultCheckerSwapper is destructed.
 */
template <typename ResultCheckerT>
struct ResultCheckerSwapper : GuardSwap<ReaderCardAdapter, std::shared_ptr<ResultChecker>,
                                        &ReaderCardAdapter::getResultChecker,
                                        &ReaderCardAdapter::setResultChecker>
{
    explicit ResultCheckerSwapper(ReaderCardAdapter &rca)
        : GuardSwap(rca, std::make_shared<ResultCheckerT>())
    {
    }
};

template <>
struct ResultCheckerSwapper<std::nullptr_t>
    : GuardSwap<ReaderCardAdapter, std::shared_ptr<ResultChecker>,
                &ReaderCardAdapter::getResultChecker,
                &ReaderCardAdapter::setResultChecker>
{
    explicit ResultCheckerSwapper(ReaderCardAdapter &rca)
        : GuardSwap(rca, nullptr)
    {
    }
};
#endif

}