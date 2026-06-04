#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// ANSI colours
#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN    "\x1b[36m"
#define RESET   "\x1b[0m"

// Talent tree (single linear path)
typedef struct {
    int tier;   // 0 = none, 1..5 = tiers unlocked
} TalentTree;

// Character
typedef struct {
    char name[32];
    char element[16];
    char role[16];
    int level;
    int maxHP;
    int hp;
    int attack;
    int defense;
    int xp;
    TalentTree talents;
} Character;

// Status effects
typedef struct {
    int burn;
    int shock;
    int regen;
    int barrier;
} Status;

// Elemental multiplier
float getMultiplier(const char *atk, const char *def) {
    if (strcmp(atk, "Fire") == 0 && strcmp(def, "Nature") == 0) return 1.5f;
    if (strcmp(atk, "Nature") == 0 && strcmp(def, "Lightning") == 0) return 1.5f;
    if (strcmp(atk, "Lightning") == 0 && strcmp(def, "Water") == 0) return 1.5f;
    if (strcmp(atk, "Water") == 0 && strcmp(def, "Fire") == 0) return 1.5f;

    if (strcmp(atk, "Fire") == 0 && strcmp(def, "Water") == 0) return 0.7f;
    if (strcmp(atk, "Nature") == 0 && strcmp(def, "Fire") == 0) return 0.7f;
    if (strcmp(atk, "Lightning") == 0 && strcmp(def, "Nature") == 0) return 0.7f;
    if (strcmp(atk, "Water") == 0 && strcmp(def, "Lightning") == 0) return 0.7f;

    return 1.0f;
}

// Health bar
void printHealthBar(const char *label, int hp, int maxHP) {
    int width = 20;
    int filled = (hp * width) / maxHP;
    if (filled < 0) filled = 0;
    if (filled > width) filled = width;

    printf("%s: [", label);
    for (int i = 0; i < width; i++) {
        if (i < filled) printf(GREEN "#");
        else printf(" ");
    }
    printf(RESET "] %d/%d\n", hp, maxHP);
}

// Crit
int checkCritical(const Character *c) {
    int baseChance = 10;
    if (strcmp(c->role, "DPS") == 0)  baseChance += 5;
    if (strcmp(c->role, "Mage") == 0) baseChance += 3;
    return (rand() % 100 < baseChance);
}

// Status tick
void applyStatus(Character *c, Status *s) {
    if (s->burn > 0) {
        printf(RED "%s suffers burn damage! (-2 HP)\n" RESET, c->name);
        c->hp -= 2;
        s->burn--;
    }
    if (s->regen > 0) {
        printf(GREEN "%s regenerates! (+2 HP)\n" RESET, c->name);
        c->hp += 2;
        if (c->hp > c->maxHP) c->hp = c->maxHP;
        s->regen--;
    }
}

// Talent descriptions per class/tier
const char* getTalentDesc(const Character *c, int tier) {
    if (strcmp(c->role, "DPS") == 0) {
        switch (tier) {
            case 1: return "+2 Attack";
            case 2: return "+2 Attack";
            case 3: return "+4 Max HP";
            case 4: return "+5%% Critical Chance (flavour)";
            case 5: return "+3 Attack";
        }
    } else if (strcmp(c->role, "Tank") == 0) {
        switch (tier) {
            case 1: return "+2 Defense";
            case 2: return "+6 Max HP";
            case 3: return "+2 Defense";
            case 4: return "+4 Max HP";
            case 5: return "+1 Defense, +3 Max HP";
        }
    } else if (strcmp(c->role, "Mage") == 0) {
        switch (tier) {
            case 1: return "+3 Attack";
            case 2: return "+2 Attack";
            case 3: return "+3 Max HP";
            case 4: return "+2 Attack";
            case 5: return "+2 Max HP, +1 Attack";
        }
    } else if (strcmp(c->role, "Support") == 0) {
        switch (tier) {
            case 1: return "+1 Defense, +3 Max HP";
            case 2: return "+2 Attack";
            case 3: return "+4 Max HP";
            case 4: return "+1 Defense";
            case 5: return "+2 Max HP, +1 Attack";
        }
    }
    return "Unknown";
}

