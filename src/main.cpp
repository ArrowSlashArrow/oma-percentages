/**
 * Include the Geode headers.
 */
#include <Geode/Geode.hpp>
/**
 * Brings cocos2d and all Geode namespaces to the current scope.
 */
using namespace geode::prelude;

// -- Utils --
// All utils I got are from Cvolton's BetterInfo: https://github.com/Cvolton/betterinfo-geode/

gd::string decodeBase64Gzip(const gd::string& input) {
	return ZipUtils::decompressString(input, false, 0);
}

uint64_t timeInMs() {
	using namespace std::chrono;
	return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

inline bool objectIDIsSpeedPortal(int id) {
	return (id == 200 || id == 201 || id == 202 || id == 203 || id == 1334);
}

inline float travelForPortalId(int speed) {
	switch (speed)
	{
		case 200:
			return 251.16008f;
			break;
		default:
			return 311.58011f;
			break;
		case 202:
			return 387.42014f;
			break;
		case 203:
			return 468.00015f;
			break;
		case 1334:
			return 576.00018f;
			break;
	}
}

inline int speedToPortalId(int speed) {
	switch(speed) {
		default:
			return 201;
			break;
		case 1:
			return 200;
			break;
		case 2:
			return 202;
			break;
		case 3:
			return 203;
			break;
		case 4:
			return 1334;
			break;
	}
}

int stoi(std::string_view str) {
	int result = 0;
	std::from_chars(str.data(), str.data() + str.size(), result);
	return result;
}

float stof(std::string_view str) {
	return utils::numFromString<float>(str).unwrapOr(0);
}

float timeForLevelString(const gd::string& levelString) {
    struct SpeedPortalObject {
        int id;
        float xPos;
        bool checked;
    };

    auto a = timeInMs();

    auto decompressString = decodeBase64Gzip(levelString);
    auto c = timeInMs();
    std::string keyID;
    std::vector<SpeedPortalObject> speedPortals;

    float prevPortalX = 0;
    int prevPortalId = 0;

    float timeFull = 0;

    float maxPos = 0;
    for (auto currentObject : asp::iter::split(decompressString, ";")) {
        size_t i = 0;
        int objID = 0;
        float xPos = 0;
        bool checked = false;

        for(auto currentKey : asp::iter::split(currentObject, ",")) {
            if(i % 2 == 0) keyID = currentKey;
            else {
                if(keyID == "1") objID = stoi(currentKey);
                else if(keyID == "2") xPos = stof(currentKey);
                else if(keyID == "13") checked = stoi(currentKey);
                else if(keyID == "kA4") prevPortalId = speedToPortalId(stoi(currentKey));
            }
            i++;

            if(xPos != 0 && objID != 0 && checked == true) break;
        }

        if(maxPos < xPos) maxPos = xPos;
        if(!checked || !objectIDIsSpeedPortal(objID)) continue;

        speedPortals.push_back({objID, xPos, checked});
    }

    std::sort(speedPortals.begin(), speedPortals.end(), [](const SpeedPortalObject& a, const SpeedPortalObject& b) {
        return a.xPos < b.xPos;
    });

    for(const auto& portal : speedPortals) {
        //log::info("Object ID: {}, X Position: {}, Portal ID: {}", portal.id, portal.xPos, prevPortalId);
        timeFull += (portal.xPos - prevPortalX) / travelForPortalId(prevPortalId);
        prevPortalId = portal.id;
        prevPortalX = portal.xPos;
    }

    //log::info("Last portal ID: {}, Last X Position: {}", prevPortalId, prevPortalX);
    timeFull += (maxPos - prevPortalX) / travelForPortalId(prevPortalId);
    auto b = timeInMs() - a;
    log::debug("Time for levelString: {}ms, decompress: {}ms, parse: {}ms, maxPos {}", b, c - a, timeInMs() - c, maxPos);
    return timeFull;
}

// -- End of utils --

// ending percentage of each song
// i got these from bennoct's oma showcase by pressing ctrl + -> to go to the end of each song's chapter
// halfway art index: 17
// credits index: 31
float song_percentages[] = {
	1.49,	// stereo madness
	3.01,	// jumper
	4.82,	// clubstep
	12.52,	// every end
	14.39,	// deadlocked
	16.20,	// tokyo nights
	18.66,	// zone of thunder v2
	21.23,	// pagoda
	23.91,	// summit
	26.85,	// shiawase vip
	28.71,	// idolize
	31.06,	// sonic blaster
	33.36,	// slow down
	35.80,	// classical vip
	37.44,	// flow
	42.62,	// monolith
	46.53,	// wavelight
	46.80,	// halfway art
	49.56,	// phobos
	50.63,	// intercept 2
	52.40,	// eyes in the water
	55.64,	// dimension
	59.25,	// dark matter
	62.03,	// question mark
	65.61,	// isolation
	69.72,	// sphere
	73.02,	// we can dream
	77.98,	// 1077
	82.83,	// time machine
	89.89,	// flamewall
	94.22,	// at the speed of light
	95.39,	// credits
	100.0,	// magic touch
	0.0		// exists because of NOT_STARTPOS
};

std::string song_names[] = {
	"Stereo Madness",
	"Jumper",
	"Clubstep",
	"Every End",
	"Deadlocked",
	"Tokyo Nights",
	"Thunderzone v2",
	"Pagoda",
	"Summit",
	"Shiawase VIP",
	"Idolize",
	"Sonic Blaster",
	"Slow Down",
	"Classical VIP",
	"Flow",
	"Monolith",
	"Wavelight",
	"Halfway art",
	"Phobos",
	"Intercept 2",
	"Eyes in the Water",
	"Dimension",
	"Dark Matter",
	"Question mark",
	"Isolation",
	"Sphere",
	"We Can Dream",
	"1077",
	"Time Machine",
	"Flamewall",
	"At the Speed of Light",
	"Credits",
	"Magic Touch",
};


// arbitrary sentinel value
const size_t NOT_STARTPOS = 33;
// 30 (+ 1 padding) startposes: (pos_x, pos_y, index of percentage offset from song_percentages)
std::array<std::tuple<int, int, size_t>, 31> startpos_starts = {
	{
		{-1000000, -1000000, NOT_STARTPOS},	// stereo madness; set to -1m, -1m because there is not startpos for this song
		{31200, 99, 0},		// jumper
		{66400, 105, 1}, 		// clubstep
		{165, 2055, 2},		// every end
		{164415, 1695, 3},	// deadlocked
		{103, 3825, 4},		// tokyo nights
		{38835, 6375, 5},		// zone of thunder v2
		{98355, 6585, 6},		// pagoda
		{157005, 7875, 7},	// summit
		{75, 7155, 8},		// shiawase vip
		{76969, 7875, 9},		// idolize
		{119475, 8535, 10},	// sonic blaster
		{175753, 9735, 11},	// slow down
		{101, 8925, 12},		// classical vip
		{54525, 9765, 13},	// flow
		{87255, 9915, 14},	// monolith
		{73, 12555, 15},		// wavelight
		{84255, 12465, 17},	// phobos
		{140387, 14655, 18},	// intercept 2
		{168285, 14805, 19},	// eyes in the water
		{97, 15825, 20},		// dimension
		{70845, 16755, 21},	// dark matter
		{151565, 17755, 22},	// question mark
		{75, 18225, 23},		// isolation
		{76155, 19095, 24},	// sphere
		{165400, 19700, 25},	// we can dream
		{705, 20985, 26},		// 1077
		{111675, 22815, 27},	// time machine
		{105, 23145, 28},		// flamewall
		{75, 25965, 29},		// at the speed of light
		{112635, 25995, 31},	// magic touch
	}
};

float g_level_length = -1.0f;
bool playing_startpos = false;
size_t startpos_index = 0;

// it takes some time to enter the song, and this offset keeps track of that. used to adjust percentage.
float time_offset = 0.0f;

const int OMA_ID = 131249343;
const int OMA_STARTPOS_ID = 131249368;
// maybe we should include the startpos ID too but that's untested

// get level length from LevelInfoLayer
#include <Geode/modify/LevelInfoLayer.hpp>
class $modify(OMALevelInfo, LevelInfoLayer) {
	void updateLabelValues() {
		LevelInfoLayer::updateLabelValues();
		startpos_index = 0;
		playing_startpos = false;
		time_offset = 0.0f;
		if (m_level->m_levelID == OMA_STARTPOS_ID) {
			auto raw_time = m_level->m_timestamp / 240.0f;
			g_level_length = raw_time > 0 ? raw_time : m_level->isPlatformer() ? 0 : std::ceil(timeForLevelString(m_level->m_levelString));
			playing_startpos = true;
			log::info("playing oma startpos");
		} else if (m_level->m_levelID == OMA_ID) {
			auto raw_time = m_level->m_timestamp / 240.0f;
			g_level_length = raw_time > 0 ? raw_time : m_level->isPlatformer() ? 0 : std::ceil(timeForLevelString(m_level->m_levelString));
			log::info("got level length: {}", g_level_length);
		} else {
			log::info("not playing OMA, won't compute time");
		}
	}
};



/**
 * `$modify` lets you extend and modify GD's classes.
 * To hook a function in Geode, simply $modify the class
 * and write a new function definition with the signature of
 * the function you want to hook.
 *
 * Here we use the overloaded `$modify` macro to set our own class name,
 * so that we can use it for button callbacks.
 *
 * Notice the header being included, you *must* include the header for
 * the class you are modifying, or you will get a compile error.
 */
#include <Geode/modify/PlayLayer.hpp>
class $modify(OMAPercentage, PlayLayer) {
	// hook into the play layer to init this when the player is playing a level
	bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
		// check that we are in a level
		if (!PlayLayer::init(level, useReplay, dontCreateObjects)) {
			log::error("Couldn't init PlayLayer!");
			return false;
		}

		log::debug("in a level");

		if (this->check_in_OMA()) {
			// create label
			auto screen_size = CCDirector::sharedDirector()->getWinSize();
			auto label = CCLabelBMFont::create("", "bigFont.fnt");

			label->setID("oma_song"_spr);
			label->setScale(0.5f);
			label->setPosition({screen_size.width / 2.0f, screen_size.height - 20.0f});
			this->m_uiLayer->addChild(label);
		}

		return true;
	}

	// returns the index of the startpos that it detected the player to be at
	// NOT_STARTPOS if none detected
	int get_startpos_idx(float x, float y) {
		size_t idx = 1;
		while (idx < 31) {
			auto p = startpos_starts[idx];
			auto dx = std::get<0>(p) - x;
			auto dy = std::get<1>(p) - y;
			auto d = (dx * dx) + (dy * dy);
			if (d < 10000) {
				return std::get<2>(p);
			}

			idx += 1;
		}
		return NOT_STARTPOS;
	}

	void postUpdate(float dt) {
		PlayLayer::postUpdate(dt);

		if (!this->check_in_OMA()) {
			return;
		}

		auto pl = PlayLayer::get();
		auto label = geode::cast::typeinfo_cast<CCLabelBMFont*>(this->m_uiLayer->getChildByID("oma_song"_spr));
		if (!label) {
			return;
		}

		// old way
		// float percentage = pl->m_player1->getPositionX() / pl->m_levelLength * 100.0f;

		// get current player percentage
		float percentage = 100.0f * (pl->m_gameState.m_levelTime - time_offset) / g_level_length;
		if (playing_startpos && percentage < 0.5) {	// percentage < 0.5 prevents us from scanning a startpos that is later in the level
			// apply offset to percentage
			auto p1 = pl->m_player1;
			auto sp_idx = this->get_startpos_idx(p1->getPositionX(), p1->getPositionY());
			if (sp_idx != NOT_STARTPOS) {
				// update detected startpos index
				// log::info("got startpos index: {}", sp_idx);
				startpos_index = sp_idx;
				time_offset = pl->m_gameState.m_levelTime;
			}
		}
		// then, map startpos index to percent offset
		// since we aren't in the level for long before we hit a teleport this is necessary
		// to properly adjust the percentage
		percentage += song_percentages[startpos_index];
		// log::info("new percentage: {}, sp idx: {}", percentage, startpos_index);

		label->setCString(this->get_song(percentage).c_str());
	}

	bool check_in_OMA() {
		if (playing_startpos) return true;
		auto pl = PlayLayer::get();
		if (!pl) {
			return false;
		}

		// this checks for OMA's level ID which means it will not work on the OMA startpos level
		auto id = pl->m_level->m_levelID;
		if (id != OMA_ID) {
			return false;
		}

		// check that a level length was found. don't display if not found
		if (g_level_length < 0.0f) {
			return false;
		}

		return true;
	}

	// the logic is simple: get percentage, find song based on interval, get song percentage
	std::string get_song(float percentage) {
		if (percentage >= 100.0f) {
			return "Magic Touch 100%";
		}

		// get index of part in level
		size_t idx = 0;
		while (song_percentages[idx] < percentage) {
			idx += 1;
		}

		float upper_bound = song_percentages[idx];
		float lower_bound = 0.0;
		if (idx > 0) {
			lower_bound = song_percentages[idx - 1];
		}

		float dp = upper_bound - lower_bound;
		float normal_percent = percentage - lower_bound;	// normalized to satisfy 0 <= x < dp

		return fmt::format("{} {}%", song_names[idx], static_cast<int>(100.0 * normal_percent / dp));
	}
};


