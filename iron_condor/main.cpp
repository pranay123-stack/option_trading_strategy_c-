#include "IronCondorStrategy.h"

int main() {
    double entryTime = 9.15;
    double squareOffTime = 15.345; // 15:20:59 in decimal hours

    double sellCallStrike = 215;
    double buyCallStrike = 220;
    double sellPutStrike = 210;
    double buyPutStrike = 205;

    double sellCallPremium = 7.63;
    double buyCallPremium = 5.35;
    double sellPutPremium = 7.20;
    double buyPutPremium = 5.52;

    IronCondorStrategy strategy(entryTime, squareOffTime, sellCallStrike, buyCallStrike, sellPutStrike, buyPutStrike,
                                sellCallPremium, buyCallPremium, sellPutPremium, buyPutPremium);

    // Fetch LTP (example value)
    double ltp = 215.0;
    strategy.fetchLTP(ltp);

    // Simulate order placement and store initial positions
    strategy.simulateOrderPlacement();

    // Run strategy from entry time to square-off time
    strategy.runStrategy();

    return 0;
}
