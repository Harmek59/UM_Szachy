## Chess engine minmax

In MinMax.h few option can be set by using #define:

-ALPHA_BETA - alpha beta pruning

-MOVE_ORDERING - should be used only with alpha beta, it tries approximate best moves to check them first, so alpha beta
pruning performs better

-GENERATE_MOVES_WITH_ADDITIONAL_DATA - it generates moves with additional data like checks, captures, so it can be used
in MOVE_ORDERING, however it looks like it does perform worse with this option

-TIME_OUT - time limit for calculations (you can specify number of ms)

-HASH_TABLE - it's trash, don't use

-DEBUG_STATS - do not use it (worse performance)

In main.cpp there is *evaluate*, which is used for final evaluation. In MinMax.h there is
*calculateMoveWeight* lambda, which is used to approximate move ordering 