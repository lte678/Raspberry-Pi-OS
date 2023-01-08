#include <iostream>
#include <cstdlib>
#include <cmath>


void generate_primes(long limit) {
    long number_of_primes = 0;
    // Edge case
    if(limit >= 2) {
        number_of_primes++;
    }
    for(long n = 3; n <= limit; n += 2) {
        number_of_primes++;
        // Try numbers from 2 to sqrt(n) as factors
        for(long m = 3; m <= static_cast<long>(sqrt((double)n)) + 1; m++) {
            if(n % m == 0) {
                // m is a factor of n. n is not prime
                number_of_primes--;
                break;
            }
        }
        // If we exit without breaking, number_of_primes remains incremented

        // The number is always odd, this is a little hacky :D
        if(n % 25000 == 1) {
            std::cout << n << " numbers searched..." << std::endl;
        }
    }
    std::cout << number_of_primes << " primes found!" << std::endl;
}


int main(int argc, char **argv) {
    if(argc != 2) {
        std::cerr << "Incorrect number of arguments!" << std::endl;
        std::cerr << "Usage: primes [limit]     Finds primes up to the number 'limit'" << std::endl;
        return EXIT_FAILURE;
    }
    long limit = atol(argv[1]);
    std::cout << "Generating " << limit << " primes..." << std::endl;

    generate_primes(limit);

    return EXIT_SUCCESS;
}