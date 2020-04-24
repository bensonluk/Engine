#ifndef quantext_makesubperiodsswap_hpp
#define quantext_makesubperiodsswap_hpp

#include <ql/time/dategenerationrule.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <qle/instruments/subperiodsswap.hpp>

namespace QuantExt {

    //! helper class
    /*! This class provides a more comfortable way
    to instantiate sub period swap
    */
    class MakeSubPeriodsSwap {
    public:
        MakeSubPeriodsSwap(
            const QuantLib::Period& swapTenor,
            const boost::shared_ptr<QuantLib::IborIndex>& iborIndex,
            QuantLib::Rate fixedRate = QuantLib::Null<QuantLib::Rate>(),
            const QuantLib::Period& forwardStart = 0 * QuantLib::Days);

        operator SubPeriodsSwap() const;
        operator boost::shared_ptr<SubPeriodsSwap>() const;

        MakeSubPeriodsSwap& receiveFixed(bool flag = true);
        MakeSubPeriodsSwap& withNominal(QuantLib::Real n);
        MakeSubPeriodsSwap& withForwardStart(const QuantLib::Period& fs);

        MakeSubPeriodsSwap& withSettlementDays(QuantLib::Natural settlementDays);
        MakeSubPeriodsSwap& withEffectiveDate(const QuantLib::Date&);
        MakeSubPeriodsSwap& withTerminationDate(const QuantLib::Date&);
        MakeSubPeriodsSwap& withRule(QuantLib::DateGeneration::Rule r);

        MakeSubPeriodsSwap& withFixedLegTenor(const QuantLib::Period& t);
        MakeSubPeriodsSwap& withFixedLegCalendar(const QuantLib::Calendar& cal);
        MakeSubPeriodsSwap& withFixedLegConvention(QuantLib::BusinessDayConvention bdc);
        MakeSubPeriodsSwap& withFixedLegTerminationDateConvention(
            QuantLib::BusinessDayConvention bdc);
        MakeSubPeriodsSwap& withFixedLegRule(QuantLib::DateGeneration::Rule r);
        MakeSubPeriodsSwap& withFixedLegFirstDate(const QuantLib::Date& d);
        MakeSubPeriodsSwap& withFixedLegNextToLastDate(const QuantLib::Date& d);
        MakeSubPeriodsSwap& withFixedLegDayCount(const QuantLib::DayCounter& dc);
        MakeSubPeriodsSwap& withFixedLegPaymentLag(QuantLib::Natural lag);

        MakeSubPeriodsSwap& withFloatingLegTenor(const QuantLib::Period& t);
        MakeSubPeriodsSwap& withFloatingLegCalendar(const QuantLib::Calendar& cal);
        MakeSubPeriodsSwap& withFloatingLegConvention(QuantLib::BusinessDayConvention bdc);
        MakeSubPeriodsSwap& withFloatingLegTerminationDateConvention(
            QuantLib::BusinessDayConvention bdc);
        MakeSubPeriodsSwap& withFloatingLegRule(QuantLib::DateGeneration::Rule r);
        MakeSubPeriodsSwap& withFloatingLegFirstDate(const QuantLib::Date& d);
        MakeSubPeriodsSwap& withFloatingLegNextToLastDate(const QuantLib::Date& d);
        MakeSubPeriodsSwap& withFloatingLegDayCount(const QuantLib::DayCounter& dc);
        MakeSubPeriodsSwap& withFloatingLegSpread(QuantLib::Spread sp);
        MakeSubPeriodsSwap& withFloatingLegPaymentLag(QuantLib::Natural lag);

        MakeSubPeriodsSwap& withSubPeriodsCalendar(const QuantLib::Calendar& cal);
        MakeSubPeriodsSwap& withSubPeriodsConvention(QuantLib::BusinessDayConvention bdc);
        MakeSubPeriodsSwap& withSubPeriodsTerminationDateConvention(QuantLib::BusinessDayConvention bdc);
        MakeSubPeriodsSwap& withSubPeriodsRule(QuantLib::DateGeneration::Rule r);
        MakeSubPeriodsSwap& withSubPeriodsType(QuantExt::SubPeriodsCoupon::Type type);

        MakeSubPeriodsSwap& withDiscountingTermStructure(
                        const Handle<YieldTermStructure>& discountCurve);
        MakeSubPeriodsSwap& withPricingEngine(const boost::shared_ptr<QuantLib::PricingEngine>& engine);

    private:
        QuantLib::Period swapTenor_;
        boost::shared_ptr<QuantLib::IborIndex> iborIndex_;
        QuantLib::Rate fixedRate_;
        QuantLib::Period forwardStart_;

        QuantLib::Natural settlementDays_;
        QuantLib::Date effectiveDate_, terminationDate_;
        QuantLib::Calendar fixedCalendar_, floatCalendar_, subPeriodsCalendar_;

        bool receivedFixed_;
        QuantLib::Real nominal_;
        QuantLib::Period fixedTenor_, floatTenor_;
        QuantLib::BusinessDayConvention fixedConvention_, fixedTerminationDateConvention_;
        QuantLib::BusinessDayConvention floatConvention_, floatTerminationDateConvention_;
        QuantLib::BusinessDayConvention subPeriodsConvention_, subPeriodsTerminationDateConvention_;
        QuantLib::DateGeneration::Rule fixedRule_, floatRule_, subPeriodsRule_;
        QuantLib::Date fixedFirstDate_, fixedNextToLastDate_;
        QuantLib::Date floatFirstDate_, floatNextToLastDate_;
        QuantLib::Spread floatSpread_;
        QuantLib::DayCounter fixedDayCount_, floatDayCount_;
        QuantLib::Natural fixedPaymentLag_, floatPaymentLag_;
        SubPeriodsCoupon::Type subPeriodsCouponType_;
        bool includeSpread_;

        boost::shared_ptr<QuantLib::PricingEngine> engine_;

    };
} //namespace QuantExt


#endif