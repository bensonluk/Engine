#include <qle/instruments/makesubperiodsswap.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/time/schedule.hpp>
#include <ql/time/calendars/jointcalendar.hpp>


namespace QuantExt {

    MakeSubPeriodsSwap::operator SubPeriodsSwap() const {
        boost::shared_ptr<SubPeriodsSwap> swap = *this;
        return *swap;
    }

    MakeSubPeriodsSwap::MakeSubPeriodsSwap(
        const Period& swapTenor,
        const boost::shared_ptr<IborIndex>& index,
        //const Currency& fixedCurrency,
        Rate fixedRate,
        const Period& forwardStart
        )
        : swapTenor_(swapTenor), iborIndex_(index),
        fixedRate_(fixedRate), forwardStart_(forwardStart),
        settlementDays_(iborIndex_->fixingDays()),
        fixedCalendar_(index->fixingCalendar()),
        floatCalendar_(index->fixingCalendar()),
        subPeriodsCalendar_(index->fixingCalendar()),
        nominal_(1.0),
        floatTenor_(index->tenor()),
        fixedConvention_(ModifiedFollowing),
        fixedTerminationDateConvention_(ModifiedFollowing),
        floatConvention_(index->businessDayConvention()),
        floatTerminationDateConvention_(index->businessDayConvention()),
        subPeriodsConvention_(index->businessDayConvention()),
        subPeriodsTerminationDateConvention_(index->businessDayConvention()),
        fixedRule_(DateGeneration::Backward), floatRule_(DateGeneration::Backward),
        subPeriodsRule_(DateGeneration::Backward),
        fixedFirstDate_(Date()), fixedNextToLastDate_(Date()),
        floatFirstDate_(Date()), floatNextToLastDate_(Date()),
        floatSpread_(0.0),
        floatDayCount_(index->dayCounter()),
        fixedPaymentLag_(0), floatPaymentLag_(0),
        subPeriodsCouponType_(SubPeriodsCoupon::Type::Compounding),
        includeSpread_(false)
        {}

    MakeSubPeriodsSwap::operator boost::shared_ptr<SubPeriodsSwap>() const {

        /*
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
        */

        Calendar jointcdr = JointCalendar(fixedCalendar_, floatCalendar_);

        Date startDate;
        if (effectiveDate_ != Date())
            startDate = effectiveDate_;
        else {
            Date refDate = Settings::instance().evaluationDate();
            // if the evaluation date is not a business day
            // then move to the next business day
            refDate = jointcdr.adjust(refDate);
            Date spotDate = jointcdr.advance(refDate, settlementDays_*Days);
            startDate = spotDate + forwardStart_;
            if (forwardStart_.length()<0)
                startDate = floatCalendar_.adjust(startDate, Preceding);
            else
                startDate = floatCalendar_.adjust(startDate, Following);
        }

        Date endDate = terminationDate_;
        if (endDate == Date()) {
            endDate = startDate + swapTenor_;
        }
       
        const Currency& curr = iborIndex_->currency();
        Period fixedTenor = fixedTenor_ == Period() ? floatTenor_ : fixedTenor_;
        QL_REQUIRE(fixedTenor_ != Period(), "unknown fixed leg default tenor for " << curr);

        DayCounter fixedDayCount;
        if (fixedDayCount_ != DayCounter())
            fixedDayCount = fixedDayCount_;
        else
            QL_FAIL("unknown fixed leg day counter for " << curr);

        Rate usedFixedRate = fixedRate_;
        if (fixedRate_ == Null<Rate>()) {
            SubPeriodsSwap temp = SubPeriodsSwap(startDate, 1.0, swapTenor_, !receivedFixed_,
                // fixed leg
                fixedTenor_, 0.0, fixedCalendar_, fixedDayCount_, fixedConvention_,
                fixedTerminationDateConvention_, fixedPaymentLag_, fixedRule_,
                // float leg
                floatTenor_, floatCalendar_, floatDayCount_, floatConvention_,
                floatTerminationDateConvention_, iborIndex_, floatSpread_, floatPaymentLag_, floatRule_,
                // Sub period
                subPeriodsCalendar_, subPeriodsConvention_,
                subPeriodsTerminationDateConvention_, subPeriodsCouponType_, includeSpread_,
                subPeriodsRule_
                );
             if (engine_ == 0) {
                Handle<YieldTermStructure> disc =
                                        iborIndex_->forwardingTermStructure();
                QL_REQUIRE(!disc.empty(),
                           "null term structure set to this instance of " <<
                           iborIndex_->name());
                bool includeSettlementDateFlows = false;
                ext::shared_ptr<PricingEngine> engine(new
                    DiscountingSwapEngine(disc, includeSettlementDateFlows));
                temp.setPricingEngine(engine);
            } else
                temp.setPricingEngine(engine_);

            usedFixedRate = temp.fairRate();
        }

        // the 4th argument of the constructor is true if pay fixed, reverse of convention here
        boost::shared_ptr<SubPeriodsSwap> swap = boost::make_shared<SubPeriodsSwap>(
            startDate, nominal_, swapTenor_, !receivedFixed_,
            // fixed leg
            fixedTenor_, usedFixedRate, fixedCalendar_, fixedDayCount_, fixedConvention_,
            fixedTerminationDateConvention_, fixedPaymentLag_, fixedRule_,
            // float leg
            floatTenor_, floatCalendar_, floatDayCount_, floatConvention_,
            floatTerminationDateConvention_, iborIndex_, floatSpread_, floatPaymentLag_, floatRule_,
            // Sub period
            subPeriodsCalendar_, subPeriodsConvention_,
            subPeriodsTerminationDateConvention_, subPeriodsCouponType_, includeSpread_,
            subPeriodsRule_
            );

        if (engine_ == 0) {
            Handle<YieldTermStructure> disc =
                                    iborIndex_->forwardingTermStructure();
            bool includeSettlementDateFlows = false;
            ext::shared_ptr<PricingEngine> engine(new
                DiscountingSwapEngine(disc, includeSettlementDateFlows));
            swap->setPricingEngine(engine);
        } else
            swap->setPricingEngine(engine_);

        return swap;
    }

