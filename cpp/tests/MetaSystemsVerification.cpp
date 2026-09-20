#include "EchoheartsMetaSystemsCore.hpp"
#include <cassert>
#include <iostream>

using namespace Echohearts::Meta;

int main() {
    int passed = 0;

    {
        MarketRules rules;
        MarketListingRequest material{"player_a", "PurityResin", "Material", "TEST_CREDIT", 10, 120};
        auto ok = ValidateListing(material, rules, 100);
        assert(ok.code == MarketValidationCode::Ok);
        assert(ok.listingFee == 12);

        MarketListingRequest partner{"player_a", "EcoKin_001", "EcoKin", "TEST_CREDIT", 1, 100};
        assert(ValidateListing(partner, rules, 100).code == MarketValidationCode::RestrictedCategory);

        MarketListingRequest manipulated{"player_a", "PurityResin", "Material", "TEST_CREDIT", 1, 1000};
        assert(ValidateListing(manipulated, rules, 100).code == MarketValidationCode::OutsidePriceBand);

        auto quote = QuotePurchase(120, 10, rules);
        assert(quote.buyerCost == 1200);
        assert(quote.sellerPayout == 1140);
        assert(quote.economySink == 60);
        ++passed;
    }

    {
        HousingLayout layout(8, 8);
        FurnitureDefinition sofa{"Sofa_A", 3, 1};
        FurnitureDefinition table{"Table_A", 2, 2};
        assert(layout.TryPlace("sofa_1", sofa, 0, 0, 0) == PlacementCode::Ok);
        assert(layout.TryPlace("table_1", table, 1, 0, 0) == PlacementCode::Overlap);
        assert(layout.TryPlace("table_2", table, 3, 0, 1) == PlacementCode::Ok);
        assert(layout.TryPlace("sofa_2", sofa, 7, 7, 0) == PlacementCode::OutOfBounds);
        assert(layout.Remove("sofa_1"));
        ++passed;
    }

    {
        CrossRegionEvent event{"Stormfront", {"Vespera", "Skyforge"}, 1000, 300, 1000};
        assert(EventAppliesToRegion(event, "Vespera"));
        assert(EventAppliesToRegion(event, "Skyforge"));
        assert(!EventAppliesToRegion(event, "Paleon"));
        assert(IsEventActive(event, 1100));
        assert(!IsEventActive(event, 1500));
        assert(IsEventActive(event, 2100));
        ++passed;
    }

    {
        std::vector<DailyMissionDefinition> pool = {
            {"restore_patch", "Restoration", 0, false},
            {"care_partner", "Care", 0, false},
            {"explore_landmark", "Explore", 0, false},
            {"arena_round", "Arena", 1, false},
            {"rare_spawn_grind", "Explore", 0, true}
        };
        auto a = SelectDailyMissions(pool, 3, 42, 100, 3);
        auto b = SelectDailyMissions(pool, 3, 42, 100, 3);
        assert(a.size() == 3 && b.size() == 3);
        for (std::size_t i = 0; i < a.size(); ++i) assert(a[i].missionId == b[i].missionId);
        for (const auto& m : a) assert(!m.requiresRareSpawn);
        assert(a[0].category != a[1].category && a[0].category != a[2].category && a[1].category != a[2].category);
        ++passed;
    }

    std::cout << "Echohearts Meta Systems verification: " << passed << "/4 groups passed\n";
    return 0;
}
