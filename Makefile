all:
	$(MAKE) -C kol
	$(MAKE) -C Message
	$(MAKE) -C EventData
	$(MAKE) -C RingBuffer
	$(MAKE) -C ControlThread
	$(MAKE) -C EventBuilder
	$(MAKE) -C EventDistributor
	$(MAKE) -C Recorder

clean:
	$(MAKE) -C kol clean
	$(MAKE) -C Message clean
	$(MAKE) -C EventData clean
	$(MAKE) -C RingBuffer clean
	$(MAKE) -C ControlThread clean
	$(MAKE) -C EventBuilder clean
	$(MAKE) -C EventDistributor clean
	$(MAKE) -C Recorder clean
