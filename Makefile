TARGETS = power_modulating_interfaces utilties

all: $(TARGETS)

.PHONY: $(TARGETS)
power_modulating_interfaces:
	$(MAKE) -C power_modulating_interfaces all

utilities:
	$(MAKE) -C utilities all
