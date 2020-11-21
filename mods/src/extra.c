#include <stdint.h>
#include <stdio.h>

#include <libcore/libcore.h>

#include "extra.h"
#include "server/server.h"

static int mob_spawning = 0;
// Override Mob Spawning
static uint32_t LevelData_getSpawnMobs_injection(__attribute__((unused)) unsigned char *level_data) {
    return mob_spawning;
}

// Store Right-Click Status
static int is_right_click = 0;
void extra_set_is_right_click(int val) {
    is_right_click = val;
}

typedef void (*releaseUsingItem_t)(unsigned char *game_mode, unsigned char *player);

typedef void (*Minecraft_tickInput_t)(unsigned char *minecraft);
static Minecraft_tickInput_t Minecraft_tickInput = (Minecraft_tickInput_t) 0x15ffc;

typedef int (*Player_isUsingItem_t)(unsigned char *player);
static Player_isUsingItem_t Player_isUsingItem = (Player_isUsingItem_t) 0x8f15c;

// Enable Bow & Arrow Fix
static int fix_bow = 0;

// Store Function Input
static int hide_gui_toggle = 0;
void extra_hide_gui() {
    hide_gui_toggle++;
}
static int third_person_toggle = 0;
void extra_third_person() {
    third_person_toggle++;
}

// Handle Input Fixes
static void Minecraft_tickInput_injection(unsigned char *minecraft) {
    // Call Original Method
    (*Minecraft_tickInput)(minecraft);

    if (fix_bow && !is_right_click) {
        // GameMode Is Offset From minecraft By 0x160
        // Player Is Offset From minecraft By 0x18c
        unsigned char *game_mode = *(unsigned char **) (minecraft + 0x160);
        unsigned char *player = *(unsigned char **) (minecraft + 0x18c);
        if (player != NULL && game_mode != NULL && (*Player_isUsingItem)(player)) {
            unsigned char *game_mode_vtable = *(unsigned char **) game_mode;
            releaseUsingItem_t releaseUsingItem = *(releaseUsingItem_t *) (game_mode_vtable + 0x5c);
            (*releaseUsingItem)(game_mode, player);
        }
    }

    // Clear Unused Sign Input
    extra_clear_input();

    // Handle Functions
    unsigned char *options = minecraft + 0x3c;
    if (hide_gui_toggle % 2 != 0) {
        // Toggle Hide GUI
        *(options + 0xec) = *(options + 0xec) ^ 1;
    }
    hide_gui_toggle = 0;
    if (third_person_toggle % 2 != 0) {
        // Toggle Third Person
        *(options + 0xed) = *(options + 0xed) ^ 1;
    }
    third_person_toggle = 0;
}

typedef void (*Gui_tickItemDrop_t)(unsigned char *);
static Gui_tickItemDrop_t Gui_tickItemDrop = (Gui_tickItemDrop_t) 0x27778;

#include <SDL/SDL_events.h>

// Block UI Interaction When Mouse Is Locked
static void Gui_tickItemDrop_injection(unsigned char *this) {
    if (SDL_ShowCursor(SDL_QUERY) == SDL_ENABLE) {
        // Call Original Method
        (*Gui_tickItemDrop)(this);
    }
}

typedef void (*Gui_handleClick_t)(unsigned char *this, int32_t param_2, int32_t param_3, int32_t param_4);
static Gui_handleClick_t Gui_handleClick = (Gui_handleClick_t) 0x2599c;

// Block UI Interaction When Mouse Is Locked
static void Gui_handleClick_injection(unsigned char *this, int32_t param_2, int32_t param_3, int32_t param_4) {
    if (SDL_ShowCursor(SDL_QUERY) == SDL_ENABLE) {
        // Call Original Method
        (*Gui_handleClick)(this, param_2, param_3, param_4);
    }
}

static int is_survival = -1;

static void *Creator = (void *) 0x1a044;
static void *SurvivalMode = (void *) 0x1b7d8;
static void *CreativeMode = (void *) 0x1b258;

