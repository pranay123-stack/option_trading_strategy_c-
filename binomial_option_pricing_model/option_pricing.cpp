#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <random>
#include <numeric>

class BinomialOptionPricing {
public:
    // Enum to represent different types of options
    enum OptionType { Call, Put };
    // Enum to represent different exercise styles
    enum ExerciseStyle { European, American };

private:
    // Private member variables to store option parameters
    OptionType optionType;
    ExerciseStyle exerciseStyle;
    double strikePrice;
    double initialStockPrice;
    double riskFreeRate;
    double volatility;
    double timeToMaturity;
    double dividendYield;
    int steps;

public:
    // Constructor to initialize the option parameters
    BinomialOptionPricing(OptionType optType, ExerciseStyle exStyle, double strike, double stockPrice,
                          double rate, double vol, double maturity, double divYield, int numSteps)
        : optionType(optType), exerciseStyle(exStyle), strikePrice(strike), initialStockPrice(stockPrice),
          riskFreeRate(rate), volatility(vol), timeToMaturity(maturity), dividendYield(divYield), steps(numSteps) {}

    // Method to calculate the option price using the binomial tree method
    double price() {
        return computePriceUsingTree();
    }

 
    // Method to calculate the Greeks (delta, gamma, theta, vega, rho)
    void calculateGreeks(double& delta, double& gamma, double& theta, double& vega, double& rho) {
        double originalPrice = computePriceUsingTree();

        double S_up = initialStockPrice * 1.01;
        double S_down = initialStockPrice * 0.99;
        double originalS = initialStockPrice;

        initialStockPrice = S_up;
        double price_up = computePriceUsingTree();
        initialStockPrice = S_down;
        double price_down = computePriceUsingTree();

        delta = (price_up - price_down) / (S_up - S_down); // Delta calculation
        gamma = (price_up - 2 * originalPrice + price_down) / (0.01 * originalS * 0.01 * originalS); // Gamma calculation

        initialStockPrice = originalS;

        double originalT = timeToMaturity;
        timeToMaturity -= 1.0 / 365.0;
        double price_theta = computePriceUsingTree();
        theta = (price_theta - originalPrice) / (-1.0 / 365.0); // Theta calculation

        timeToMaturity = originalT;

        double originalV = volatility;
        volatility += 0.01;
        double price_vega = computePriceUsingTree();
        vega = (price_vega - originalPrice) / 0.01; // Vega calculation

        volatility = originalV;

        double originalR = riskFreeRate;
        riskFreeRate += 0.01;
        double price_rho = computePriceUsingTree();
        rho = (price_rho - originalPrice) / 0.01; // Rho calculation

        riskFreeRate = originalR;
    }

    // Method to compute the implied volatility for a given market price
    double computeImpliedVolatility(double marketPrice) {
        double low = 0.001;
        double high = 5.0;
        double epsilon = 1e-5;
        double impliedVol = 0.0;

        while (high - low > epsilon) {
            impliedVol = (low + high) / 2.0;
            volatility = impliedVol;
            double price = computePriceUsingTree();

            if (std::fabs(price - marketPrice) < epsilon) {
                break;
            } else if (price > marketPrice) {
                high = impliedVol;
            } else {
                low = impliedVol;
            }
        }
        return impliedVol;
    }

    // Method to compute the implied interest rate for a given market price
    double computeImpliedInterestRate(double marketPrice) {
        double low = -0.1;
        double high = 0.1;
        double epsilon = 1e-5;
        double impliedRate = 0.0;

        while (high - low > epsilon) {
            impliedRate = (low + high) / 2.0;
            riskFreeRate = impliedRate;
            double price = computePriceUsingTree();

            if (std::fabs(price - marketPrice) < epsilon) {
                break;
            } else if (price > marketPrice) {
                high = impliedRate;
            } else {
                low = impliedRate;
            }
        }
        return impliedRate;
    }

    // Method to calculate the payoff of a call option
    double callOptionValue(double stockPrice) {
        return std::max(0.0, stockPrice - strikePrice);
    }

    // Method to calculate the payoff of a put option
    double putOptionValue(double stockPrice) {
        return std::max(0.0, strikePrice - stockPrice);
    }

private:
    // Method to calculate the option payoff based on the option type
    double optionPayoff(double stockPrice) {
        switch (optionType) {
            case Call:
                return callOptionValue(stockPrice);
            case Put:
                return putOptionValue(stockPrice);
            default:
                return 0.0;
        }
    }

