#pragma once

#include <stdbool.h>
#include <stdint.h>

/* ->content structure:
STRING:
| Bit   | Value
| 0     | isString (=0)
| 0:63  | strPointer (char *, guaranteed &1==0)
SPECIAL:
| Bit   | Value
| 0     | isSpecial
| 1     | isHidden
| 2     | isBoolean
| 3     | affectsBatteryView
| 4     | isFloat (may be elimated in future)
| 5     | isForeground
| 6:29  | unit (UTF-8, 3 bytes max, little endian)
| 30    | (unused)
| 31    | hasUnit
| 32:63 | value (32-bit)
*/

// Bit 1<<0: NotPointer, bc pointer alignments won't allow such bit
// Bit 1<<1:
#define BIN_IS_STRING               0
#define BIN_IS_SPECIAL              (1 << 0)
#define BIN_IS_BOOLEAN              (1 << 2 | BIN_IS_SPECIAL)
#define BIN_AFFECTS_BATTERY_CELL    (1 << 3)
#define BIN_IS_FLOAT                (1 << 4 | BIN_IS_SPECIAL)
#define BIN_IS_FOREGROUND           (1 << 5 | BIN_IS_FLOAT | BIN_AFFECTS_BATTERY_CELL)
#define BIN_IS_BACKGROUND           (0 | BIN_IS_FLOAT | BIN_AFFECTS_BATTERY_CELL)
#define BIN_IS_HIDDEN               (1 << 1)
#define BIN_UNIT_BITMASK            (((1 << 24) - 1) << 6)
// ^ Use >>6 when retrieving, max 3 bytes
// FIXME: Undefined Behavior: Left shift of 1 by 31 places cannot be represented in type 'int'
#define BIN_HAS_UNIT                (1 << 31)
#define BIN_VALUE_BIT_MASK          (((uint64_t)-1) << 32)
#define BIN_BITS_BIT_MASK           ((1L << 32) - 1)

#define BIN_UNIT_DEGREE_C           (0x8384e2 << 6 | BIN_HAS_UNIT)
#define BIN_UNIT_PERCENT            (0x25 << 6 | BIN_HAS_UNIT)
// max 3 bytes unit, conversion:
// e.g. degreeC is e2 84 83 in utf8,
// convert it to little endian, 0x8384e2, and put in the bitmask.

// content: <1024: integer 1-127
// Bit 1<<9 == 1 : IsTrueOrFalse
// Bit 1<<9 == 0 : IsNum
// Bit 1<<8 == 1 : Affects Progress View (is percentage)
// Bit 1<<7 == 1 : Is Foreground
// Bit 1<<7 == 0 : Is Background
// Bit 1<<7 is ignored if bit 1<<8==0
// ELSE IF: content: >=1024, content&1==1 && content&(1<<10)!=0
// float point value.
// ELSE: char *
/*#define BIN_IS_TRUE_OR_FALSE        (1 << 9)
#define BIN_IS_VALUE 0
#define BIN_AFFECTS_BATTERY_CELL    (1 << 8)
#define BIN_IS_FOREGROUND           (1 << 7 | BIN_AFFECTS_BATTERY_CELL)
#define BIN_IS_BACKGROUND           (0 << 0 | BIN_AFFECTS_BATTERY_CELL)
#define BIN_IS_FLOAT (1<<10 | 1)*/

struct battery_info_node {
    const char *description; // NONNULL
    int identifier;
    uint64_t content;
    struct battery_info_node *prev;
    struct battery_info_node *next;
};

struct battery_info_node *
bi_construct_linked_list(struct battery_info_node *template);
// This navigates next chain first, then prev, if found, v is updated.
bool bi_find_next(struct battery_info_node **v, int identifier);
// This function modifies the value without changing the
// definition bits.
void bi_node_change_content_value(struct battery_info_node *node,
                                  uint32_t value);
void bi_node_change_content_value_float(struct battery_info_node *node,
                                        float value);
void battery_info_update(struct battery_info_node *head);
struct battery_info_node *battery_info_init(void);
