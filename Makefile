all:
	$(MAKE) -C kol;
	$(MAKE) -C Message;

clean:
	$(MAKE) -C kol clean;
	$(MAKE) -C Message clean;