    MakeSubPeriodsSwap& MakeSubPeriodsSwap::receiveFixed(bool flag) {
        receivedFixed_ = flag;
        return *this;
    }

    /*
    MakeVanillaSwap& MakeVanillaSwap::withType(VanillaSwap::Type type) {
        type_ = type;
        return *this;
    }
    */

    MakeSubPeriodsSwap& MakeSubPeriodsSwap::withNominal(Real n) {
        nominal_ = n;
        return *this;
    }

    MakeSubPeriodsSwap& MakeSubPeriodsSwap::withForwardStart(const Period& fs) {
        forwardStart_ = fs;
        return *this;
    }

    MakeSubPeriodsSwap& MakeSubPeriodsSwap::withSettlementDays(Natural settlementDays) {
        settlementDays_ = settlementDays;
        effectiveDate_ = Date();
        return *this;
    }

    MakeSubPeriodsSwap& MakeSubPeriodsSwap::withEffectiveDate(const Date& effectiveDate) {
        effectiveDate_ = effectiveDate;
        return *this;
    }

    MakeSubPeriodsSwap& MakeSubPeriodsSwap::withTerminationDate(const Date& terminationDate) {
        terminationDate_ = terminationDate;
        swapTenor_ = Period();
        return *this;
    }

    MakeSubPeriodsSwap& MakeSubPeriodsSwap::withRule(DateGeneration::Rule r) {
        fixedRule_ = r;
        floatRule_ = r;
        subPeriodsRule_ = r;
        return *this;
    }

    MakeSubPeriodsSwap& MakeSubPeriodsSwap::withDiscountingTermStructure(
                                        const Handle<YieldTermStructure>& d) {
        bool includeSettlementDateFlows = false;
        engine_ = ext::shared_ptr<PricingEngine>(new
            DiscountingSwapEngine(d, includeSettlementDateFlows));
        return *this;
    }

    MakeSubPeriodsSwap& MakeSubPeriodsSwap::withPricingEngine(
                             const ext::shared_ptr<PricingEngine>& engine) {
        engine_ = engine;
        return *this;
    }

    MakeSubPeriodsSwap& MakeSubPeriodsSwap::withFixedLegTenor(const Period& t) {
        fixedTenor_ = t;
        return *this;
    }

    MakeSubPeriodsSwap& MakeSubPeriodsSwap::withFixedLegCalendar(const Calendar& cal) {
        fixedCalendar_ = cal;
        return *this;
    }