// Apply talent bonuses on unlock
void applyTalentTier(Character *c) {
    if (c->talents.tier >= 5) return;
    c->talents.tier++;
    int t = c->talents.tier;

    printf(CYAN "\n=== Talent Tier %d unlocked for %s! ===\n" RESET, t, c->name);

    if (strcmp(c->role, "DPS") == 0) {
        if (t == 1 || t == 2) {
            c->attack += 2;
        } else if (t == 3) {
            c->maxHP += 4; c->hp = c->maxHP;
        } else if (t == 4) {
            // flavour: crit chance already handled in checkCritical
        } else if (t == 5) {
            c->attack += 3;
        }
    } else if (strcmp(c->role, "Tank") == 0) {
        if (t == 1) {
            c->defense += 2;
        } else if (t == 2) {
            c->maxHP += 6; c->hp = c->maxHP;
        } else if (t == 3) {
            c->defense += 2;
        } else if (t == 4) {
            c->maxHP += 4; c->hp = c->maxHP;
        } else if (t == 5) {
            c->defense += 1; c->maxHP += 3; c->hp = c->maxHP;
        }
    } else if (strcmp(c->role, "Mage") == 0) {
        if (t == 1) {
            c->attack += 3;
        } else if (t == 2) {
            c->attack += 2;
        } else if (t == 3) {
            c->maxHP += 3; c->hp = c->maxHP;
        } else if (t == 4) {
            c->attack += 2;
        } else if (t == 5) {
            c->maxHP += 2; c->attack += 1; c->hp = c->maxHP;
        }
    } else if (strcmp(c->role, "Support") == 0) {
        if (t == 1) {
            c->defense += 1; c->maxHP += 3; c->hp = c->maxHP;
        } else if (t == 2) {
            c->attack += 2;
        } else if (t == 3) {
            c->maxHP += 4; c->hp = c->maxHP;
        } else if (t == 4) {
            c->defense += 1;
        } else if (t == 5) {
            c->maxHP += 2; c->attack += 1; c->hp = c->maxHP;
        }
    }

    printf("Talent effect: %s\n", getTalentDesc(c, t));
}

// ASCII vertical talent tree UI
void showTalentTree(const Character *c) {
    printf(CYAN "\n============== TALENT TREE ==============\n" RESET);
    printf("Character: %s (Level %d)\n", c->name, c->level);
    printf("Role: %s | Element: %s\n", c->role, c->element);
    printf("----------------------------------------\n\n");

    printf("                TALENT TREE\n");
    printf("                 (%s)\n\n", c->name);

    for (int tier = 5; tier >= 1; tier--) {
        int unlocked = (c->talents.tier >= tier);
        printf("                     Tier %d\n", tier);
        printf("                      [%c]\n", unlocked ? 'X' : ' ');
        if (tier > 1) printf("                       |\n");
    }

    printf("\nTier Details:\n");
    for (int tier = 1; tier <= 5; tier++) {
        int unlocked = (c->talents.tier >= tier);
        printf(" Tier %d [%c]: %s\n",
               tier, unlocked ? 'X' : ' ', getTalentDesc(c, tier));
    }

    printf("\n========================================\n");
}

// Level up
void levelUp(Character *c) {
    while (c->xp >= 100) {
        c->xp -= 100;
        c->level++;
        c->maxHP += 4;
        c->attack += 1;
        c->defense += 1;
        c->hp = c->maxHP;
        printf(YELLOW "\n%s reached Level %d! Base stats increased.\n" RESET,
               c->name, c->level);
        applyTalentTier(c);
        showTalentTree(c);
    }
}