    // Method to compute the option price using the binomial tree method
    double computePriceUsingTree() {
        double dt = timeToMaturity / steps;
        std::vector<double> prices(steps + 1);
        for (int i = 0; i <= steps; i++) {
            double stockPriceAtNode = initialStockPrice * std::pow(exp(volatility * std::sqrt(dt)), steps - i) * std::pow(exp(-volatility * std::sqrt(dt)), i);
            prices[i] = optionPayoff(stockPriceAtNode);
        }

        for (int step = steps - 1; step >= 0; step--) {
            double rate = riskFreeRate;
            double u = exp(volatility * sqrt(dt));
            double d = 1 / u;
            double q = (exp(rate * dt) - d) / (u - d);

            std::vector<double> prices_next(step + 1);
            for (int i = 0; i <= step; i++) {
                double optionValue = (q * prices[i] + (1 - q) * prices[i + 1]) * exp(-rate * dt);
                if (exerciseStyle == American) {
                    double stockPriceAtNode = initialStockPrice * std::pow(exp(volatility * std::sqrt(dt)), step - i) * std::pow(exp(-volatility * std::sqrt(dt)), i);
                    optionValue = std::max(optionValue, optionPayoff(stockPriceAtNode));
                }
                prices_next[i] = optionValue;
            }
            prices = prices_next;
        }

        return prices[0];
    }
};

int main() {
    try {
        // Initialize option parameters
        double strikePrice = 100, stockPrice = 100, rate = 0.05, volatility = 0.2, maturity = 1, dividendYield = 0.02;
        int numSteps = 100;

        // Create BinomialOptionPricing model object
        BinomialOptionPricing model(BinomialOptionPricing::Call, BinomialOptionPricing::American,
                                    strikePrice, stockPrice, rate, volatility, maturity, dividendYield, numSteps);

        // Calculate and print the option price
        std::cout << "Option Price: " << model.price() << std::endl;

        // Calculate and print the implied volatility for a given market price
        double marketPrice = 10.5;
        double impliedVol = model.computeImpliedVolatility(marketPrice);
        std::cout << "Implied Volatility for market price " << marketPrice << ": " << impliedVol << std::endl;

        // Calculate and print the implied interest rate for a given market price
        double impliedRate = model.computeImpliedInterestRate(marketPrice);
        std::cout << "Implied Interest Rate for market price " << marketPrice << ": " << impliedRate << std::endl;

        // Calculate and print the Greeks
        double delta, gamma, theta, vega, rho;
        model.calculateGreeks(delta, gamma, theta, vega, rho);
        std::cout << "Delta: " << delta << ", Gamma: " << gamma << ", Theta: " << theta
                  << ", Vega: " << vega << ", Rho: " << rho << std::endl;
    } catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}



/*

Output Explanation on Input
The input parameters for the option are as follows:

Strike Price: 100
Stock Price: 100
Risk-Free Rate: 0.05 (5%)
Volatility: 0.2 (20%)
Time to Maturity: 1 year
Dividend Yield: 0.02 (2%)
Number of Steps: 100
Option Type: Call
Exercise Style: American
Output
Option Price: 10.4306

This is the price of the American call option calculated using the binomial tree method. It represents the premium that needs to be paid to purchase the option.
Implied Volatility for Market Price 10.5: 0.201851

This is the volatility implied by the market price of the option, which is 10.5. The implied volatility is calculated using the bisection method to find the volatility that makes the model price equal to the market price.
Implied Interest Rate for Market Price 10.5: 0.0500061

This is the interest rate implied by the market price of the option, which is 10.5. The implied interest rate is calculated using the bisection method to find the rate that makes the model price equal to the market price.
Delta: 0.636011

Delta measures the sensitivity of the option price to changes in the stock price. A delta of 0.636011 means that for a $1 increase in the stock price, the option price will increase by approximately $0.636.
Gamma: 0.074895

Gamma measures the sensitivity of delta to changes in the stock price. A gamma of 0.074895 means that for a $1 increase in the stock price, the delta will increase by approximately 0.074895.
Theta: 6.43768

Theta measures the sensitivity of the option price to the passage of time. A theta of 6.43768 means that the option price decreases by approximately $6.43768 per day as the option approaches maturity. (Note: The positive value indicates that the model might use annualized theta; typically, theta is negative, indicating decay.)
Vega: 37.4922

Vega measures the sensitivity of the option price to changes in volatility. A vega of 37.4922 means that for a 1% increase in volatility, the option price will increase by approximately $37.4922.
Rho: 53.7559

Rho measures the sensitivity of the option price to changes in the risk-free interest rate. A rho of 53.7559 means that for a 1% increase in the interest rate, the option price will increase by approximately $53.7559.
How These Outputs Are Calculated
Option Price
The option price is calculated using a binomial tree model. The steps involved include:

Initial Setup: Calculate the stock prices at maturity for all possible paths.
Backward Induction: Calculate the option values at each node starting from the last step and working backward to the present.
Implied Volatility
The implied volatility is calculated using the bisection method:

Set Bounds: Start with initial bounds for volatility.
Iterate: Adjust the bounds based on the difference between the model price and the market price until the desired precision is achieved.
Implied Interest Rate
The implied interest rate is calculated similarly to implied volatility:

Set Bounds: Start with initial bounds for the interest rate.
Iterate: Adjust the bounds based on the difference between the model price and the market price until the desired precision is achieved.
Greeks
Delta: Calculated by finite differences, adjusting the stock price slightly up and down and observing the change in the option price.
Gamma: Calculated by observing the change in delta as the stock price changes.
Theta: Calculated by observing the change in the option price as time to maturity decreases.
Vega: Calculated by observing the change in the option price as volatility changes.




*/