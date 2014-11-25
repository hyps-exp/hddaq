all:
	$(MAKE) -C kol;
	$(MAKE) -C Message;
	$(MAKE) -C Frontend;

clean:
	$(MAKE) -C kol clean;
	$(MAKE) -C Message clean;
	$(MAKE) -C Frontend clean;

