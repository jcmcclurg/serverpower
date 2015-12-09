TARGETS = power_modulating_interfaces utilities

all: $(TARGETS)

.PHONY: $(TARGETS)

power_modulating_interfaces:
	$(MAKE) -C power_modulating_interfaces all

utilities:
	$(MAKE) -C utilities all
