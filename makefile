.PHONY: clean build test

MAKE_FLAGS=-j $$(nproc)

# ============================================================================ #
#  VARIABLE DEFAULTS
# ============================================================================ #

# Variant (sylvan, coom, buddy)
V:=sylvan

# Memory
M:=128

# Input variable
N:=0

# ============================================================================ #
#  BUILD
# ============================================================================ #
build:
	@mkdir -p build/ && cd build/ && cmake ..

build-pigeonhole_principle: | build
	@cd build/ && make ${MAKE_FLAGS} ${V}_pigeonhole_principle

build-queens: | build
	@cd build/ && make ${MAKE_FLAGS} ${V}_queens

build-queens-sat: | build
	@cd build/ && make ${MAKE_FLAGS} ${V}_queens_sat

build-tic_tac_toe: | build
	@cd build/ && make ${MAKE_FLAGS} ${V}_tic_tac_toe


clean:
	@rm -rf build/


# ============================================================================ #
#  EXAMPLES
# ============================================================================ #
pigeonhole_principle: N := 10
pigeonhole_principle: | build-pigeonhole_principle
	@$(subst VARIANT,$(V),./build/src/VARIANT_pigeonhole_principle) $(N) $(M)

queens: N := 8
queens: | build-queens
	@$(subst VARIANT,$(V),./build/src/VARIANT_queens) $(N) $(M)

queens-sat: N := 8
queens-sat: | build-queens-sat
	@$(subst VARIANT,$(V),./build/src/VARIANT_queens_sat) $(N) $(M)

tic_tac_toe: N := 20
tic_tac_toe: | build-tic_tac_toe
	@$(subst VARIANT,$(V),./build/src/VARIANT_tic_tac_toe) $(N) $(M)

