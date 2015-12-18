TARGETS = stress signal_insert_delays rapl

all: $(TARGETS)

.PHONY: $(TARGETS) clean

stress:
	$(MAKE) -C stress all

signal_insert_delays:
	$(MAKE) -C signal_insert_delays all

rapl:
	$(MAKE) -C rapl all

clean:
	$(MAKE) -C stress clean
	$(MAKE) -C signal_insert_delays clean
	$(MAKE) -C rapl clean
