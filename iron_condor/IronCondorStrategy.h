#ifndef IRONCONDORSTRATEGY_H
#define IRONCONDORSTRATEGY_H

#include <vector>
#include <iostream>
#include <map>
#include <random>
#include <iomanip>

class IronCondorStrategy {
public:
    IronCondorStrategy(double entryTime, double squareOffTime, double sellCallStrike, double buyCallStrike,
                       double sellPutStrike, double buyPutStrike, double sellCallPremium, double buyCallPremium,
                       double sellPutPremium, double buyPutPremium);

    void fetchLTP(double ltp);
    void simulateOrderPlacement();
    void trackBidAsk(double currentTime);
    void checkStopLoss(double currentTime);
    void runStrategy();

private:
    struct OrderResponse {
        double tradePrice;
    };

    double entryTime, squareOffTime;
    double sellCallStrike, buyCallStrike;
    double sellPutStrike, buyPutStrike;
    double sellCallPremium, buyCallPremium;
    double sellPutPremium, buyPutPremium;
    double ltp;
    double initialTradedPriceCallSpread, initialTradedPricePutSpread;
    double bidCallBuy, askCallSell, bidPutBuy, askPutSell;
    double totalInitialPremium;
    bool stopLossHit;
    double stopLossTime;

    std::map<std::string, double> positions;

    OrderResponse placeOrder(double strike, double premium);
    void updateBidAskPrices();
    void updateStopLoss();
    void calculateProfitLoss(double currentTime);
};

#endif
