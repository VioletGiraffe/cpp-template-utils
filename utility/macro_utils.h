#pragma once

#define CONCAT_TWO_STRINGS(x, y) x##y
#define CONCAT_THREE_STRINGS(x, y, z) x##y##z
#define CONCAT_EXPANDED_ARGUMENTS_2(x, y) CONCAT_TWO_STRINGS(x, y)
#define CONCAT_EXPANDED_ARGUMENTS_3(x, y, z) CONCAT_THREE_STRINGS(x, y, z)
