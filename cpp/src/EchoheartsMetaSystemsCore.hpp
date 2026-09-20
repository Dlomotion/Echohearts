#pragma once

#include <algorithm>
#include <cstdint>
#include <random>
#include <string>
#include <unordered_set>
#include <vector>

namespace Echohearts::Meta {

using UnixSeconds = std::int64_t;

struct MarketRules {
    std::int32_t listingFeeBps{100};
    std::int32_t saleTaxBps{500};
    std::int32_t minPricePercent{50};
    std::int32_t maxPricePercent{200};
    std::int32_t maxQuantityPerListing{9999};
};

struct MarketListingRequest {
    std::string sellerId;
    std::string itemId;
    std::string itemCategory;
    std::string currencyId;
    std::int32_t quantity{0};
    std::int64_t unitPrice{0};
};

enum class MarketValidationCode : std::uint8_t {
    Ok,
    MissingIdentity,
    InvalidQuantity,
    InvalidPrice,
    RestrictedCategory,
    OutsidePriceBand
};

struct MarketValidationResult {
    MarketValidationCode code{MarketValidationCode::Ok};
    std::int64_t listingFee{0};
};

inline bool IsRestrictedMarketCategory(const std::string& category) {
    return category == "EcoKin" || category == "EcoKinEgg" || category == "SentientPartner";
}

inline MarketValidationResult ValidateListing(
    const MarketListingRequest& request,
    const MarketRules& rules,
    std::int64_t trustedReferenceUnitPrice)
{
    if (request.sellerId.empty() || request.itemId.empty() || request.currencyId.empty()) {
        return {MarketValidationCode::MissingIdentity, 0};
    }
    if (request.quantity <= 0 || request.quantity > rules.maxQuantityPerListing) {
        return {MarketValidationCode::InvalidQuantity, 0};
    }
    if (request.unitPrice <= 0 || trustedReferenceUnitPrice <= 0) {
        return {MarketValidationCode::InvalidPrice, 0};
    }
    if (IsRestrictedMarketCategory(request.itemCategory)) {
        return {MarketValidationCode::RestrictedCategory, 0};
    }

    const std::int64_t minAllowed = (trustedReferenceUnitPrice * rules.minPricePercent) / 100;
    const std::int64_t maxAllowed = (trustedReferenceUnitPrice * rules.maxPricePercent) / 100;
    if (request.unitPrice < minAllowed || request.unitPrice > maxAllowed) {
        return {MarketValidationCode::OutsidePriceBand, 0};
    }

    const std::int64_t gross = request.unitPrice * static_cast<std::int64_t>(request.quantity);
    const std::int64_t fee = (gross * rules.listingFeeBps) / 10000;
    return {MarketValidationCode::Ok, fee};
}

struct MarketPurchaseQuote {
    std::int64_t buyerCost{0};
    std::int64_t sellerPayout{0};
    std::int64_t economySink{0};
};

inline MarketPurchaseQuote QuotePurchase(std::int64_t unitPrice, std::int32_t quantity, const MarketRules& rules) {
    const std::int64_t gross = unitPrice * static_cast<std::int64_t>(quantity);
    const std::int64_t tax = (gross * rules.saleTaxBps) / 10000;
    return {gross, gross - tax, tax};
}

struct FurnitureDefinition {
    std::string furnitureId;
    std::int32_t widthCells{1};
    std::int32_t heightCells{1};
};

struct FurniturePlacement {
    std::string instanceId;
    std::string furnitureId;
    std::int32_t x{0};
    std::int32_t y{0};
    std::int32_t quarterTurns{0};
    std::int32_t widthCells{1};
    std::int32_t heightCells{1};
};

enum class PlacementCode : std::uint8_t { Ok, InvalidDefinition, OutOfBounds, Overlap, DuplicateInstance };

class HousingLayout {
public:
    HousingLayout(std::int32_t widthCells, std::int32_t heightCells)
        : width_(widthCells), height_(heightCells) {}

