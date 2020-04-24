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

#include <ql/time/calendars/jointcalendar.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <qle/termstructures/subperiodsswaphelper.hpp>

namespace QuantExt {

namespace {
void no_deletion(YieldTermStructure*) {}
} // namespace

SubPeriodsSwapHelper::SubPeriodsSwapHelper(Handle<Quote> spread, const Period& swapTenor, const Period& fixedTenor,
                                           const Calendar& fixedCalendar, const DayCounter& fixedDayCount,
                                           BusinessDayConvention fixedConvention, const Period& floatPayTenor,
                                           const boost::shared_ptr<IborIndex>& iborIndex,
                                           const DayCounter& floatDayCount,
                                           const Handle<YieldTermStructure>& discountingCurve,
                                           SubPeriodsCoupon::Type type)
    : RelativeDateRateHelper(spread), settlementDays_(iborIndex->fixingDays()), iborIndex_(iborIndex), swapTenor_(swapTenor),
      fixedTenor_(fixedTenor),
      fixedCalendar_(fixedCalendar), fixedDayCount_(fixedDayCount), fixedConvention_(fixedConvention),
      fixedTerminationConvention_(fixedConvention), fixedPaymentLag_(0), fixedRule_(DateGeneration::Rule::Backward),
      floatPayTenor_(floatPayTenor), floatCalendar_(iborIndex->fixingCalendar()), floatDayCount_(floatDayCount), 
      floatConvention_(iborIndex->businessDayConvention()),
      floatTerminationConvention_(iborIndex->businessDayConvention()),
      floatSpread_(0.0), floatPaymentLag_(0), floatRule_(DateGeneration::Rule::Backward),
      subPeriodsCalendar_(iborIndex->fixingCalendar()), subPeriodsConvention_(iborIndex->businessDayConvention()),
      subPeriodsTerminationConvention_(iborIndex->businessDayConvention()),
      type_(type), includeSpread_(false), subPeriodsRule_(DateGeneration::Rule::Backward), discountHandle_(discountingCurve) {

    iborIndex_ = iborIndex_->clone(termStructureHandle_);
    iborIndex_->unregisterWith(termStructureHandle_);

    registerWith(iborIndex_);
    registerWith(spread);
    registerWith(discountHandle_);

    initializeDates();
}

SubPeriodsSwapHelper::SubPeriodsSwapHelper(Handle<Quote> spread, const Period& swapTenor,
                                           // fixed leg
                                           const Period& fixedTenor, const Calendar& fixedCalendar, 
                                           const DayCounter& fixedDayCount, BusinessDayConvention fixedConvention,
                                           BusinessDayConvention fixedTerminationConvention,
                                           Natural fixedPaymentLag, boost::optional<DateGeneration::Rule> fixedRule,
                                           // float leg
                                           const Period& floatPayTenor, const Calendar& floatCalendar,
                                           const DayCounter& floatDayCount, BusinessDayConvention floatConvention,
                                           BusinessDayConvention floatTerminationConvention,
                                           const boost::shared_ptr<IborIndex>& iborIndex, Spread floatSpread,
                                           Natural floatPaymentLag, boost::optional<DateGeneration::Rule> floatRule,
                                           // sub-periods
                                           const Calendar& subPeriodsCalendar, BusinessDayConvention subPeriodsConvention,
                                           BusinessDayConvention subPeriodsTerminationConvention,
                                           boost::optional<SubPeriodsCoupon::Type> type, boost::optional<bool> includeSpread,
                                           boost::optional<DateGeneration::Rule> subPeriodsRule,
                                           // discount curve
                                           const Handle<YieldTermStructure>& discountingCurve,
                                           boost::optional<Natural> settlementDays)
    : RelativeDateRateHelper(spread), 
      iborIndex_(iborIndex), swapTenor_(swapTenor), fixedTenor_(fixedTenor),
      settlementDays_(settlementDays == boost::none || *settlementDays == Null<Natural>() ? 0 : *settlementDays), 
      fixedCalendar_(fixedCalendar), fixedDayCount_(fixedDayCount), fixedConvention_(fixedConvention),
      fixedTerminationConvention_(fixedTerminationConvention), fixedPaymentLag_(fixedPaymentLag),
      fixedRule_(fixedRule == boost::none ? DateGeneration::Rule::Backward : * fixedRule),
      floatPayTenor_(floatPayTenor), floatCalendar_(floatCalendar), floatDayCount_(floatDayCount), 
      floatConvention_(floatConvention), floatTerminationConvention_(floatTerminationConvention),
      floatSpread_(floatSpread), floatPaymentLag_(floatPaymentLag), 
      floatRule_(floatRule == boost::none ? DateGeneration::Rule::Backward : *floatRule),
      subPeriodsCalendar_(subPeriodsCalendar), subPeriodsConvention_(subPeriodsConvention),
      subPeriodsTerminationConvention_(subPeriodsTerminationConvention),
      type_(type == boost::none ? SubPeriodsCoupon::Type::Compounding : *type),
      includeSpread_(includeSpread == boost::none ? false : *includeSpread),
      subPeriodsRule_(subPeriodsRule == boost::none ? DateGeneration::Rule::Backward : *subPeriodsRule),
      discountHandle_(discountingCurve) {

    iborIndex_ = iborIndex_->clone(termStructureHandle_);
    iborIndex_->unregisterWith(termStructureHandle_);

    registerWith(iborIndex_);
    registerWith(spread);
    registerWith(discountHandle_);

    initializeDates();
}


void SubPeriodsSwapHelper::initializeDates() {

    // build swap
    Date valuationDate = Settings::instance().evaluationDate();
    Calendar spotCalendar = JointCalendar(fixedCalendar_, floatCalendar_);
    Natural spotDays = settlementDays_;
    // move val date forward in case it is a holiday
    valuationDate = spotCalendar.adjust(valuationDate);
    Date effectiveDate = spotCalendar.advance(valuationDate, spotDays * Days);

    swap_ = boost::shared_ptr<SubPeriodsSwap>(new SubPeriodsSwap(

        effectiveDate, 1.0, swapTenor_, true,
        // fixed leg
        fixedTenor_, 0.0, fixedCalendar_, fixedDayCount_,fixedConvention_,
        fixedTerminationConvention_, fixedPaymentLag_, fixedRule_,
        // float leg
        floatPayTenor_, floatCalendar_, floatDayCount_, floatConvention_,
        floatTerminationConvention_, iborIndex_, floatSpread_, floatPaymentLag_, floatRule_,
        // Sub period
        subPeriodsCalendar_, subPeriodsConvention_, subPeriodsTerminationConvention_,
        type_, includeSpread_, subPeriodsRule_));

    boost::shared_ptr<PricingEngine> engine(new DiscountingSwapEngine(discountRelinkableHandle_));
    swap_->setPricingEngine(engine);

    // set earliest and latest
    earliestDate_ = swap_->startDate();
    latestDate_ = swap_->maturityDate();

    boost::shared_ptr<FloatingRateCoupon> lastFloating =
        boost::dynamic_pointer_cast<FloatingRateCoupon>(swap_->floatLeg().back());
#ifdef QL_USE_INDEXED_COUPON
    /* May need to adjust latestDate_ if you are projecting libor based
    on tenor length rather than from accrual date to accrual date. */
    Date fixingValueDate = iborIndex_->valueDate(lastFloating->fixingDate());
    Date endValueDate = iborIndex_->maturityDate(fixingValueDate);
    latestDate_ = std::max(latestDate_, endValueDate);
#else
    /* Subperiods coupons do not have a par approximation either... */
    if (boost::dynamic_pointer_cast<SubPeriodsCoupon>(lastFloating)) {
        Date fixingValueDate = iborIndex_->valueDate(lastFloating->fixingDate());
        Date endValueDate = iborIndex_->maturityDate(fixingValueDate);
        latestDate_ = std::max(latestDate_, endValueDate);
    }
#endif
}

void SubPeriodsSwapHelper::setTermStructure(YieldTermStructure* t) {

    bool observer = false;

    boost::shared_ptr<YieldTermStructure> temp(t, no_deletion);
    termStructureHandle_.linkTo(temp, observer);

    if (discountHandle_.empty())
        discountRelinkableHandle_.linkTo(temp, observer);
    else
        discountRelinkableHandle_.linkTo(*discountHandle_, observer);

    RelativeDateRateHelper::setTermStructure(t);
}

Real SubPeriodsSwapHelper::impliedQuote() const {
    QL_REQUIRE(termStructure_ != 0, "Termstructure not set");
    swap_->recalculate();
    return swap_->fairRate();
}

void SubPeriodsSwapHelper::accept(AcyclicVisitor& v) {
    Visitor<SubPeriodsSwapHelper>* v1 = dynamic_cast<Visitor<SubPeriodsSwapHelper>*>(&v);
    if (v1 != 0)
        v1->visit(*this);
    else
        RateHelper::accept(v);
}
} // namespace QuantExt