    MakeSubPeriodsSwap& MakeSubPeriodsSwap::withFixedLegConvention(BusinessDayConvention bdc) {
        fixedConvention_ = bdc;
        return *this;
    }

    MakeSubPeriodsSwap&
    MakeSubPeriodsSwap::withFixedLegTerminationDateConvention(BusinessDayConvention bdc) {
        fixedTerminationDateConvention_ = bdc;
        return *this;
    }

    MakeSubPeriodsSwap& MakeSubPeriodsSwap::withFixedLegRule(DateGeneration::Rule r) {
        fixedRule_ = r;
        return *this;
    }

    MakeSubPeriodsSwap& MakeSubPeriodsSwap::withFixedLegFirstDate(const Date& d) {
        fixedFirstDate_ = d;
        return *this;
    }

    MakeSubPeriodsSwap& MakeSubPeriodsSwap::withFixedLegNextToLastDate(const Date& d) {
        fixedNextToLastDate_ = d;
        return *this;
    }

    MakeSubPeriodsSwap& MakeSubPeriodsSwap::withFixedLegDayCount(const DayCounter& dc) {
        fixedDayCount_ = dc;
        return *this;
    }

    MakeSubPeriodsSwap& MakeSubPeriodsSwap::withFixedLegPaymentLag(Natural lag) {
        fixedPaymentLag_ = lag;
        return *this;
    }

    MakeSubPeriodsSwap& MakeSubPeriodsSwap::withFloatingLegTenor(const Period& t) {
        floatTenor_ = t;
        return *this;
    }

    MakeSubPeriodsSwap& MakeSubPeriodsSwap::withFloatingLegCalendar(const Calendar& cal) {
        floatCalendar_ = cal;
        return *this;
    }

    MakeSubPeriodsSwap& MakeSubPeriodsSwap::withFloatingLegConvention(BusinessDayConvention bdc) {
        floatConvention_ = bdc;
        return *this;
    }

    MakeSubPeriodsSwap&
    MakeSubPeriodsSwap::withFloatingLegTerminationDateConvention(BusinessDayConvention bdc) {
        floatTerminationDateConvention_ = bdc;
        return *this;
    }

    MakeSubPeriodsSwap& MakeSubPeriodsSwap::withFloatingLegRule(DateGeneration::Rule r) {
        floatRule_ = r;
        return *this;
    }

    MakeSubPeriodsSwap& MakeSubPeriodsSwap::withFloatingLegFirstDate(const Date& d) {
        floatFirstDate_ = d;
        return *this;
    }

    MakeSubPeriodsSwap& MakeSubPeriodsSwap::withFloatingLegNextToLastDate(const Date& d) {
        floatNextToLastDate_ = d;
        return *this;
    }

    MakeSubPeriodsSwap& MakeSubPeriodsSwap::withFloatingLegDayCount(const DayCounter& dc) {
        floatDayCount_ = dc;
        return *this;
    }

    MakeSubPeriodsSwap& MakeSubPeriodsSwap::withFloatingLegSpread(Spread sp) {
        floatSpread_ = sp;
        return *this;
    }

    MakeSubPeriodsSwap& MakeSubPeriodsSwap::withFloatingLegPaymentLag(Natural lag) {
        floatPaymentLag_ = lag;
        return *this;
    }

    MakeSubPeriodsSwap& MakeSubPeriodsSwap::withSubPeriodsCalendar(const Calendar& cal) {
        subPeriodsCalendar_ = cal;
        return *this;
    }

    MakeSubPeriodsSwap& MakeSubPeriodsSwap::withSubPeriodsConvention(BusinessDayConvention bdc) {
        subPeriodsConvention_ = bdc;
        return *this;
    }

    MakeSubPeriodsSwap&
    MakeSubPeriodsSwap::withSubPeriodsTerminationDateConvention(BusinessDayConvention bdc) {
        subPeriodsTerminationDateConvention_ = bdc;
        return *this;
    }

    MakeSubPeriodsSwap& MakeSubPeriodsSwap::withSubPeriodsRule(DateGeneration::Rule r) {
        subPeriodsRule_ = r;
        return *this;
    }

    MakeSubPeriodsSwap& MakeSubPeriodsSwap::withSubPeriodsType(SubPeriodsCoupon::Type type) {
        subPeriodsCouponType_ = type;
        return *this;
    }

} // namespace QuantExt