    PlacementCode TryPlace(const std::string& instanceId,
                           const FurnitureDefinition& definition,
                           std::int32_t x,
                           std::int32_t y,
                           std::int32_t quarterTurns)
    {
        if (instanceId.empty() || definition.furnitureId.empty() || definition.widthCells <= 0 || definition.heightCells <= 0) {
            return PlacementCode::InvalidDefinition;
        }
        if (std::any_of(placements_.begin(), placements_.end(), [&](const FurniturePlacement& p) { return p.instanceId == instanceId; })) {
            return PlacementCode::DuplicateInstance;
        }

        const bool rotated = (quarterTurns % 2) != 0;
        const std::int32_t w = rotated ? definition.heightCells : definition.widthCells;
        const std::int32_t h = rotated ? definition.widthCells : definition.heightCells;
        if (x < 0 || y < 0 || x + w > width_ || y + h > height_) {
            return PlacementCode::OutOfBounds;
        }

        FurniturePlacement candidate{instanceId, definition.furnitureId, x, y, quarterTurns, w, h};
        for (const auto& existing : placements_) {
            if (Overlaps(candidate, existing)) {
                return PlacementCode::Overlap;
            }
        }
        placements_.push_back(candidate);
        return PlacementCode::Ok;
    }

    bool Remove(const std::string& instanceId) {
        const auto oldSize = placements_.size();
        placements_.erase(std::remove_if(placements_.begin(), placements_.end(),
                                         [&](const FurniturePlacement& p) { return p.instanceId == instanceId; }),
                          placements_.end());
        return placements_.size() != oldSize;
    }

    const std::vector<FurniturePlacement>& Placements() const { return placements_; }

private:
    static bool Overlaps(const FurniturePlacement& a, const FurniturePlacement& b) {
        return a.x < b.x + b.widthCells && a.x + a.widthCells > b.x &&
               a.y < b.y + b.heightCells && a.y + a.heightCells > b.y;
    }

    std::int32_t width_{0};
    std::int32_t height_{0};
    std::vector<FurniturePlacement> placements_;
};

struct CrossRegionEvent {
    std::string eventId;
    std::vector<std::string> regionTags;
    UnixSeconds firstStartUtc{0};
    UnixSeconds durationSeconds{0};
    UnixSeconds recurrenceSeconds{0};
};

inline bool EventAppliesToRegion(const CrossRegionEvent& event, const std::string& regionTag) {
    return std::find(event.regionTags.begin(), event.regionTags.end(), regionTag) != event.regionTags.end();
}

inline bool IsEventActive(const CrossRegionEvent& event, UnixSeconds nowUtc) {
    if (event.durationSeconds <= 0 || nowUtc < event.firstStartUtc) return false;
    if (event.recurrenceSeconds <= 0) {
        return nowUtc < event.firstStartUtc + event.durationSeconds;
    }
    const UnixSeconds elapsed = nowUtc - event.firstStartUtc;
    const UnixSeconds phase = elapsed % event.recurrenceSeconds;
    return phase < event.durationSeconds;
}

inline UnixSeconds DailyResetIndex(UnixSeconds nowUtc, std::int32_t resetHourUtc) {
    constexpr UnixSeconds Day = 86400;
    const UnixSeconds offset = static_cast<UnixSeconds>(resetHourUtc) * 3600;
    return (nowUtc - offset) / Day;
}

struct DailyMissionDefinition {
    std::string missionId;
    std::string category;
    std::int32_t minimumChapter{0};
    bool requiresRareSpawn{false};
};

inline std::vector<DailyMissionDefinition> SelectDailyMissions(
    const std::vector<DailyMissionDefinition>& pool,
    std::size_t count,
    std::uint64_t accountSeed,
    UnixSeconds dayIndex,
    std::int32_t currentChapter)
{
    std::vector<DailyMissionDefinition> eligible;
    for (const auto& mission : pool) {
        if (mission.minimumChapter <= currentChapter && !mission.requiresRareSpawn) {
            eligible.push_back(mission);
        }
    }

    std::mt19937_64 rng(accountSeed ^ (static_cast<std::uint64_t>(dayIndex) * 0x9E3779B97F4A7C15ULL));
    std::shuffle(eligible.begin(), eligible.end(), rng);

    std::vector<DailyMissionDefinition> selected;
    std::unordered_set<std::string> usedCategories;
    for (const auto& mission : eligible) {
        if (selected.size() >= count) break;
        if (usedCategories.insert(mission.category).second) {
            selected.push_back(mission);
        }
    }
    if (selected.size() < count) {
        for (const auto& mission : eligible) {
            if (selected.size() >= count) break;
            const bool alreadySelected = std::any_of(selected.begin(), selected.end(), [&](const auto& s) { return s.missionId == mission.missionId; });
            if (!alreadySelected) selected.push_back(mission);
        }
    }
    return selected;
}

} // namespace Echohearts::Meta
