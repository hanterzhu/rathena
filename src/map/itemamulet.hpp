// Copyright (c) Pandas Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "status.hpp"
#include "itemdb.hpp"

#include <common/cbasetypes.hpp>
#include <common/database.hpp>

struct s_amulet_properties {
    uint32 nameid;
    uint32 group;
    uint32 maxstack;
    uint32 priority;
};

struct s_amulet_cal {
    uint32 priority;
    uint32 maxstack;
    short amount;
    uint32 index;
    struct script_code *script;
};

bool amulet_is(t_itemid nameid);
int amulet_type(t_itemid nameid);

bool amulet_is_firstone(map_session_data *sd, struct item *item, int amount);
bool amulet_is_lastone(map_session_data *sd, int n, int amount);

void amulet_apply_additem(map_session_data *sd, int n, bool is_firstone);
void amulet_apply_delitem(map_session_data *sd, int n, bool is_lastone);

void amulet_status_calc(map_session_data *sd, uint8 opt);

class AmuletProperties : public TypesafeYamlDatabase<uint32, s_amulet_properties> {
public:
    AmuletProperties() : TypesafeYamlDatabase("AMULET_PROPERTIES_DB", 2) {

    }

    const std::string getDefaultLocation();
    uint64 parseBodyNode(const ryml::NodeRef& node) override;

    void parsePropertiesToItemDB(ItemDatabase& item_db);
    std::shared_ptr<s_amulet_properties> getProperty(uint32 nameid);
};

extern AmuletProperties amulet_properties_db;
