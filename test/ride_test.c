#include <assert.h>
#include <stdio.h>

#include "ride.h"
#include "ride_ratings.h"

#define run_test(fn_name) \
    printf("%s\n", #fn_name); \
    fn_name()

void test_apply_penalty()
{
	ride_rating excitement = RIDE_RATING(3,72);
	ride_rating new_excitement = apply_intensity_penalty(excitement, RIDE_RATING(4,52));
	assert(new_excitement == 372);
}

int main(int argc, char *argv[])
{
	run_test(test_apply_penalty);
	printf("\nOK\n");
	return 0;
}
