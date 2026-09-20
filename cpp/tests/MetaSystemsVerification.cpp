#include "EchoheartsMetaSystemsCore.hpp"

#include <iostream>
#include <stdexcept>
#include <string>

using namespace Echohearts::Meta;

namespace {

void Require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

} // namespace

int main() {
    try {
        int passed = 0;

        {
            MarketRules rules;
            MarketListingRequest material{"player_a", "PurityResin", "Material", "TEST_CREDIT", 10, 120};
            const auto ok = ValidateListing(material, rules, 100);
            Require(ok.code == MarketValidationCode::Ok, "valid material listing rejected");
            Require(ok.listingFee == 12, "listing fee mismatch");

            MarketListingRequest partner{"player_a", "EcoKin_001", "EcoKin", "TEST_CREDIT", 1, 100};
            Require(ValidateListing(partner, rules, 100).code == MarketValidationCode::RestrictedCategory,
                    "Eco-Kin commerce guard failed");

            MarketListingRequest manipulated{"player_a", "PurityResin", "Material", "TEST_CREDIT", 1, 1000};
            Require(ValidateListing(manipulated, rules, 100).code == MarketValidationCode::OutsidePriceBand,
                    "price-band guard failed");

            const auto quote = QuotePurchase(120, 10, rules);
            Require(quote.buyerCost == 1200, "buyer cost mismatch");
            Require(quote.sellerPayout == 1140, "seller payout mismatch");
            Require(quote.economySink == 60, "economy sink mismatch");
            ++passed;
        }

        {
            HousingLayout layout(8, 8);
            FurnitureDefinition sofa{"Sofa_A", 3, 1};
            FurnitureDefinition table{"Table_A", 2, 2};
            Require(layout.TryPlace("sofa_1", sofa, 0, 0, 0) == PlacementCode::Ok,
                    "valid sofa placement rejected");
            Require(layout.TryPlace("table_1", table, 1, 0, 0) == PlacementCode::Overlap,
                    "housing overlap was not rejected");
            Require(layout.TryPlace("table_2", table, 3, 0, 1) == PlacementCode::Ok,
                    "rotated table placement rejected");
            Require(layout.TryPlace("sofa_2", sofa, 7, 7, 0) == PlacementCode::OutOfBounds,
                    "out-of-bounds housing placement was not rejected");
            Require(layout.Remove("sofa_1"), "housing removal failed");
            ++passed;
        }

        {
            CrossRegionEvent event{"Stormfront", {"Vespera", "Skyforge"}, 1000, 300, 1000};
            Require(EventAppliesToRegion(event, "Vespera"), "event missing Vespera region");
            Require(EventAppliesToRegion(event, "Skyforge"), "event missing Skyforge region");
            Require(!EventAppliesToRegion(event, "Paleon"), "event incorrectly applies to Paleon");
            Require(IsEventActive(event, 1100), "event should be active in first window");
            Require(!IsEventActive(event, 1500), "event should be inactive between windows");
            Require(IsEventActive(event, 2100), "event should be active in recurring window");
            ++passed;
        }

        {
            const std::vector<DailyMissionDefinition> pool = {
                {"restore_patch", "Restoration", 0, false},
                {"care_partner", "Care", 0, false},
                {"explore_landmark", "Explore", 0, false},
                {"arena_round", "Arena", 1, false},
                {"rare_spawn_grind", "Explore", 0, true}
            };
            const auto a = SelectDailyMissions(pool, 3, 42, 100, 3);
            const auto b = SelectDailyMissions(pool, 3, 42, 100, 3);
            Require(a.size() == 3 && b.size() == 3, "daily mission selection count mismatch");
            for (std::size_t i = 0; i < a.size(); ++i) {
                Require(a[i].missionId == b[i].missionId, "daily mission selection is not deterministic");
            }
            for (const auto& mission : a) {
                Require(!mission.requiresRareSpawn, "rare-spawn daily mission passed eligibility filter");
            }
            Require(a[0].category != a[1].category && a[0].category != a[2].category && a[1].category != a[2].category,
                    "daily mission category diversity failed");
            ++passed;
        }

        std::cout << "Echohearts Meta Systems verification: " << passed << "/4 groups passed\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "Echohearts Meta Systems verification FAILED: " << ex.what() << "\n";
        return 1;
    }
}
