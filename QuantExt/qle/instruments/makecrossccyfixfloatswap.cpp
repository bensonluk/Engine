#include <qle/instruments/makecrossccyfixfloatswap.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/time/schedule.hpp>


namespace QuantExt {

MakeCrossCcyFixFloatSwap::operator CrossCcyFixFloatSwap() const {
    boost::shared_ptr<CrossCcyFixFloatSwap> swap = *this;
    return *swap;
}

MakeCrossCcyFixFloatSwap::MakeCrossCcyFixFloatSwap(
    const Period& swapTenor,
    const boost::shared_ptr<IborIndex>& index,
    const Currency& fixedCurrency,
    Rate fixedRate,
    const Period& forwardStart
    )
    : swapTenor_(swapTenor), iborIndex_(index),
    fixedRate_(fixedRate), forwardStart_(forwardStart),
    fixedCurrency_(fixedCurrency),
    settlementDays_(iborIndex_->fixingDays()),
    fixedCalendar_(index->fixingCalendar()),
    floatCalendar_(index->fixingCalendar()),
    type_(CrossCcyFixFloatSwap::Payer), //nominal_(1.0),
    fixedNominal_(Null<Real>()), floatNominal_(Null<Real>()),
    spotFxQuote_(nullptr),
    floatTenor_(index->tenor()),
    fixedConvention_(ModifiedFollowing),
    fixedTerminationDateConvention_(ModifiedFollowing),
    fixedPaymentLag_(0),
    floatConvention_(index->businessDayConvention()),
    floatTerminationDateConvention_(index->businessDayConvention()),
    fixedRule_(DateGeneration::Backward), floatRule_(DateGeneration::Backward),
    fixedEndOfMonth_(false), floatEndOfMonth_(false),
    fixedFirstDate_(Date()), fixedNextToLastDate_(Date()),
    floatFirstDate_(Date()), floatNextToLastDate_(Date()),
    floatSpread_(0.0),
    floatDayCount_(index->dayCounter()),
    floatPaymentLag_(0) {}

MakeCrossCcyFixFloatSwap::operator boost::shared_ptr<CrossCcyFixFloatSwap>() const {

    Real fixedNominal, floatNominal;
    int numOfProvidedNominal = 0;
    if (fixedNominal_ != Null<Real>()) {
        fixedNominal = fixedNominal_;
        numOfProvidedNominal++;
    }
    if (floatNominal_ != Null<Real>()) {
        floatNominal = floatNominal_;
        numOfProvidedNominal++;
    }

    switch (numOfProvidedNominal) {
    case 2:
        break;
    case 1:
        if (spotFxQuote_ != nullptr) {
            Real spotFxQuote = spotFxQuote_->value();
            if (spotFxQuote != Null<Real>()) {
                if (fixedNominal_ != Null<Real>())
                    floatNominal = fixedNominal / spotFxQuote;
                else
                    fixedNominal = floatNominal * spotFxQuote;
                break;
            }
        }
        //[[fallthrough]];
    case 0:
        QL_FAIL("At least two of Fixed nominal, Floating nominal, and Spot FX Quote are required.");
        break;
    default:
        QL_FAIL("Unexpected error when resolving nominals");

    }


    Date startDate;
    if (effectiveDate_ != Date())
        startDate = effectiveDate_;
    else {
        Date refDate = Settings::instance().evaluationDate();
        // if the evaluation date is not a business day
        // then move to the next business day
        refDate = floatCalendar_.adjust(refDate);
        Date spotDate = floatCalendar_.advance(refDate,
            settlementDays_*Days);
        startDate = spotDate + forwardStart_;
        if (forwardStart_.length()<0)
            startDate = floatCalendar_.adjust(startDate,
                Preceding);
        else
            startDate = floatCalendar_.adjust(startDate,
                Following);
    }


    Date endDate = terminationDate_;
    if (endDate == Date()) {
        if (floatEndOfMonth_)
            endDate = floatCalendar_.advance(startDate,
                swapTenor_,
                ModifiedFollowing,
                floatEndOfMonth_);
        else
            endDate = startDate + swapTenor_;
    }

    const Currency& fixedCurrency = fixedCurrency_;
    const Currency& floatCurrency = iborIndex_->currency();
    Period fixedTenor = fixedTenor_ == Period() ? floatTenor_ : fixedTenor_;

    Schedule fixedSchedule(startDate, endDate,
        fixedTenor, fixedCalendar_,
        fixedConvention_,
        fixedTerminationDateConvention_,
        fixedRule_, fixedEndOfMonth_,
        fixedFirstDate_, fixedNextToLastDate_);

    Schedule floatSchedule(startDate, endDate,
        floatTenor_, floatCalendar_,
        floatConvention_,
        floatTerminationDateConvention_,
        floatRule_, floatEndOfMonth_,
        floatFirstDate_, floatNextToLastDate_);


    DayCounter fixedDayCount;
    if (fixedDayCount_ != DayCounter())
        fixedDayCount = fixedDayCount_;
    else
        QL_FAIL("unknown fixed leg day counter for " << fixedCurrency);

    Rate usedFixedRate = fixedRate_;
    if (fixedRate_ == Null<Rate>()) {
        CrossCcyFixFloatSwap temp(
            type_, fixedNominal, fixedCurrency, fixedSchedule,
            0.0, // fixed rate
            fixedDayCount, fixedConvention_, fixedPaymentLag_, fixedCalendar_,
            floatNominal, floatCurrency, floatSchedule, iborIndex_, floatSpread_,
            floatConvention_, floatPaymentLag_, floatCalendar_
            );

        if (engine_ == 0)
            QL_FAIL("At least one of fixed rate and swap engine has to be provided");

        temp.setPricingEngine(engine_);
        usedFixedRate = temp.fairFixedRate();
    }

    boost::shared_ptr<CrossCcyFixFloatSwap> swap = boost::make_shared<CrossCcyFixFloatSwap>(
        type_, fixedNominal, fixedCurrency, fixedSchedule,
        usedFixedRate,
        fixedDayCount, fixedConvention_, fixedPaymentLag_, fixedCalendar_,
        floatNominal, floatCurrency, floatSchedule, iborIndex_, floatSpread_,
        floatConvention_, floatPaymentLag_, floatCalendar_
        );

    if (engine_ != 0)
        swap->setPricingEngine(engine_);

    return swap;
    //cout << "End of converter" << endl;
}


MakeCrossCcyFixFloatSwap& MakeCrossCcyFixFloatSwap::receiveFixed(bool flag) {
    type_ = flag ? CrossCcyFixFloatSwap::Receiver : CrossCcyFixFloatSwap::Payer;
    return *this;
}

MakeCrossCcyFixFloatSwap& MakeCrossCcyFixFloatSwap::withType(CrossCcyFixFloatSwap::Type type) {
    type_ = type;
    return *this;
}

MakeCrossCcyFixFloatSwap& MakeCrossCcyFixFloatSwap::withFixedNominal(Real n) {
    fixedNominal_ = n;
    return *this;
}

MakeCrossCcyFixFloatSwap& MakeCrossCcyFixFloatSwap::withFloatingNominal(Real n) {
    floatNominal_ = n;
    return *this;
}

MakeCrossCcyFixFloatSwap& MakeCrossCcyFixFloatSwap::withSettlementDays(Natural settlementDays) {
    settlementDays_ = settlementDays;
    effectiveDate_ = Date();
    return *this;
}

MakeCrossCcyFixFloatSwap& MakeCrossCcyFixFloatSwap::withForwardStart(const Period& fs) {
    forwardStart_ = fs;
    return *this;
}

MakeCrossCcyFixFloatSwap&
MakeCrossCcyFixFloatSwap::withEffectiveDate(const Date& effectiveDate) {
    effectiveDate_ = effectiveDate;
    return *this;
}

MakeCrossCcyFixFloatSwap&
MakeCrossCcyFixFloatSwap::withTerminationDate(const Date& terminationDate) {
    terminationDate_ = terminationDate;
    swapTenor_ = Period();
    return *this;
}

MakeCrossCcyFixFloatSwap& MakeCrossCcyFixFloatSwap::withRule(DateGeneration::Rule r) {
    fixedRule_ = r;
    floatRule_ = r;
    return *this;
}

MakeCrossCcyFixFloatSwap& MakeCrossCcyFixFloatSwap::withSpotFx(boost::shared_ptr<Quote> fx) {
    spotFxQuote_ = fx;
    return *this;
}


/*
MakeCrossCcyFixFloatSwap& MakeCrossCcyFixFloatSwap::withDiscountingTermStructure(
const Handle<YieldTermStructure>& d) {
bool includeSettlementDateFlows = false;
engine_ = ext::shared_ptr<PricingEngine>(new
DiscountingSwapEngine(d, includeSettlementDateFlows));
return *this;
}
*/

MakeCrossCcyFixFloatSwap& MakeCrossCcyFixFloatSwap::withPricingEngine(
    const boost::shared_ptr<PricingEngine>& engine) {
    engine_ = engine;
    return *this;
}

MakeCrossCcyFixFloatSwap& MakeCrossCcyFixFloatSwap::withFixedLegTenor(const Period& t) {
    fixedTenor_ = t;
    return *this;
}


MakeCrossCcyFixFloatSwap&
MakeCrossCcyFixFloatSwap::withFixedLegCalendar(const Calendar& cal) {
    fixedCalendar_ = cal;
    return *this;
}


MakeCrossCcyFixFloatSwap&
MakeCrossCcyFixFloatSwap::withFixedLegConvention(BusinessDayConvention bdc) {
    fixedConvention_ = bdc;
    return *this;
}

MakeCrossCcyFixFloatSwap&
MakeCrossCcyFixFloatSwap::withFixedLegTerminationDateConvention(BusinessDayConvention bdc) {
    fixedTerminationDateConvention_ = bdc;
    return *this;
}

MakeCrossCcyFixFloatSwap& MakeCrossCcyFixFloatSwap::withFixedLegRule(DateGeneration::Rule r) {
    fixedRule_ = r;
    return *this;
}

MakeCrossCcyFixFloatSwap& MakeCrossCcyFixFloatSwap::withFixedLegEndOfMonth(bool flag) {
    fixedEndOfMonth_ = flag;
    return *this;
}

MakeCrossCcyFixFloatSwap& MakeCrossCcyFixFloatSwap::withFixedLegFirstDate(const Date& d) {
    fixedFirstDate_ = d;
    return *this;
}

MakeCrossCcyFixFloatSwap&
MakeCrossCcyFixFloatSwap::withFixedLegNextToLastDate(const Date& d) {
    fixedNextToLastDate_ = d;
    return *this;
}

MakeCrossCcyFixFloatSwap&
MakeCrossCcyFixFloatSwap::withFixedLegDayCount(const DayCounter& dc) {
    fixedDayCount_ = dc;
    return *this;
}

MakeCrossCcyFixFloatSwap&
MakeCrossCcyFixFloatSwap::withFixedPaymentLag(Natural lag) {
    fixedPaymentLag_ = lag;
    return *this;
}

MakeCrossCcyFixFloatSwap& MakeCrossCcyFixFloatSwap::withFloatingLegTenor(const Period& t) {
    floatTenor_ = t;
    return *this;
}

MakeCrossCcyFixFloatSwap&
MakeCrossCcyFixFloatSwap::withFloatingLegCalendar(const Calendar& cal) {
    floatCalendar_ = cal;
    return *this;
}

MakeCrossCcyFixFloatSwap&
MakeCrossCcyFixFloatSwap::withFloatingLegConvention(BusinessDayConvention bdc) {
    floatConvention_ = bdc;
    return *this;
}

MakeCrossCcyFixFloatSwap&
MakeCrossCcyFixFloatSwap::withFloatingLegTerminationDateConvention(BusinessDayConvention bdc) {
    floatTerminationDateConvention_ = bdc;
    return *this;
}

MakeCrossCcyFixFloatSwap& MakeCrossCcyFixFloatSwap::withFloatingLegRule(DateGeneration::Rule r) {
    floatRule_ = r;
    return *this;
}

MakeCrossCcyFixFloatSwap& MakeCrossCcyFixFloatSwap::withFloatingLegEndOfMonth(bool flag) {
    floatEndOfMonth_ = flag;
    return *this;
}

MakeCrossCcyFixFloatSwap&
MakeCrossCcyFixFloatSwap::withFloatingLegFirstDate(const Date& d) {
    floatFirstDate_ = d;
    return *this;
}

MakeCrossCcyFixFloatSwap&
MakeCrossCcyFixFloatSwap::withFloatingLegNextToLastDate(const Date& d) {
    floatNextToLastDate_ = d;
    return *this;
}

MakeCrossCcyFixFloatSwap&
MakeCrossCcyFixFloatSwap::withFloatingLegDayCount(const DayCounter& dc) {
    floatDayCount_ = dc;
    return *this;
}

MakeCrossCcyFixFloatSwap& MakeCrossCcyFixFloatSwap::withFloatingLegSpread(Spread sp) {
    floatSpread_ = sp;
    return *this;
}

MakeCrossCcyFixFloatSwap&
MakeCrossCcyFixFloatSwap::withFloatingPaymentLag(Natural lag) {
    floatPaymentLag_ = lag;
    return *this;
}

} // namespace QuantExt