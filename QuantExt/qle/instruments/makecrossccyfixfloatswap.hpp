#ifndef quantext_makecrossccyfixfloatswap_hpp
#define quantext_makecrossccyfixfloatswap_hpp

#include <ql/time/dategenerationrule.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <qle/instruments/crossccyfixfloatswap.hpp>

namespace QuantExt {

//! helper class
/*! This class provides a more comfortable way
    to instantiate cross currency fix-float swap
*/
class MakeCrossCcyFixFloatSwap {
public:
    MakeCrossCcyFixFloatSwap(
        const QuantLib::Period& swapTenor,
        const boost::shared_ptr<QuantLib::IborIndex>& iborIndex,
        const QuantLib::Currency& fixedCurrency,
        QuantLib::Rate fixedRate = QuantLib::Null<QuantLib::Rate>(),
        const QuantLib::Period& forwardStart = 0 * QuantLib::Days);

    operator CrossCcyFixFloatSwap() const;
    operator boost::shared_ptr<CrossCcyFixFloatSwap>() const;

    MakeCrossCcyFixFloatSwap& receiveFixed(bool flag = true);
    MakeCrossCcyFixFloatSwap& withType(CrossCcyFixFloatSwap::Type type);
    MakeCrossCcyFixFloatSwap& withForwardStart(const QuantLib::Period& forwardStart);

    MakeCrossCcyFixFloatSwap& withSettlementDays(QuantLib::Natural settlementDays);
    MakeCrossCcyFixFloatSwap& withEffectiveDate(const QuantLib::Date&);
    MakeCrossCcyFixFloatSwap& withTerminationDate(const QuantLib::Date&);
    MakeCrossCcyFixFloatSwap& withRule(QuantLib::DateGeneration::Rule r);
    MakeCrossCcyFixFloatSwap& withSpotFx(boost::shared_ptr<QuantLib::Quote> fx);

    MakeCrossCcyFixFloatSwap& withFixedNominal(QuantLib::Real n);
    MakeCrossCcyFixFloatSwap& withFixedLegTenor(const QuantLib::Period& t);
    MakeCrossCcyFixFloatSwap& withFixedLegCalendar(const QuantLib::Calendar& cal);
    MakeCrossCcyFixFloatSwap& withFixedLegConvention(QuantLib::BusinessDayConvention bdc);
    MakeCrossCcyFixFloatSwap& withFixedLegTerminationDateConvention(
        QuantLib::BusinessDayConvention bdc);
    MakeCrossCcyFixFloatSwap& withFixedLegRule(QuantLib::DateGeneration::Rule r);
    MakeCrossCcyFixFloatSwap& withFixedLegEndOfMonth(bool flag = true);
    MakeCrossCcyFixFloatSwap& withFixedLegFirstDate(const QuantLib::Date& d);
    MakeCrossCcyFixFloatSwap& withFixedLegNextToLastDate(const QuantLib::Date& d);
    MakeCrossCcyFixFloatSwap& withFixedLegDayCount(const QuantLib::DayCounter& dc);
    MakeCrossCcyFixFloatSwap& withFixedPaymentLag(QuantLib::Natural lag);

    MakeCrossCcyFixFloatSwap& withFloatingNominal(QuantLib::Real n);
    MakeCrossCcyFixFloatSwap& withFloatingLegTenor(const QuantLib::Period& t);
    MakeCrossCcyFixFloatSwap& withFloatingLegCalendar(const QuantLib::Calendar& cal);
    MakeCrossCcyFixFloatSwap& withFloatingLegConvention(QuantLib::BusinessDayConvention bdc);
    MakeCrossCcyFixFloatSwap& withFloatingLegTerminationDateConvention(
        QuantLib::BusinessDayConvention bdc);
    MakeCrossCcyFixFloatSwap& withFloatingLegRule(QuantLib::DateGeneration::Rule r);
    MakeCrossCcyFixFloatSwap& withFloatingLegEndOfMonth(bool flag = true);
    MakeCrossCcyFixFloatSwap& withFloatingLegFirstDate(const QuantLib::Date& d);
    MakeCrossCcyFixFloatSwap& withFloatingLegNextToLastDate(const QuantLib::Date& d);
    MakeCrossCcyFixFloatSwap& withFloatingLegDayCount(const QuantLib::DayCounter& dc);
    MakeCrossCcyFixFloatSwap& withFloatingLegSpread(QuantLib::Spread sp);
    MakeCrossCcyFixFloatSwap& withFloatingPaymentLag(QuantLib::Natural lag);

    MakeCrossCcyFixFloatSwap& withPricingEngine(const boost::shared_ptr<QuantLib::PricingEngine>& engine);

private:
    QuantLib::Period swapTenor_;
    boost::shared_ptr<QuantLib::IborIndex> iborIndex_;
    QuantLib::Rate fixedRate_;
    QuantLib::Currency fixedCurrency_;
    QuantLib::Period forwardStart_;
    boost::shared_ptr<QuantLib::Quote> spotFxQuote_;

    QuantLib::Natural settlementDays_;
    QuantLib::Date effectiveDate_, terminationDate_;
    QuantLib::Calendar fixedCalendar_, floatCalendar_;

    CrossCcyFixFloatSwap::Type type_;
    QuantLib::Real fixedNominal_, floatNominal_;
    QuantLib::Period fixedTenor_, floatTenor_;
    QuantLib::BusinessDayConvention fixedConvention_, fixedTerminationDateConvention_;
    QuantLib::BusinessDayConvention floatConvention_, floatTerminationDateConvention_;
    QuantLib::DateGeneration::Rule fixedRule_, floatRule_;
    bool fixedEndOfMonth_, floatEndOfMonth_;
    QuantLib::Date fixedFirstDate_, fixedNextToLastDate_;
    QuantLib::Date floatFirstDate_, floatNextToLastDate_;
    QuantLib::Spread floatSpread_;
    QuantLib::DayCounter fixedDayCount_, floatDayCount_;
    QuantLib::Natural fixedPaymentLag_, floatPaymentLag_;

    boost::shared_ptr<QuantLib::PricingEngine> engine_;

};
} //namespace QuantExt


#endif