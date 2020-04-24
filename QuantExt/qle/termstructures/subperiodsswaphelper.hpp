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

/*! \file subperiodsswaphelper.hpp
    \brief Single currency sub periods swap helper
    \ingroup termstructures
*/

#ifndef quantext_sub_period_swap_helper_hpp_
#define quantext_sub_period_swap_helper_hpp_

#include <ql/termstructures/yield/ratehelpers.hpp>

#include <qle/instruments/subperiodsswap.hpp>

//#include <boost/optional.hpp>

namespace QuantExt {
using namespace QuantLib;

//! Rate helper for bootstrapping using Sub Periods Swaps
/*! \ingroup termstructures
 */
class SubPeriodsSwapHelper : public RelativeDateRateHelper {
public:
    SubPeriodsSwapHelper(Handle<Quote> spread, const Period& swapTenor, const Period& fixedTenor,
                         const Calendar& fixedCalendar, const DayCounter& fixedDayCount,
                         BusinessDayConvention fixedConvention, const Period& floatPayTenor,
                         const boost::shared_ptr<IborIndex>& iborIndex, const DayCounter& floatDayCount,
                         const Handle<YieldTermStructure>& discountingCurve = Handle<YieldTermStructure>(),
                         SubPeriodsCoupon::Type type = SubPeriodsCoupon::Compounding);
    
    SubPeriodsSwapHelper(Handle<Quote> spread, const Period& swapTenor, 
                        // fixed leg
                         const Period& fixedTenor, const Calendar& fixedCalendar, const DayCounter& fixedDayCount,
                         BusinessDayConvention fixedConvention, BusinessDayConvention fixedTerminationConvention,
                         Natural fixedPaymentLag, boost::optional<DateGeneration::Rule> fixedRule,
                         // float leg
                         const Period& floatPayTenor, const Calendar& floatCalendar, const DayCounter& floatDayCount,
                         BusinessDayConvention floatConvention, BusinessDayConvention floatTerminationConvention,
                         const boost::shared_ptr<IborIndex>& iborIndex, Spread floatSpread, Natural floatPaymentLag,
                         boost::optional<DateGeneration::Rule> floatRule,
                         // sub-periods
                         const Calendar& subPeriodsCalendar, BusinessDayConvention subPeriodsConvention,
                         BusinessDayConvention subPeriodsTerminationConvention,
                         boost::optional<SubPeriodsCoupon::Type> type, boost::optional<bool> includeSpread,
                         boost::optional<DateGeneration::Rule> subPeriodsRule,
                         // discount curve
                         const Handle<YieldTermStructure>& discountingCurve,
                         boost::optional<Natural> settlementDays);

    //! \name RateHelper interface
    //@{
    Real impliedQuote() const;
    void setTermStructure(YieldTermStructure*);
    //@}
    //! \name SubPeriodsSwapHelper inspectors
    //@{
    boost::shared_ptr<SubPeriodsSwap> swap() const { return swap_; }
    //@}
    //! \name Visitability
    //@{
    void accept(AcyclicVisitor&);
    //@}

protected:
    void initializeDates();

private:
    boost::shared_ptr<SubPeriodsSwap> swap_;
    boost::shared_ptr<IborIndex> iborIndex_;
    Natural settlementDays_;
    Period swapTenor_;

    Period fixedTenor_;
    Calendar fixedCalendar_;
    DayCounter fixedDayCount_;
    BusinessDayConvention fixedConvention_;
    BusinessDayConvention fixedTerminationConvention_;
    Natural fixedPaymentLag_;
    DateGeneration::Rule fixedRule_;

    Period floatPayTenor_;
    Calendar floatCalendar_;
    DayCounter floatDayCount_;
    BusinessDayConvention floatConvention_;
    BusinessDayConvention floatTerminationConvention_;
    Spread floatSpread_;
    Natural floatPaymentLag_;
    DateGeneration::Rule floatRule_;

    Calendar subPeriodsCalendar_;
    BusinessDayConvention subPeriodsConvention_;
    BusinessDayConvention subPeriodsTerminationConvention_;
    SubPeriodsCoupon::Type type_;
    bool includeSpread_;
    DateGeneration::Rule subPeriodsRule_;

    RelinkableHandle<YieldTermStructure> termStructureHandle_;
    Handle<YieldTermStructure> discountHandle_;
    RelinkableHandle<YieldTermStructure> discountRelinkableHandle_;
};
} // namespace QuantExt

#endif
