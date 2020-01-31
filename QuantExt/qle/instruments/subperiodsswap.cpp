/*
 Copyright (C) 2017 Quaternion Risk Management Ltd
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

#include <ql/cashflows/fixedratecoupon.hpp>
#include <ql/time/schedule.hpp>
#include <qle/cashflows/subperiodscoupon.hpp>
#include <qle/instruments/subperiodsswap.hpp>

using namespace QuantLib;

namespace QuantExt {

SubPeriodsSwap::SubPeriodsSwap(const Date& effectiveDate, Real nominal, const Period& swapTenor, bool isPayer,
                               const Period& fixedTenor, Rate fixedRate, const Calendar& fixedCalendar,
                               const DayCounter& fixedDayCount, BusinessDayConvention fixedConvention,
                               const Period& floatPayTenor, const boost::shared_ptr<IborIndex>& iborIndex,
                               const DayCounter& floatingDayCount, DateGeneration::Rule rule,
                               SubPeriodsCoupon::Type type)

    : Swap(2), nominal_(nominal), isPayer_(isPayer), fixedRate_(fixedRate), fixedDayCount_(fixedDayCount),
      floatIndex_(iborIndex), floatSpread_(0.0), floatDayCount_(floatingDayCount), floatPayTenor_(floatPayTenor), type_(type),
      includeSpread_(false) {

    Date terminationDate = effectiveDate + swapTenor;

    // Fixed leg
    fixedSchedule_ = MakeSchedule()
                         .from(effectiveDate)
                         .to(terminationDate)
                         .withTenor(fixedTenor)
                         .withCalendar(fixedCalendar)
                         .withConvention(fixedConvention)
                         .withTerminationDateConvention(fixedConvention)
                         .withRule(rule);

    legs_[0] = FixedRateLeg(fixedSchedule_)
                   .withNotionals(nominal_)
                   .withCouponRates(fixedRate_, fixedDayCount_)
                   .withPaymentAdjustment(fixedConvention);

    // Sub Periods Leg, schedule is the PAY schedule
    BusinessDayConvention floatPmtConvention = iborIndex->businessDayConvention();
    Calendar floatPmtCalendar = iborIndex->fixingCalendar();
    floatSchedule_ = MakeSchedule()
                         .from(effectiveDate)
                         .to(terminationDate)
                         .withTenor(floatPayTenor)
                         .withCalendar(floatPmtCalendar)
                         .withConvention(floatPmtConvention)
                         .withTerminationDateConvention(floatPmtConvention)
                         .withRule(rule);

    legs_[1] = SubPeriodsLeg(floatSchedule_, floatIndex_)
                   .withNotional(nominal_)
                   .withSpread(floatSpread_)
                   .withPaymentAdjustment(floatPmtConvention)
                   .withPaymentDayCounter(floatDayCount_)
                   .withPaymentCalendar(floatPmtCalendar)
                   .includeSpread(includeSpread_)
                   .withType(type_);

    // legs_[0] is fixed
    payer_[0] = isPayer_ ? -1.0 : +1.0;
    payer_[1] = isPayer_ ? +1.0 : -1.0;

    // Register this instrument with its coupons
    Leg::const_iterator it;
    for (it = legs_[0].begin(); it != legs_[0].end(); ++it)
        registerWith(*it);
    for (it = legs_[1].begin(); it != legs_[1].end(); ++it)
        registerWith(*it);
}

SubPeriodsSwap::SubPeriodsSwap(const Date& effectiveDate, Real nominal, const Period& swapTenor, bool isPayer,
                               // fixed leg
                               const Period& fixedTenor, Rate fixedRate, const Calendar& fixedCalendar,
                               const DayCounter& fixedDayCount, BusinessDayConvention fixedConvention,
                               BusinessDayConvention fixedTerminationConvention,
                               Natural fixedPaymentLag, boost::optional<DateGeneration::Rule> fixedRule,
                               // float leg
                               const Period& floatPayTenor, const Calendar& floatCalendar,
                               const DayCounter& floatDayCount, BusinessDayConvention floatConvention,
                               BusinessDayConvention floatTerminationConvention,
                               const boost::shared_ptr<IborIndex>& iborIndex, Spread floatSpread,
                               Natural floatPaymentLag, boost::optional<DateGeneration::Rule> floatRule,
                               // Sub period
                               const Calendar& subPeriodsCalendar, BusinessDayConvention subPeriodsConvention,
                               BusinessDayConvention subPeriodsTerminationConvention,
                               boost::optional<SubPeriodsCoupon::Type> type, boost::optional<bool> includeSpread,
                               boost::optional<DateGeneration::Rule> subPeriodsRule 
                               )

    : Swap(2), nominal_(nominal), isPayer_(isPayer), fixedRate_(fixedRate), fixedDayCount_(fixedDayCount),
      floatIndex_(iborIndex), floatSpread_(floatSpread), floatDayCount_(floatDayCount), floatPayTenor_(floatPayTenor),
      type_(type == boost::none ? SubPeriodsCoupon::Type::Compounding : *type),
      includeSpread_(type == boost::none ? false : *includeSpread)
    {

    Date terminationDate = effectiveDate + swapTenor;

    // Fixed leg
    fixedSchedule_ = MakeSchedule()
        .from(effectiveDate)
        .to(terminationDate)
        .withTenor(fixedTenor)
        .withCalendar(fixedCalendar)
        .withConvention(fixedConvention)
        .withTerminationDateConvention(fixedTerminationConvention)
        .withRule(subPeriodsRule == boost::none ? DateGeneration::Rule::Backward : *subPeriodsRule);

    legs_[0] = FixedRateLeg(fixedSchedule_)
        .withNotionals(nominal_)
        .withCouponRates(fixedRate_, fixedDayCount_)
        .withPaymentAdjustment(fixedConvention)
        .withPaymentLag(fixedPaymentLag);

    // Sub Periods Leg, schedule is the PAY schedule
    BusinessDayConvention floatPmtConvention = floatConvention;
    Calendar floatPmtCalendar = floatCalendar;
    floatSchedule_ = MakeSchedule()
        .from(effectiveDate)
        .to(terminationDate)
        .withTenor(floatPayTenor)
        .withCalendar(floatPmtCalendar)
        .withConvention(floatPmtConvention)
        .withTerminationDateConvention(floatTerminationConvention)
        .withRule(floatRule == boost::none ? DateGeneration::Rule::Backward : *floatRule);

    legs_[1] = SubPeriodsLeg(floatSchedule_, floatIndex_)
        .withNotional(nominal_)
        .withSpread(floatSpread_)
        .withPaymentAdjustment(floatPmtConvention)
        .withPaymentDayCounter(floatDayCount_)
        .withPaymentCalendar(floatPmtCalendar)
        .withPaymentLag(floatPaymentLag)
        .withSubPeriodsAdjustment(subPeriodsConvention)
        .withSubPeriodsCalendar(subPeriodsCalendar)
        .includeSpread(includeSpread_)
        .withType(type_)
        .withSubPeriodsRule(subPeriodsRule == boost::none ? DateGeneration::Rule::Backward : *subPeriodsRule);

    // legs_[0] is fixed
    payer_[0] = isPayer_ ? -1.0 : +1.0;
    payer_[1] = isPayer_ ? +1.0 : -1.0;

    // Register this instrument with its coupons
    Leg::const_iterator it;
    for (it = legs_[0].begin(); it != legs_[0].end(); ++it)
        registerWith(*it);
    for (it = legs_[1].begin(); it != legs_[1].end(); ++it)
        registerWith(*it);
}

Real SubPeriodsSwap::fairRate() const {
    static const Spread basisPoint = 1.0e-4;
    calculate();
    QL_REQUIRE(legBPS_[0] != Null<Real>(), "result not available");
    return fixedRate_ - NPV_ / (legBPS_[0] / basisPoint);
}
} // namespace QuantExt
