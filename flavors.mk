ifeq ($(FLAVOR),q3a)
TARGET := ioquake3_gc
BUILD := build
FLAVOR_CFLAGS := -DBASEGAME=\"baseq3\"
else ifeq ($(FLAVOR),oa)
TARGET := ioquake3_gc_oa
BUILD := build_oa
FLAVOR_CFLAGS := -DBASEGAME=\"baseoa\" -DSTANDALONE
else
$(error Unknown FLAVOR "$(FLAVOR)": use q3a or oa)
endif