// Patch Game Mode
static void set_is_survival(int new_is_survival) {
    if (is_survival != new_is_survival) {
        INFO("Setting Game Mode: %s", new_is_survival ? "Survival" : "Creative");

        // Correct Inventpry UI
        unsigned char inventory_patch[4] = {new_is_survival ? 0x00 : 0x01, 0x30, 0xa0, 0xe3};
        patch((void *) 0x16efc, inventory_patch);

        // Use Correct Size For GameMode Object
        unsigned char size_patch[4] = {new_is_survival ? 0x24 : 0x18, 0x00, 0xa0, 0xe3};
        patch((void *) 0x16ee4, size_patch);

        // Replace Creator Constructor With CreatorMode Or SurvivalMode Constructor
        overwrite(Creator, new_is_survival ? SurvivalMode : CreativeMode);

        is_survival = new_is_survival;
    }
}

typedef void (*Minecraft_setIsCreativeMode_t)(unsigned char *, int32_t);
static Minecraft_setIsCreativeMode_t Minecraft_setIsCreativeMode = (Minecraft_setIsCreativeMode_t) 0x16ec4;

// Handle Gamemode Switching
static void Minecraft_setIsCreativeMode_injection(unsigned char *this, int32_t new_game_mode) {
    set_is_survival(!new_game_mode);

    // Call Original Method
    (*Minecraft_setIsCreativeMode)(this, new_game_mode);
}

// Get Custom Username
static char *get_username() {
    char *username = getenv("MCPI_USERNAME");
    if (username == NULL) {
        username = "StevePi";
    }
    return username;
}

typedef void (*Minecraft_init_t)(unsigned char *this);
static Minecraft_init_t Minecraft_init = (Minecraft_init_t) 0x1700c;

static int fancy_graphics;
static int peaceful_mode;
static int anaglyph;
// Configure Options
static void Minecraft_init_injection(unsigned char *this) {
    // Call Original Method
    (*Minecraft_init)(this);

    unsigned char *options = this + 0x3c;
    // Enable Fancy Graphics
    *(options + 0x17) = fancy_graphics;
    // Enable Crosshair In Touch GUI
    *(options + 0x105) = 1;
    // Peaceful Mode
    *(int32_t *) (options + 0xe8) = peaceful_mode ? 0 : 2;
    // 3D Anaglyph
    *(options + 0x15) = anaglyph;
}

// Is Dedicated Server
static int is_server = 0;

// Check For Feature
int extra_has_feature(const char *name) {
    if (is_server) {
        // Enable All Features In Server
        return 1;
    } else {
        char *env = getenv("MCPI_FEATURES");
        char *features = strdup(env != NULL ? env : "");
        char *tok = strtok(features, "|");
        int ret = 0;
        while (tok != NULL) {
            if (strcmp(tok, name) == 0) {
                ret = 1;
                break;
            }
            tok = strtok(NULL, "|");
        }
        free(features);
        INFO("Feature: %s: %s", name, ret ? "Enabled" : "Disabled");
        return ret;
    }
}

// Get Graphics Mode
int extra_get_mode() {
    char *mode = getenv("MCPI_MODE");
    if (mode == NULL) {
        ERR("%s", "MCPI Mode Not Specified");
    } else if (strcmp("virgl", mode) == 0) {
        return 0;
    } else if (strcmp("native", mode) == 0) {
        return 1;
    } else if (strcmp("server", mode) == 0) {
        return 2;
    } else {
        ERR("Inavlid MCPI_MODE: %s", mode);
    }
}

// Enable Touch GUI
static int32_t Minecraft_isTouchscreen(__attribute__((unused)) unsigned char *minecraft) {
    return 1;
}

// Store Smooth Lighting
static int smooth_lighting;
int extra_get_smooth_lighting() {
    return smooth_lighting;
}

