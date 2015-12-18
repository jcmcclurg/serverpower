TARGETS = power_modulating_interfaces utilities

all: $(TARGETS)

.PHONY: $(TARGETS) clean

power_modulating_interfaces:
	$(MAKE) -C power_modulating_interfaces all

utilities:
	$(MAKE) -C utilities all

clean:
	$(MAKE) -C power_modulating_interfaces clean
	$(MAKE) -C utilities clean
