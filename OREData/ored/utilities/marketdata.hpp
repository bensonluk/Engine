/*
 Copyright (C) 2020 Quaternion Risk Management Ltd
 All rights reserved.

 This file is part of ORE, a free-software/open-source library
 for transparent pricing and risk analysis - http://opensourcerisk.org

 ORE is free software: you can redistribute it and/or modify it
 under the terms of the Modified BSD License.  You should have received a
 copy of the license along with this program.
 The license is also available online at <http://opensourcerisk.org>

 This program is distributed on the basis that it will form a useful
 contribution to risk analytics and model standardisation, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
*/

/*! \file ored/utilities/marketdata.hpp
    \brief market data related utilties
    \ingroup utilities
*/

#pragma once

#include <ored/marketdata/market.hpp>
#include <string>

namespace ore {
namespace data {

extern const std::string xccyCurveNamePrefix;

/*! For a given currency code, \p ccyCode, return the internal name for the cross currency based yield curve.

    For a given currency code, \p ccyCode, this function returns `__XCCY__-ccyCode`. This curve, if available, is
    currently used in special cases to allow for separate discount curves when discounting the cashflows on cross
    currency interest rate swaps.
*/
std::string xccyCurveName(const std::string& ccyCode);

/*! Attempt to return a yield curve from the market using the name generated by `xccyCurveName(ccyCode)`. If no yield
    curve is available, return the discount curve for the given currency code, \p ccyCode.
*/
QuantLib::Handle<QuantLib::YieldTermStructure>
xccyYieldCurve(const boost::shared_ptr<Market>& market, const std::string& ccyCode,
               const std::string& configuration = Market::defaultConfiguration);

/*! Attempt to return a yield curve from the market using the name generated by `xccyCurveName(ccyCode)`. If no yield
    curve is available, return the discount curve for the given currency code, \p ccyCode. The parameter
    \p outXccyExists is populated with `true` if a yield curve was found under `xccyCurveName(ccyCode)` and it is
    populated with `false` if there was no such yield curve.
*/
QuantLib::Handle<QuantLib::YieldTermStructure>
xccyYieldCurve(const boost::shared_ptr<Market>& market, const std::string& ccyCode, bool& outXccyExists,
               const std::string& configuration = Market::defaultConfiguration);

} // namespace data
} // namespace ore