__attribute__((constructor)) static void init() {
    is_server = extra_get_mode() == 2;
    if (is_server) {
        server_init();
    }

    int touch_gui = !is_server && extra_has_feature("Touch GUI");
    if (touch_gui) {
        // Main UI
        overwrite((void *) 0x1639c, Minecraft_isTouchscreen);
        // Force Correct Toolbar Size
        unsigned char toolbar_patch[4] = {0x01, 0x00, 0x50, 0xe3};
        patch((void *) 0x257b0, toolbar_patch);
    }

    // Dynamic Game Mode Switching
    set_is_survival(1);
    overwrite_calls((void *) Minecraft_setIsCreativeMode, Minecraft_setIsCreativeMode_injection);

    // Get Default Game Mode
    if (!is_server) {
        int default_game_mode = !extra_has_feature("Survival Mode");

        // Set Default Game Mode
        unsigned char default_game_mode_patch[4] = {default_game_mode ? 0x01 : 0x00, 0x30, 0xa0, 0xe3};
        patch((void *) 0x3d9b8, default_game_mode_patch);
        patch((void *) 0x38a78, default_game_mode_patch);
    }

    // Disable Item Dropping Using The Cursor When Cursor Is Hidden
    overwrite_calls((void *) Gui_tickItemDrop, Gui_tickItemDrop_injection);
    // Disable Opening Inventory Using The Cursor When Cursor Is Hidden
    overwrite_calls((void *) Gui_handleClick, Gui_handleClick_injection);

    // Enable Bow & Arrow Fix
    fix_bow = extra_has_feature("Fix Bow & Arrow");
    // Fix Bow & Arrow + Clear Unused Sign Input
    overwrite_calls((void *) Minecraft_tickInput, Minecraft_tickInput_injection);

    if (extra_has_feature("Fix Attacking")) {
        // Allow Attacking Mobs
        unsigned char attacking_patch[4] = {0x00, 0xf0, 0x20, 0xe3};
        patch((void *) 0x162d4, attacking_patch);
        // Fix Instamining When Using This Patch
        unsigned char instamine_patch[4] = {0x61, 0x00, 0x00, 0xea};
        patch((void *) 0x15b0c, instamine_patch);
        // Fix Excessive Hand Swinging When Using This Patch
        unsigned char excessive_swing_patch[4] = {0x06, 0x00, 0x00, 0xea};
        patch((void *) 0x1593c, excessive_swing_patch);
    }

    if (is_server) {
        mob_spawning = server_get_mob_spawning();
    } else {
        mob_spawning = extra_has_feature("Mob Spawning");
    }
    // Set Mob Spawning
    overwrite((void *) 0xbabec, LevelData_getSpawnMobs_injection);

    // Replace CreatorLevel With ServerLevel (This Fixes Beds And Mob Spawning)
    unsigned char level_patch[4] = {0x68, 0x7e, 0x01, 0xeb};
    patch((void *) 0x16f84, level_patch);

    // Allocate Correct Size For ServerLevel
    unsigned char level_size_patch[4] = {0x94, 0x0b, 0x00, 0x00};
    patch((void *) 0x17004, level_size_patch);

    // Enable Fancy Graphics
    fancy_graphics = extra_has_feature("Fancy Graphics");
    // Peaceful Mode
    peaceful_mode = extra_has_feature("Peaceful Mode");
    // 3D Anaglyph
    anaglyph = extra_has_feature("3D Anaglyph");

    // Set Options
    overwrite_calls((void *) Minecraft_init, Minecraft_init_injection);

    // Allow Connecting To Non-Pi Servers
    unsigned char server_patch[4] = {0x0f, 0x00, 0x00, 0xea};
    patch((void *) 0x6dc70, server_patch);

    // Change Username
    const char *username;
    if (is_server) {
        // MOTD is Username
        username = server_get_motd();
    } else {
        username = get_username();
        INFO("Setting Username: %s", username);
    }
    char **default_username = (char **) 0x18fd4;
    if (strcmp(*default_username, "StevePi") != 0) {
        ERR("%s", "Default Username Is Invalid");
    }
    patch_address((void *) default_username, (void *) username);

    if (extra_has_feature("Disable Autojump By Default")) {
        // Disable Autojump By Default
        unsigned char autojump_patch[4] = {0x00, 0x30, 0xa0, 0xe3};
        patch((void *) 0x44b90, autojump_patch);
    }

    // Show Block Outlines
    int block_outlines = extra_has_feature("Show Block Outlines");
    unsigned char outline_patch[4] = {block_outlines ? !touch_gui : touch_gui, 0x00, 0x50, 0xe3};
    patch((void *) 0x4a210, outline_patch);

    if (extra_has_feature("Remove Invalid Item Background")) {
        // Remove Invalid Item Background (A Red Background That Appears For Items That Are Not Obtainable Without Modding/Inventory Editing)
        unsigned char invalid_item_background_patch[4] = {0x00, 0xf0, 0x20, 0xe3};
        patch((void *) 0x63c98, invalid_item_background_patch);
    }

    smooth_lighting = extra_has_feature("Smooth Lighting");
    if (smooth_lighting) {
        // Enable Smooth Lighting
        unsigned char smooth_lighting_patch[4] = {0x01, 0x00, 0x53, 0xe3};
        patch((void *) 0x59ea4, smooth_lighting_patch);
    }
}
