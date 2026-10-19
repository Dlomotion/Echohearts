PRAGMA foreign_keys = ON;
PRAGMA user_version = 1;

CREATE TABLE players (
    player_id TEXT PRIMARY KEY,
    display_name TEXT NOT NULL,
    created_utc INTEGER NOT NULL
);

CREATE TABLE inventory (
    inventory_id INTEGER PRIMARY KEY,
    player_id TEXT NOT NULL REFERENCES players(player_id) ON DELETE CASCADE,
    item_id TEXT NOT NULL,
    quantity INTEGER NOT NULL CHECK(quantity >= 0),
    UNIQUE(player_id, item_id)
);

CREATE TABLE market_listings (
    listing_id TEXT PRIMARY KEY,
    seller_id TEXT NOT NULL REFERENCES players(player_id) ON DELETE CASCADE,
    item_id TEXT NOT NULL,
    item_category TEXT NOT NULL CHECK(item_category NOT IN ('EcoKin', 'EcoKinEgg', 'SentientPartner')),
    currency_id TEXT NOT NULL,
    quantity INTEGER NOT NULL CHECK(quantity > 0),
    unit_price INTEGER NOT NULL CHECK(unit_price > 0),
    created_utc INTEGER NOT NULL,
    expires_utc INTEGER NOT NULL CHECK(expires_utc > created_utc)
);

CREATE TABLE housing_placements (
    placement_id TEXT PRIMARY KEY,
    player_id TEXT NOT NULL REFERENCES players(player_id) ON DELETE CASCADE,
    furniture_id TEXT NOT NULL,
    grid_x INTEGER NOT NULL,
    grid_y INTEGER NOT NULL,
    quarter_turns INTEGER NOT NULL CHECK(quarter_turns BETWEEN 0 AND 3)
);

CREATE TABLE world_events (
    event_id TEXT PRIMARY KEY,
    first_start_utc INTEGER NOT NULL,
    duration_seconds INTEGER NOT NULL CHECK(duration_seconds > 0),
    recurrence_seconds INTEGER NOT NULL CHECK(recurrence_seconds >= 0)
);

CREATE TABLE world_event_regions (
    event_id TEXT NOT NULL REFERENCES world_events(event_id) ON DELETE CASCADE,
    region_tag TEXT NOT NULL,
    PRIMARY KEY(event_id, region_tag)
);

CREATE TABLE daily_mission_progress (
    player_id TEXT NOT NULL REFERENCES players(player_id) ON DELETE CASCADE,
    day_index INTEGER NOT NULL,
    mission_id TEXT NOT NULL,
    progress INTEGER NOT NULL DEFAULT 0 CHECK(progress >= 0),
    claimed INTEGER NOT NULL DEFAULT 0 CHECK(claimed IN (0,1)),
    PRIMARY KEY(player_id, day_index, mission_id)
);

CREATE INDEX idx_market_seller ON market_listings(seller_id);
CREATE INDEX idx_housing_player ON housing_placements(player_id);
CREATE INDEX idx_daily_player_day ON daily_mission_progress(player_id, day_index);
