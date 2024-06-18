#include <uspto/config.h>
#include <uspto/submission.h>

int main() {
    generateQueries(getValidationDataDirectory() / "neighbors_small.csv", 2500);
    return 0;
}
