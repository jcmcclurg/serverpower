TARGETS = integralController playback power_gadget_whileloop triangleWave

all: $(TARGETS)

.PHONY: $(TARGETS) clean
integralController:
	$(MAKE) -C integralController all

playback:
	$(MAKE) -C playback all

power_gadget_whileloop:
	$(MAKE) -C power_gadget_whileloop all

triangleWave:
	$(MAKE) -C triangleWave all

clean:
	$(MAKE) -C integralController clean
	$(MAKE) -C playback clean
	$(MAKE) -C power_gadget_whileloop clean
	$(MAKE) -C triangleWave clean