int main(void) {
    srand((unsigned)time(NULL));

    // Character roster
    Character chars[4] = {
        {"Pyreblade Knight",  "Fire",      "DPS",    1, 34, 34, 8, 4, 0, {0}},
        {"Frostborn Sentinel","Ice",       "Tank",   1, 42, 42, 5, 7, 0, {0}},
        {"Stormcaller Adept", "Lightning", "Mage",   1, 30, 30,10, 3, 0, {0}},
        {"Verdant Warden",    "Nature",    "Support",1, 36, 36, 6, 5, 0, {0}}
    };

    printf(CYAN "=== CHOOSE YOUR CHARACTER ===\n\n" RESET);
    for (int i = 0; i < 4; i++) {
        printf("%d) %s (%s)  Role: %s  HP: %d  ATK: %d  DEF: %d\n",
               i + 1,
               chars[i].name,
               chars[i].element,
               chars[i].role,
               chars[i].maxHP,
               chars[i].attack,
               chars[i].defense);
    }

    int sel;
    printf("\nEnter your choice (1-4): ");
    if (scanf("%d", &sel) != 1 || sel < 1 || sel > 4) {
        printf("Invalid choice.\n");
        return 0;
    }

    Character player = chars[sel - 1];

    int running = 1;
    while (running) {
        printf(BLUE "\n=== MAIN MENU ===\n" RESET);
        printf("1) Start Battle\n");
        printf("2) View Talent Tree\n");
        printf("3) Exit\n");
        printf("Enter choice: ");
        int menu;
        if (scanf("%d", &menu) != 1) {
            printf("Invalid input.\n");
            return 0;
        }

        if (menu == 2) {
            showTalentTree(&player);
            continue;
        } else if (menu == 3) {
            printf("Goodbye.\n");
            break;
        } else if (menu != 1) {
            printf("Invalid choice.\n");
            continue;
        }

        // New enemy each battle
        Character enemy = {"Umbravore Shade", "Shadow", "Boss", 1, 40, 40, 7, 4, 0, {0}};
        Status pStatus = {0};
        Status eStatus = {0};
        int guard = 0;

        printf(GREEN "\nYou enter battle as %s (%s, %s).\n" RESET,
               player.name, player.element, player.role);
        printf(RED "Enemy appears: %s (%s, %s)!\n\n" RESET,
               enemy.name, enemy.element, enemy.role);

        while (player.hp > 0 && enemy.hp > 0) {
            applyStatus(&player, &pStatus);
            applyStatus(&enemy, &eStatus);
            if (player.hp <= 0 || enemy.hp <= 0) break;

            printf(BLUE "\n=== BATTLE ===\n" RESET);
            printHealthBar(player.name, player.hp, player.maxHP);
            printHealthBar(enemy.name, enemy.hp, enemy.maxHP);

            printf("\n1) Basic Attack\n");
            printf("2) Elemental Strike\n");
            printf("3) Class Skill\n");
            printf("4) Heal\n");
            printf("5) Guard\n");
            printf("6) Run\n");

            int choice;
            printf("Enter move: ");
            if (scanf("%d", &choice) != 1) {
                printf("Invalid input.\n");
                return 0;
            }

            // PLAYER TURN
            if (choice == 1) {
                int dmg = player.attack - enemy.defense / 4;
                if (dmg < 1) dmg = 1;
                if (checkCritical(&player)) {
                    printf(YELLOW "CRITICAL HIT!\n" RESET);
                    dmg = (int)(dmg * 1.8f);
                }
                printf(GREEN "%s attacks!\n" RESET, player.name);
                enemy.hp -= dmg;
                printf("You dealt %d damage.\n", dmg);
            }
            else if (choice == 2) {
                printf(GREEN "%s uses Elemental Strike!\n" RESET, player.name);
                if (rand() % 100 < 80) {
                    float mult = getMultiplier(player.element, enemy.element);
                    int dmg = (int)((player.attack + 4) * mult);
                    if (checkCritical(&player)) {
                        printf(YELLOW "CRITICAL ELEMENTAL HIT!\n" RESET);
                        dmg = (int)(dmg * 1.8f);
                    }
                    dmg -= enemy.defense / 5;
                    if (dmg < 1) dmg = 1;
                    enemy.hp -= dmg;
                    printf("You dealt %d damage.\n", dmg);
                } else {
                    printf("The attack missed!\n");
                }
            }
            else if (choice == 3) {
                if (strcmp(player.role, "DPS") == 0) {
                    printf(RED "%s uses Flameburst Slash!\n" RESET, player.name);
                    int dmg = player.attack + 6;
                    enemy.hp -= dmg;
                    printf("You dealt %d damage!\n", dmg);
                    if (rand() % 100 < 40) {
                        printf(RED "Enemy is BURNED!\n" RESET);
                        eStatus.burn = 3;
                    }
                } else if (strcmp(player.role, "Tank") == 0) {
                    printf(CYAN "%s raises a Glacial Barrier!\n" RESET, player.name);
                    pStatus.barrier = 3;
                } else if (strcmp(player.role, "Mage") == 0) {
                    printf(YELLOW "%s casts Thunder Arc!\n" RESET, player.name);
                    int dmg = player.attack + 4;
                    enemy.hp -= dmg;
                    printf("You dealt %d damage!\n", dmg);
                    if (rand() % 100 < 50) {
                        printf(YELLOW "Enemy is SHOCKED!\n" RESET);
                        eStatus.shock = 2;
                    }
                } else if (strcmp(player.role, "Support") == 0) {
                    printf(GREEN "%s uses Verdant Surge!\n" RESET, player.name);
                    player.hp += 10;
                    if (player.hp > player.maxHP) player.hp = player.maxHP;
                    printf("You healed to %d HP.\n", player.hp);
                    if (rand() % 100 < 40) {
                        printf(GREEN "Regeneration applied!\n" RESET);
                        pStatus.regen = 3;
                    }
                }
            }
            else if (choice == 4) {
                printf(GREEN "%s heals!\n" RESET, player.name);
                player.hp += 8;
                if (player.hp > player.maxHP) player.hp = player.maxHP;
                printf("You healed to %d HP.\n", player.hp);
            }
            else if (choice == 5) {
                printf(GREEN "%s guards!\n" RESET, player.name);
                guard = 1;
            }
            else if (choice == 6) {
                printf(YELLOW "You fled.\n" RESET);
                break;
            } else {
                printf("Invalid move. Turn wasted.\n");
            }

            if (enemy.hp <= 0) break;

            // ENEMY TURN
            printf(RED "\nEnemy turn!\n" RESET);
            int dmg = enemy.attack;
            if (checkCritical(&enemy)) {
                printf(RED "Enemy CRITICAL HIT!\n" RESET);
                dmg = (int)(dmg * 1.5f);
            }
            if (pStatus.barrier > 0) {
                printf(CYAN "Glacial Barrier reduces damage!\n" RESET);
                dmg /= 2;
                pStatus.barrier--;
            }
            if (guard) {
                dmg /= 2;
                guard = 0;
            }
            if (dmg < 1) dmg = 1;
            player.hp -= dmg;
            printf("You took %d damage.\n", dmg);
        }

        if (player.hp <= 0) {
            printf(RED "\nYou were defeated.\n" RESET);
            running = 0;
        } else if (enemy.hp <= 0) {
            printf(GREEN "\nVictory!\n" RESET);
            int gained = 80 + rand() % 41;
            player.xp += gained;
            printf(CYAN "%s gained %d XP!\n" RESET, player.name, gained);
            levelUp(&player);
        }

        // restore some HP between battles
        if (player.hp > 0 && running) {
            if (player.hp < player.maxHP) {
                player.hp += 5;
                if (player.hp > player.maxHP) player.hp = player.maxHP;
            }
        }
    }

    return 0;
}

