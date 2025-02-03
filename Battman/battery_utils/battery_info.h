#pragma once

#include <stdbool.h>
// content: <1024: integer 1-127
// Bit 1<<9 == 1 : IsTrueOrFalse
// Bit 1<<9 == 0 : IsNum
// Bit 1<<8 == 1 : Affects Progress View (is percentage)
// Bit 1<<7 == 1 : Is Foreground
// Bit 1<<7 == 0 : Is Background
// Bit 1<<7 is ignored if bit 1<<8==0
// ELSE IF: content: >=1024, content&1==1
// No pointer shall end with 1, so it does not collapse
// ^ Exception being cases where weird operations done to char *
// ^ Please malloc().
// Not used now. Ignore.
// ELSE: char *
#define BIN_IS_TRUE_OR_FALSE        (1 << 9)
#define BIN_IS_VALUE 0
#define BIN_AFFECTS_BATTERY_CELL    (1 << 8)
#define BIN_IS_FOREGROUND           (1 << 7 | BIN_AFFECTS_BATTERY_CELL)
#define BIN_IS_BACKGROUND           (0 << 0 | BIN_AFFECTS_BATTERY_CELL)

struct battery_info_node {
	const char *description; // NONNULL
	int identifier;
	void *content;
	struct battery_info_node *prev;
	struct battery_info_node *next;
};

struct battery_info_node *bi_construct_linked_list(struct battery_info_node *template);
// This navigates next chain first, then prev, if found, v is updated.
bool bi_find_next(struct battery_info_node **v, int identifier);
// This function modifies the value without changing the
// definition bits.
void bi_node_change_content_value(struct battery_info_node *node, unsigned int value);
void battery_info_update(struct battery_info_node *head);
struct battery_info_node *battery_info_init(void);
