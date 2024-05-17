#include "IronCondorStrategy.h"
#include <chrono>
#include <thread>
#include <random>

IronCondorStrategy::IronCondorStrategy(double entryTime, double squareOffTime, double sellCallStrike, double buyCallStrike,
                                       double sellPutStrike, double buyPutStrike, double sellCallPremium, double buyCallPremium,
                                       double sellPutPremium, double buyPutPremium)
    : entryTime(entryTime), squareOffTime(squareOffTime), sellCallStrike(sellCallStrike), buyCallStrike(buyCallStrike),
      sellPutStrike(sellPutStrike), buyPutStrike(buyPutStrike), sellCallPremium(sellCallPremium), buyCallPremium(buyCallPremium),
      sellPutPremium(sellPutPremium), buyPutPremium(buyPutPremium), ltp(0), initialTradedPriceCallSpread(0), initialTradedPricePutSpread(0),
      bidCallBuy(0), askCallSell(0), bidPutBuy(0), askPutSell(0), totalInitialPremium(0), stopLossHit(false), stopLossTime(0) {}

void IronCondorStrategy::fetchLTP(double ltp) {
    this->ltp = ltp;
}

void IronCondorStrategy::simulateOrderPlacement() {
    OrderResponse sellCallOrder = placeOrder(sellCallStrike, sellCallPremium);
    OrderResponse buyCallOrder = placeOrder(buyCallStrike, buyCallPremium);
    OrderResponse sellPutOrder = placeOrder(sellPutStrike, sellPutPremium);
    OrderResponse buyPutOrder = placeOrder(buyPutStrike, buyPutPremium);

    positions["sellCall"] = sellCallOrder.tradePrice;
    positions["buyCall"] = buyCallOrder.tradePrice;
    positions["sellPut"] = sellPutOrder.tradePrice;
    positions["buyPut"] = buyPutOrder.tradePrice;

    initialTradedPriceCallSpread = positions["sellCall"] + positions["buyCall"];
    initialTradedPricePutSpread = positions["sellPut"] + positions["buyPut"];
    totalInitialPremium = initialTradedPriceCallSpread + initialTradedPricePutSpread;
}

IronCondorStrategy::OrderResponse IronCondorStrategy::placeOrder(double strike, double premium) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.95 * premium, 1.05 * premium);

    OrderResponse response;
    response.tradePrice = dis(gen);
    return response;
}

void IronCondorStrategy::trackBidAsk(double currentTime) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(1.0, 3.0);

    bidCallBuy = dis(gen);
    askCallSell = dis(gen);
    bidPutBuy = dis(gen);
    askPutSell = dis(gen);

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Time: " << currentTime << " | Bid/Ask Call Spread: " << bidCallBuy << "/" << askCallSell
              << " | Bid/Ask Put Spread: " << bidPutBuy << "/" << askPutSell << std::endl;
}

void IronCondorStrategy::checkStopLoss(double currentTime) {
    if (initialTradedPriceCallSpread > (bidCallBuy - askCallSell) || initialTradedPricePutSpread > (bidPutBuy - askPutSell)) {
        stopLossHit = true;
        stopLossTime = currentTime;
        std::cout << "Stop loss hit at time: " << currentTime << std::endl;
        calculateProfitLoss(currentTime);
    }
}

void IronCondorStrategy::calculateProfitLoss(double currentTime) {
    double callSpreadPL = (positions["sellCall"] - askCallSell) + (bidCallBuy - positions["buyCall"]);
    double putSpreadPL = (positions["sellPut"] - askPutSell) + (bidPutBuy - positions["buyPut"]);

    double totalPL = (callSpreadPL + putSpreadPL) * 15;

    std::cout << "Final Total Profit/Loss at time " << currentTime << ": " << totalPL << std::endl;
}

void IronCondorStrategy::runStrategy() {
    std::this_thread::sleep_for(std::chrono::seconds(1)); // Simulate initial wait time

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(entryTime + 1.0 / 60.0, squareOffTime - 1.0 / 60.0);
    double randomStopLossTime = dis(gen);

    for (double currentTime = entryTime + 1.0 / 60.0; currentTime <= squareOffTime; currentTime += 1.0 / 60.0) {
        trackBidAsk(currentTime);

        checkStopLoss(currentTime);

        if (stopLossHit) {
            return;
        }

        std::this_thread::sleep_for(std::chrono::seconds(1)); // Simulate 1-minute interval
    }

    if (!stopLossHit) {
        calculateProfitLoss(squareOffTime);
    }
}
